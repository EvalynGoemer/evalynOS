#include <arch/generic/timer/timer.h>
#include <arch/generic/thread/exit.h>
#include "acpi/tables/madt.h"
#include "arch/loongarch64/cpu/fpu/fpu.h"
#include "arch/loongarch64/timer/timer.h"
#include <arch/loongarch64/cpu/interrupts.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched/scheduler.h>
#include <arch/generic/paging/paging.h>
#include <utils/dstruct/llist.h>
#include <arch/loongarch64/cpu/ap.h>
#include <arch/loongarch64/cpu/tlb/tlb.h>
#include <arch/loongarch64/intrin/iocsr.h>
#include <mem/pmm.h>
#include <mem/pfndb.h>
#include <mem/vmem.h>

llist_t detected_cores = LLIST_INIT;
static uint64_t start_time = 0;

static uint64_t setup_trampoline() {
    uint64_t vbase = executable_address_request.response->virtual_base;
    uint64_t pbase = executable_address_request.response->physical_base;
    uint64_t trampoline_page = (uint64_t)ap_trampoline - vbase + pbase;
    ap_trampoline_data.tlb_handler = (uint64_t)tlb_refill_handler - vbase + pbase;
    ap_trampoline_data.page_tables = kernel_page_table;

    paging_map_page(kernel_page_table, trampoline_page, trampoline_page, PAGE_KRWX);
    return trampoline_page;
}

void arch_reclaim_ap_memory() {
    if (detected_cpus <= 1)
        return;

    uint64_t vbase = executable_address_request.response->virtual_base;
    uint64_t pbase = executable_address_request.response->physical_base;
    uint64_t trampoline_page = (uint64_t)ap_trampoline - vbase + pbase;
    paging_unmap_range(kernel_page_table, trampoline_page, PAGE_SIZE);
}

static void start_cpu(uint32_t core_id) {
    uint32_t value = (1ull << 31) | (core_id << 16);
    iocsr_write_32(IOCSR_IPI_SEND, value);
}

static void mailbox_send(uint32_t core_id, uint32_t data, uint8_t idx) {
    uint64_t value = 1ull << 31;
    value |= idx << 2;
    value |= core_id << 16;
    value |= (uint64_t)data << 32;
    iocsr_write_64(IOCSR_IPI_MAIL_SEND, value);
}

static void mailbox_send_64(uint32_t core_id, uint64_t data, uint8_t idx) {
    mailbox_send(core_id, data >> 32, idx * 2 + 1);
    mailbox_send(core_id, data, idx * 2);
}

void arch_init_aps() {
    if (detected_cpus <= 1) {
        LOG_TAGGED("AP/STARTUP", ANSI_BYELLOW, "No APs to start")
        return;
    }

    LOG_TAGGED("AP/INIT", ANSI_BYELLOW, "Starting %d Core(s)", detected_cpus - 1)

    start_time = timer_get_ns();
    uint64_t trampoline_page = setup_trampoline();

    uint32_t i = 1;
    LLIST_FOR_EACH(detected_cores, node) {
        detected_core_t* core = CONTAINER_OF(node, detected_core_t, node);
        if (core->bsp) continue;

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

        mailbox_send_64(core->core_id, trampoline_page, 0);
        mailbox_send_64(core->core_id, sp, 3);
        start_cpu(core->core_id);
    }
}

void loongarch64_ap_entry(per_ap_data_t* per_data) {
    LOG_TAGGED("AP/STARTUP", ANSI_BYELLOW, "AP %d started", per_data->id);

    early_sched_init_ap(per_data->init_thread, per_data->idle_thread_stack, PAGE_SIZE);
    setup_interrupts();

    setup_fpu(false);

    timer_set_timeout_ms(1);
    csrxchg(CSR_ECFG, CSR_ECFG_TIMER_EN, CSR_ECFG_TIMER_EN);

    LOG_TAGGED("AP/STARTUP", ANSI_BYELLOW, "AP %d finished setup in %lld ns", per_data->id, timer_get_ns() - start_time);

    thread_exit();
}
