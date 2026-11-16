#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <arch/intrin/interrupts.h>
#include <arch/generic/cpu/ap.h>
#include <arch/generic/panic.h>
#include <arch/generic/init.h>
#include <arch/generic/paging/paging.h>
#include <mem/memmap.h>
#include <mem/pmm.h>
#include <mem/vmem.h>
#include <mem/buddy.h>
#include <mem/pfndb.h>
#include <mem/reclaim.h>
#include <utils/misc/build_id.h>
#include <utils/misc/ustar.h>
#include <utils/lib.h>
#include <utils/limine.h>
#include <utils/defer.h>
#include <sched/scheduler.h>
#include <sched/idle.h>
#include <sched/wait.h>
#include <arch/generic/thread/switch.h>
#include <arch/generic/thread/init.h>
#include <arch/generic/thread/exit.h>
#include <arch/generic/thread/new.h>
#include <arch/intrin/cpulocal.h>
#include <utils/dstruct/llist.h>
#include <utils/locks/irqlock.h>
#include <utils/atomic.h>

void kmain() {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)
        hcf();

    limine_save_requests();
    arch_bootstrap_init();
    arch_earlycon_init();

    printf(ANSI_CLEAR ANSI_HOME);
    LOG("EvalynOS Started");
    print_build_info();

    arch_early_init();
    early_sched_init_bsp();

    memmap_print();
    paging_init();
    pfndb_init();
    buddy_init();
    vmem_init();
    malloc_init();
    sched_init();

    arch_post_mm_init();

    uint64_t idle_stack = TO_HHDM(pmm_alloc_page()) + PAGE_SIZE;
    defer arch_thread_pivot((uintptr_t)idle_thread_entry, idle_stack);

    thread_t* thread = create_kthread((uintptr_t)setup_aps, 0, 0);
    enqueue_thread(thread);
}
