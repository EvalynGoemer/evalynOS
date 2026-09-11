#include <arch/generic/thread/switch.h>
#include <stdbool.h>
#include <stdio.h>

#include <stdlib.h>
#include <utils/misc/ustar.h>
#include <loader/elf.h>

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
#include <utils/misc/build_id.h>
#include <utils/lib.h>
#include <utils/limine.h>
#include <sched/scheduler.h>
#include <arch/generic/thread/init.h>
#include <arch/generic/thread/new.h>
#include <arch/intrin/cpulocal.h>
#include <utils/dstruct/llist.h>
#include <utils/locks/irqlock.h>

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
    paging_init();
    pfndb_init();
    buddy_init();
    vmem_init();
    malloc_init();

    arch_post_mm_init();
    arch_init_aps();

    LOG("Starting Scheduler");

    thread_t* thread = create_uthread_from_elf("./helloworld.elf");
    enqueue_thread(thread);

    while (true)
        wfi();
    hcf();
}
