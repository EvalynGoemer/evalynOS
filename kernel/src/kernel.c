#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <arch/intrin/interrupts.h>
#include <arch/generic/cpu/ap.h>
#include <arch/generic/panic.h>
#include <arch/generic/init.h>
#include <arch/generic/paging/paging.h>
#include <mem/memmap.h>
#include <mem/pmm.h>
#include <mem/vmem.h>
#include <mem/freelist_pmm.h>
#include <mem/pfndb.h>
#include <utils/misc/build_id.h>
#include <utils/lib.h>
#include <utils/limine.h>
#include <sched/scheduler.h>
#include <arch/generic/thread/init.h>
#include <arch/intrin/cpulocal.h>
#include <utils/dstruct/llist.h>
#include <utils/locks/irqlock.h>

static void test_thread(int n) {
    while (1) {
        printf("test thread %d running\n", n);
        wfi();
    }
}

static thread_t test_threads[3];

void kmain() {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)
        hcf();

    arch_bootstrap_init();
    arch_earlycon_init();

    printf(ANSI_CLEAR ANSI_HOME);
    LOG("EvalynOS Started");
    print_build_info();

    arch_early_init();

    early_sched_init();

    memmap_print();
    freelist_pmm_init();

    paging_init();
    pfndb_init();
    vmem_init();

    arch_post_mm_init();
    arch_init_aps();

    irqlock_t* lock = (irqlock_t*)CPU_LOCAL_GET_SCHED_LOCK_PTR();
    int lock1r = irqlock_lock(lock);

    for (unsigned int i = 0; i < ARRAY_SIZE(test_threads); i++) {
        uintptr_t paddr = (uintptr_t)pmm_alloc_page();
        test_threads[i].kstack_alloc_base = paddr;
        test_threads[i].kstack_alloc_size = PAGE_SIZE;
        test_threads[i].kstack = arch_prepare_thread_stack(TO_HHDM(paddr) + PAGE_SIZE, (uintptr_t)test_thread, i);
        test_threads[i].state = THREAD_RUNABLE;
        llist_push_back(CPU_LOCAL_GET_RUN_QUEUE_PTR(), &test_threads[i].node);
    }

    irqlock_unlock(lock, lock1r);

    LOG("Starting Scheduler");
    while (1) {
        printf("idle/bsp thread running\n");
        #ifdef __x86_64__
        asm volatile ("int $0xfa");
        asm volatile ("int $0xfa");
        asm volatile ("int $0xfa");
        asm volatile ("int $0xfa");
        asm volatile ("int $0xfa");
        wfi();
        schedule();
        #elif __riscv
        asm volatile ("ebreak");
        asm volatile ("ebreak");
        asm volatile ("ebreak");
        asm volatile ("ebreak");
        asm volatile ("ebreak");
        wfi();
        #elif __loongarch64
        asm volatile ("break 0");
        asm volatile ("break 0");
        asm volatile ("break 0");
        asm volatile ("break 0");
        asm volatile ("break 0");
        wfi();
        #endif
    }

    hcf();
}
