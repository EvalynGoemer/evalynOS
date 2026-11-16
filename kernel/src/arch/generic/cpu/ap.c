#include <arch/generic/timer/timer.h>
#include <arch/generic/cpu/ap.h>
#include <sched/scheduler.h>
#include <sched/wait.h>
#include <stdint.h>
#include <arch/intrin/cpulocal.h>
#include <arch/generic/paging/paging.h>
#include <arch/generic/thread/new.h>
#include <arch/generic/thread/exit.h>
#include <mem/pmm.h>
#include <mem/vmem.h>
#include <mem/reclaim.h>

#include <stdio.h>
#include <stdlib.h>

uint32_t total_cpus = 0;
uint32_t started_cpus = 0;
uint64_t ap_init_start_time = 0;

void setup_aps() {
    ap_init_start_time = timer_get_ns();

    if (total_cpus <= 1) {
        LOG_TAGGED("AP/STARTUP", ANSI_BYELLOW, "No APs to start")
    } else {
        LOG_TAGGED("AP/INIT", ANSI_BYELLOW, "Starting %d Core(s)", total_cpus - 1)
        arch_init_aps();

        // wait for APs to start with a dynamic timeout
        uint32_t started = 0;
        for (int i = 100; i > 0; i--) {
            uint32_t old = started;
            atomic_ptr_load32(&started_cpus, &started, AMO_ACQUIRE);
            if (started == total_cpus) break;
            if (started > old) i += 10;
            sched_wait_on(nullptr, 25000000);
        }

        // TODO; have a way to track which CPUs did not start to remove them from use
        if (started != total_cpus) {
            panic("Failed to start all APs");
        } else {
            LOG_TAGGED("AP/STARTUP", ANSI_BYELLOW, "All APs started in %llu ns", timer_get_ns() - ap_init_start_time)
            arch_reclaim_ap_memory();
        }
    }

    reclaim_boot_memory();

    // TODO: when logging is better spawn this in kmain
    thread_t* thread = create_uthread_from_elf("./usr/bin/core.elf");
    thread->syscall_perms = UINT64_MAX;
    enqueue_thread(thread);

    thread_exit();
}

uint64_t allocate_cpu_local() {
    uint64_t cpulocal_vaddr;
    uint64_t cpulocal_size = (uint64_t)__cpu_local_end - (uint64_t)__cpu_local_start;
    uint64_t cpulocal_pages = cpulocal_size / PAGE_SIZE;

    if (cpulocal_pages > 1) {
        cpulocal_vaddr = vmem_alloc(&kernel_vmem_allocator, cpulocal_size, 0);
        for (uint64_t p = 0; p < cpulocal_pages; p++) {
            uint64_t paddr = pmm_alloc_page();
            paging_map_page(kernel_page_table, cpulocal_vaddr + p * PAGE_SIZE, paddr, PAGE_KRW);
        }
    } else {
        cpulocal_vaddr = TO_HHDM(pmm_alloc_page());
    }

    memcpy((void*)cpulocal_vaddr, __cpu_local_start, cpulocal_size);
    return cpulocal_vaddr;
}

thread_t* allocate_init_thread() {
    uint64_t stack = TO_HHDM(pmm_alloc_page() + PAGE_SIZE);
    thread_t* thread = malloc(sizeof(thread_t));
    memset(thread, 0, sizeof(thread_t));
    thread->kstack = stack;
    thread->kstack_top = stack;
    thread->kstack_size = PAGE_SIZE;
    thread->state = THREAD_RUNNING;
    return thread;
}
