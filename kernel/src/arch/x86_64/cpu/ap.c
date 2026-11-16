#include <arch/generic/cpu/ap.h>
#include <arch/x86_64/cpu/CRx.h>
#include <arch/x86_64/descriptor_tables/gdt.h>
#include <arch/x86_64/descriptor_tables/idt.h>
#include <arch/x86_64/drivers/fred/fred.h>
#include <arch/x86_64/syscall/syscall.h>
#include <sched/scheduler.h>
#include <sched/wait.h>
#include <arch/generic/thread/new.h>
#include <arch/generic/thread/exit.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <utils/lib.h>
#include <arch/x86_64/cpu/ap.h>
#include <utils/limine.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/acpi/madt.h>
#include <arch/generic/paging/paging.h>
#include <mem/pmm.h>
#include <mem/vmem.h>
#include <arch/x86_64/timer/timer.h>
#include <arch/intrin/cpulocal.h>

global_ap_data_t* global_ap_data;

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

static uint64_t setup_trampoline() {
    // make a 2mb identity map with the first lomem page
    uint64_t ap_page_table = get_nth_lomem_page(0);
    assert(ap_page_table != 0);
    memcpy(TO_HHDM_PTR(ap_page_table), TO_HHDM_PTR(kernel_page_table), PAGE_SIZE);
    paging_map_page(ap_page_table, 0, 0, PAGE_KRWX | PAGE_LARGE);

    // find the second lowmem page for the trampoline its self
    uint64_t trampoline_page = get_nth_lomem_page(1);
    assert(trampoline_page != 0);

    // relocate the trampoline
    uint64_t trampoline_virt = (uint64_t)ap_trampoline;
    #define RELOC(dst, sym) (dst) = trampoline_page + ((uint64_t)(sym) - trampoline_virt)
    RELOC(ap_trampoline_data.gdtr_base, ap_trampoline_gdt);
    RELOC(ap_trampoline_data.fjmp32_addr,  ap_trampoline32);
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

static void setup_global_ap_data() {
    uint64_t global_data_size = ALIGN_UP(sizeof(global_ap_data_t) + (total_cpus * sizeof(uintptr_t)), PAGE_SIZE);
    uint64_t global_data_vaddr = vmem_alloc(&kernel_vmem_allocator, global_data_size, 0);

    for (uint64_t i = 0; i < global_data_size / PAGE_SIZE; i++)
        paging_map_page(kernel_page_table, global_data_vaddr + i * PAGE_SIZE, pmm_alloc_page(), PAGE_KRW);

    global_ap_data = (global_ap_data_t*)global_data_vaddr;
    global_ap_data->internal_core_id_counter = 1;
    global_ap_data->size = global_data_size;

    for (uint32_t i = 1; i < total_cpus; i++) {
        per_ap_data_t* per_data = kmalloc(sizeof(per_ap_data_t));

        thread_t* init_thread = allocate_init_thread();
        per_data->init_thread = init_thread;
        per_data->init_thread_stack = init_thread->kstack;
        per_data->idle_thread_stack = TO_HHDM(pmm_alloc_page()) + PAGE_SIZE;

        uint64_t cpulocal_vaddr = allocate_cpu_local();
        *(uintptr_t*)cpulocal_vaddr = cpulocal_vaddr; // setup self pointer
        per_data->cpulocal_base = cpulocal_vaddr - (uint64_t)__cpu_local_start;

        global_ap_data->per_ap_data_ptrs[i] = per_data;
    }
}

void arch_reclaim_ap_memory() {
    uint64_t ap_page_table = get_nth_lomem_page(0);
    paging_unmap_range(ap_page_table, 0, PAGE_SIZE_LARGE);

    for (uint32_t i = 1; i < total_cpus; i++)
        kfree(global_ap_data->per_ap_data_ptrs[i], sizeof(per_ap_data_t));

    uint64_t global_data_vaddr = (uint64_t)global_ap_data;
    uint64_t global_data_size = global_ap_data->size;

    for (uint64_t i = 0; i < global_data_size / PAGE_SIZE; i++) {
        uint64_t paddr = paging_get_paddr(kernel_page_table, global_data_vaddr + i * PAGE_SIZE, nullptr);
        pmm_free_page(paddr);
    }

    paging_unmap_range(kernel_page_table, global_data_vaddr, global_data_size);
    vmem_free(&kernel_vmem_allocator, global_data_vaddr, global_data_size);
    global_ap_data = nullptr;
}

void arch_init_aps() {
    setup_global_ap_data();
    uint64_t trampoline_page = setup_trampoline();
    uint32_t bsp_apic_id = arch_get_local_coreid();

    LOG_TAGGED("AP/INIT", ANSI_BYELLOW, "Sending INIT IPIs");
    RBTREE_FOR_EACH(detected_apics, node) {
        detected_apic_t* apic = CONTAINER_OF(node, detected_apic_t, node);
        if (apic->apic_id != bsp_apic_id)
            x86_send_init(apic->apic_id);
    }

    LOG_TAGGED("AP/INIT", ANSI_BYELLOW, "Waiting to send SIPIs")
    sched_wait_on(nullptr, 10000000);

    LOG_TAGGED("AP/INIT", ANSI_BYELLOW, "Sending SIPI IPIs");
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

    early_sched_init_ap(per_data->init_thread, per_data->idle_thread_stack, PAGE_SIZE);
    setup_ap_gdt(per_data->cpulocal_base);

    if (fred_enabled)
        setup_fred_ap();
    else
        setup_ap_idt();

    setup_control_regs(false);
    setup_syscall();

    setup_timers(core_id);
    setup_lapic_ap();

    LOG_TAGGED("AP/STARTUP", ANSI_BYELLOW, "AP %d finished setup in %lld ns", core_id, timer_get_ns() - ap_init_start_time);

    thread_exit();
}
