#include "mem/address_space.h"
#include "mem/pmm.h"
#include "sched/scheduler.h"
#include "sched/wait.h"
#include "utils/dstruct/llist.h"
#include "utils/lib.h"
#include "utils/locks/irqlock.h"
#include <arch/generic/thread/new.h>
#include <stdint.h>
#include <stdlib.h>

llist_t reaper_list = {0};
waitable_t reaper_waiter = {0};

static bool reaper_has_work(void* ctx) {
    UNUSED(ctx);
    return reaper_list.head != nullptr;
}

static void reaper_thread() {
    while (true) {
        wait_on_cond(&reaper_waiter, reaper_has_work, nullptr, 0);
        while (true) {
            int lock1r = irqlock_lock(&reaper_waiter.lock);
            llist_node_t* node = llist_pop(&reaper_list);
            irqlock_unlock(&reaper_waiter.lock, lock1r);

            if (node == nullptr) break;;

            thread_t* thread = CONTAINER_OF(node, thread_t, node);
            if (thread->addrspace != nullptr)
                free_address_space(thread->addrspace);

            // TODO: dont hard code the kernel stacks size
            pmm_free_page(FROM_HHDM(thread->kstack_top - PAGE_SIZE));

            // TODO: have an arch spesific hook for cleaning up FPU save area etc

            free(thread);
        }
    }
}

void init_reaper() {
    thread_t* thread = create_kthread((uintptr_t)reaper_thread, 0, 0);
    enqueue_thread(thread);
}
