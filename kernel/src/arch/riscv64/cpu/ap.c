#include "acpi/tables/madt.h"
#include "arch/riscv64/cpu/ap.h"
#include "arch/generic/panic.h"
#include "arch/generic/timer/timer.h"
#include "arch/riscv64/cpu/interrupts.h"
#include "arch/riscv64/sbi/probe.h"
#include "arch/riscv64/sbi/sbi.h"
#include "mem/pmm.h"
#include "sched/scheduler.h"
#include "utils/dstruct/llist.h"
#include "stdlib.h"
#include "utils/lib.h"
#include <stdint.h>
#include <stdio.h>
#include <arch/riscv64/sbi/hsm.h>
#include <arch/intrin/interrupts.h>
#include <arch/generic/paging/paging.h>
#include <arch/generic/thread/exit.h>
#include <arch/generic/thread/new.h>

llist_t detected_harts = LLIST_INIT;

static uint64_t setup_trampoline() {
    uint64_t vbase = executable_address_request.response->virtual_base;
    uint64_t pbase = executable_address_request.response->physical_base;
    uint64_t trampoline_page = (uint64_t)ap_trampoline - vbase + pbase;
    paging_map_page(kernel_page_table, trampoline_page, trampoline_page, PAGE_KRWX);

    ap_trampoline_data.satp = kernel_page_table >> 12;
    switch (MMU_CONFIG_TOP_LEVEL(mmu_config)) {
        case 3: ap_trampoline_data.satp |= RISCV_SATP_SV39 << 60; break;
        case 4: ap_trampoline_data.satp |= RISCV_SATP_SV48 << 60; break;
        case 5: ap_trampoline_data.satp |= RISCV_SATP_SV57 << 60; break;
        default: UNREACHABLE();
    }

    return trampoline_page;
}

static uint64_t start_time = 0;

void arch_init_aps() {
    if (detected_cpus <= 1) {
        LOG_TAGGED("AP/STARTUP", ANSI_BYELLOW, "No APs to start")
        return;
    }

    if (!sbi_probe_extension(SBI_HART_START.eid))
        panic("Unable to start other APs (HSM is not implemented by firmware)");

    LOG_TAGGED("AP/INIT", ANSI_BYELLOW, "Starting %d HART(s)", detected_cpus - 1)

    start_time = timer_get_ns();
    uint64_t trampoline_page = setup_trampoline();

    uint32_t i = 1;
    LLIST_FOR_EACH(detected_harts, node) {
        detected_hart_t* hart = CONTAINER_OF(node, detected_hart_t, node);
        if (hart->hart_id == riscv_bsp_hartid_request.response->bsp_hartid)
            continue;

        // allocate the init stack and use it as space for the per ap data
        uint64_t stack_paddr = pmm_alloc_page();
        uint64_t sp = TO_HHDM(stack_paddr + PAGE_SIZE - sizeof(per_ap_data_t));
        per_ap_data_t* data = (void*)sp;

        // setup the cores cpulocal
        uint64_t cpulocal_vaddr;
        uint64_t cpulocal_size = (uint64_t)__cpu_local_end - (uint64_t)__cpu_local_start;
        uint64_t cpulocal_pages = cpulocal_size / PAGE_SIZE;
        if (cpulocal_pages == 1) {
            cpulocal_vaddr = TO_HHDM(pmm_alloc_page());
        } else {
            cpulocal_vaddr = vmem_alloc(&kernel_vmem_allocator, cpulocal_size, 0);
            for (uint64_t p = 0; p < cpulocal_pages; p++) {
                uint64_t paddr = pmm_alloc_page();
                paging_map_page(kernel_page_table, cpulocal_vaddr + p * PAGE_SIZE, paddr, PAGE_KRW);
            }
        }

        thread_t* thread = malloc(sizeof(thread_t));
        memset(thread, 0, sizeof(thread_t));
        thread->kstack_top  = TO_HHDM(stack_paddr + PAGE_SIZE);
        thread->kstack_size = PAGE_SIZE;
        thread->state = THREAD_RUNNING;
        data->init_thread = thread;

        data->id = i++;
        data->tp = cpulocal_vaddr;
        data->idle_thread_stack = TO_HHDM(pmm_alloc_page() + PAGE_SIZE);

        bool status = sbi_hart_start(hart->hart_id, trampoline_page, sp);
        if (!status) panic("Failed to start an AP");
    }
}

void arch_reclaim_ap_memory() {
    if (detected_cpus <= 1)
        return;

    uint64_t vbase = executable_address_request.response->virtual_base;
    uint64_t pbase = executable_address_request.response->physical_base;
    uint64_t trampoline_page = (uint64_t)ap_trampoline - vbase + pbase;
    paging_unmap_range(kernel_page_table, trampoline_page, PAGE_SIZE);
}

void riscv64_ap_entry(MAYBE_UNUSED size_t hart_id, per_ap_data_t* per_data) {
    LOG_TAGGED("AP/STARTUP", ANSI_BYELLOW, "AP %d started", per_data->id);

    early_sched_init_ap(per_data->init_thread, per_data->idle_thread_stack, PAGE_SIZE);
    setup_interrupts();

    // enable the FPU
    csrs(CSR_SSTATUS, 3ull << 13);

    // setup the timer
    timer_set_timeout_ms(1);
    csrs(CSR_SIE, SIE_STIE);

    LOG_TAGGED("AP/STARTUP", ANSI_BYELLOW, "AP %d finished setup in %lld ns", per_data->id, timer_get_ns() - start_time);
    thread_exit();
}
