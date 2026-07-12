#include <assert.h>
#include <sched/scheduler.h>
#include <arch/intrin/interrupts.h>
#include <arch/generic/thread/switch.h>
#include <utils/dstruct/llist.h>
#include <utils/locks/spinlock.h>
#include <arch/intrin/cpulocal.h>

CPU_LOCAL thread_t idle_thread = {};

void early_sched_init() {
    thread_t* idle = CPU_LOCAL_PTR(idle_thread);
    idle->state = THREAD_RUNNING;
    CPU_LOCAL_SET_CURRENT_THREAD(idle);
}

void schedule() {
    thread_t* current = CPU_LOCAL_GET_CURRENT_THREAD();
    if (current->preempt_disable_counter > 0)
        return;

    disable_interrupts();

    spinlock_t* lock = CPU_LOCAL_GET_SCHED_LOCK_PTR();
    spinlock_lock(lock);

    llist_t* queue = CPU_LOCAL_GET_RUN_QUEUE_PTR();

    llist_node_t* next_node = llist_pop_front(queue);

    if (next_node == nullptr) {
        spinlock_unlock(lock);
        enable_interrupts();
        return;
    }

    thread_t* next = CONTAINER_OF(next_node, thread_t, node);

    assert(next != current);

    arch_thread_switch(current, next);
}

void schedule_finalize(thread_t* prev, thread_t* next) {
    CPU_LOCAL_SET_CURRENT_THREAD(next);
    next->state = THREAD_RUNNING;

    if (prev->state == THREAD_RUNNING) {
        prev->state = THREAD_RUNABLE;
        llist_t* queue = CPU_LOCAL_GET_RUN_QUEUE_PTR();
        llist_push_back(queue, &prev->node);
    }
}
