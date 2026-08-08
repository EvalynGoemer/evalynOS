#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#include <utils/lib.h>
#include <arch/x86_64/cpu/ap.h>
#include <utils/limine.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/acpi/madt.h>
#include <arch/generic/paging/paging.h>
#include <mem/spalloc.h>
#include <mem/pmm.h>
#include <mem/vmem.h>
#include <arch/x86_64/timer/timer.h>
#include <arch/intrin/cpulocal.h>

static uint64_t get_nth_lomem_page(uint64_t n) {
    uint64_t count = 0;
    for (uint64_t e = 0; e < memmap_request.response->entry_count; e++) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[e];
        if (entry->type != LIMINE_MEMMAP_USABLE) continue;
        if (entry->base >= 0x100000ULL) return 0;

        uint64_t start = MAX(ALIGN_UP(entry->base, PAGE_SIZE), 0x1000ULL);
        uint64_t end   = MIN(ALIGN_DOWN(entry->base + entry->length, PAGE_SIZE), 0x100000ULL);
        if (end <= start) continue;

        uint64_t pages = (end - start) / PAGE_SIZE;
        if (n < count + pages)
            return start + (n - count) * PAGE_SIZE;
        count += pages;
    }
    return 0;
}

global_ap_data_t* global_ap_data;

static uint64_t setup_trampoline() {
    // make a 2mb identity map with the first lomem page
    uint64_t ap_page_table = get_nth_lomem_page(0);
    assert(ap_page_table != 0);
    memcpy(TO_HHDM_PTR(ap_page_table), TO_HHDM_PTR(kernel_page_table), PAGE_SIZE);
    paging_map_page(ap_page_table, 0, 0, PAGE_KRWX, PAGE_LARGE);

    // find the second lowmem page for the trampoline its self
    uint64_t trampoline_page = get_nth_lomem_page(1);
    assert(trampoline_page != 0);

    // relocate the trampoline
    uint64_t trampoline_virt = (uint64_t)ap_trampoline;
    #define RELOC(dst, sym) (dst) = trampoline_page + ((uint64_t)(sym) - trampoline_virt)
    RELOC(ap_trampoline_data.gdtr_base, ap_trampoline_gdt);
    RELOC(ap_trampoline_data.fjmp32_addr,    ap_trampoline32);
    RELOC(ap_trampoline_data.fjmp64_addr,  ap_trampoline64);
    #undef RELOC

    // patch in the required data
    ap_trampoline_data.cr3  = ap_page_table;
    ap_trampoline_data.data_ptr = global_ap_data;
    if (mmu_config & MMU_CONFIG_L5) ap_trampoline_data.config |= AP_TRAMPOLINE_CONF_LA57;
    if (mmu_config & MMU_CONFIG_NX) ap_trampoline_data.config |= AP_TRAMPOLINE_CONF_NX;

    // copy the trampoline to the second lomem page and return the phys address of it
    memcpy(TO_HHDM_PTR(trampoline_page), ap_trampoline, PAGE_SIZE);
    return trampoline_page;
}

static spalloc_allocator_t per_ap_data_allocator;

static void setup_global_ap_data() {

    uint64_t global_data_size = sizeof(global_ap_data_t) + sizeof(per_ap_data_t*) * detected_cpus;
    uint64_t global_data_pages = ALIGN_UP(global_data_size, PAGE_SIZE) / PAGE_SIZE;

    uint64_t global_data_vaddr = vmem_alloc(&kernel_vmem_allocator, global_data_pages * PAGE_SIZE, 0);

    for (uint64_t i = 0; i < global_data_pages; i++) {
        uint64_t paddr = pmm_alloc_page();
        paging_map_page(kernel_page_table, global_data_vaddr + i * PAGE_SIZE, paddr, PAGE_KRW, PAGE_NORM);
    }

    global_ap_data = (global_ap_data_t*)global_data_vaddr;
    global_ap_data->internal_core_id_counter = 1;
    global_ap_data->total_cores = detected_cpus;

    bool status = spalloc_init(&per_ap_data_allocator, sizeof(per_ap_data_t), _Alignof(per_ap_data_t));
    assert(status == true);

    uint64_t cpulocal_size = (uint64_t)__cpu_local_end - (uint64_t)__cpu_local_start;
    uint64_t cpulocal_pages = cpulocal_size / PAGE_SIZE;

    for (uint32_t i = 1; i < detected_cpus; i++) {
        per_ap_data_t* per_data = spalloc_malloc(&per_ap_data_allocator);

        uint64_t stack_paddr = pmm_alloc_page();
        per_data->sp = TO_HHDM(stack_paddr) + PAGE_SIZE;

        if (cpulocal_pages == 1) {
            uint64_t cpulocal_vaddr = TO_HHDM(pmm_alloc_page());
            memcpy((void*)cpulocal_vaddr, (void*)__cpu_local_start, cpulocal_size);
            // setup the cpulocal self pointer
            *(uintptr_t*)cpulocal_vaddr = cpulocal_vaddr;
            per_data->cpulocal_base = cpulocal_vaddr - (uint64_t)__cpu_local_start;
        } else {
            uint64_t cpulocal_vaddr = vmem_alloc(&kernel_vmem_allocator, cpulocal_size, 0);
            for (uint64_t p = 0; p < cpulocal_pages; p++) {
                uint64_t paddr = pmm_alloc_page();
                paging_map_page(kernel_page_table, cpulocal_vaddr + p * PAGE_SIZE, paddr, PAGE_KRW, PAGE_NORM);
            }
            memcpy((void*)cpulocal_vaddr, (void*)__cpu_local_start, cpulocal_size);
            // setup the cpulocal self pointer
            *(uintptr_t*)cpulocal_vaddr = cpulocal_vaddr;
            per_data->cpulocal_base = cpulocal_vaddr - (uint64_t)__cpu_local_start;
        }

        global_ap_data->per_ap_data_ptrs[i] = per_data;
    }
}

void arch_init_aps() {
    if (detected_cpus <= 1) {
        LOG_TAGGED("AP/STARTUP", ANSI_BYELLOW, "No APs to start")
        return;
    }

    setup_global_ap_data();
    uint64_t trampoline_page = setup_trampoline();
    uint32_t bsp_apic_id = arch_get_local_coreid();

    LOG_TAGGED("AP/INIT", ANSI_BYELLOW, "Sending INIT IPIs to %d APs", detected_cpus - 1)
    RBTREE_FOR_EACH(detected_apics, node) {
        detected_apic_t* apic = CONTAINER_OF(node, detected_apic_t, node);
        if (apic->apic_id != bsp_apic_id)
            x86_send_init(apic->apic_id);
    }

    LOG_TAGGED("AP/INIT", ANSI_BYELLOW, "Waiting to send SIPIs")
    timer_spin_wait_ms(10);

    LOG_TAGGED("AP/INIT", ANSI_BYELLOW, "Sending SIPI IPIs to %d APs", detected_cpus - 1)
    RBTREE_FOR_EACH(detected_apics, node) {
        detected_apic_t* apic = CONTAINER_OF(node, detected_apic_t, node);
        if (apic->apic_id != bsp_apic_id)
            x86_send_sipi(apic->apic_id, trampoline_page / PAGE_SIZE);
    }
}

void x86_ap_entry(uint32_t core_id) {
    LOG_TAGGED("AP/STARTUP", ANSI_BYELLOW, "AP %d started", core_id);

    per_ap_data_t* per_data = global_ap_data->per_ap_data_ptrs[core_id];
    SET_CPU_LOCAL(per_data->cpulocal_base);

    setup_timers(core_id);

    hcf();
}
