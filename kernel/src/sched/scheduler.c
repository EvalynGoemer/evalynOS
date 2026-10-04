#include "sched/wait.h"
#include <sched/idle.h>
#include <arch/generic/paging/paging.h>
#include <arch/generic/thread/init.h>
#include <mem/address_space.h>
#include <sched/scheduler.h>
#include <sched/reaper.h>
#include <arch/intrin/interrupts.h>
#include <arch/generic/thread/switch.h>
#include <stdint.h>
#include <utils/dstruct/llist.h>
#include <utils/locks/spinlock.h>
#include <arch/intrin/cpulocal.h>

CPU_LOCAL thread_t idle_thread = {};

void early_sched_init_bsp() {
    thread_t* idle = CPU_LOCAL_PTR(idle_thread);
    idle->state = THREAD_IDLE_THREAD;
    CPU_LOCAL_SET_CURRENT_THREAD(idle);
}

void early_sched_init_ap(thread_t* init_thread, uint64_t idle_stack, uint64_t idle_stack_size) {
    init_thread->state = THREAD_RUNNING;
    CPU_LOCAL_SET_CURRENT_THREAD(init_thread);

    thread_t* idle = CPU_LOCAL_PTR(idle_thread);
    idle->state = THREAD_IDLE_THREAD;
    idle->kstack_top = idle_stack;
    idle->kstack_size = idle_stack_size;
    idle->kstack = arch_prepare_thread_stack(idle_stack, (uintptr_t)idle_thread_entry, 0, 0);
}

// should only be called by the BSP
void sched_init() {
    init_reaper();
}

void enqueue_thread(thread_t* thread) {
    int irqs = interrupts_enabled();
    disable_interrupts();
    spinlock_t* lock = CPU_LOCAL_GET_SCHED_LOCK_PTR();
    spinlock_lock(lock);
    llist_push_back(CPU_LOCAL_GET_RUN_QUEUE_PTR(), &thread->node);
    spinlock_unlock(lock);
    restore_interrupts(irqs);
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

    // no valid next thread go to idle
    if (next_node == nullptr) {
        thread_t* idle = CPU_LOCAL_PTR(idle_thread);
        // if we were already idling return
        if (current == idle) {
            spinlock_unlock(lock);
            enable_interrupts();
            return;
        }
        arch_thread_switch(current, idle);
        return;
    }

    thread_t* next = CONTAINER_OF(next_node, thread_t, node);

    arch_thread_switch(current, next);
    return;
}

void schedule_finalize(thread_t* prev, thread_t* next) {
    CPU_LOCAL_SET_CURRENT_THREAD(next);
    if (__atomic_load_n(&next->state, __ATOMIC_RELAXED) != THREAD_IDLE_THREAD)
        __atomic_store_n(&next->state, THREAD_RUNNING, __ATOMIC_RELAXED);

    switch (__atomic_load_n(&prev->state, __ATOMIC_RELAXED)) {
        // if the thread was running it should always be added back to the run queue
        case THREAD_RUNNING: {
            __atomic_store_n(&prev->state, THREAD_RUNABLE, __ATOMIC_RELAXED);
            llist_t* queue = CPU_LOCAL_GET_RUN_QUEUE_PTR();
            llist_push_back(queue, &prev->node);
            break;
        }

        case THREAD_BLOCKING:
        case THREAD_RUNABLE: {
            // attempt to commit the state as blocking; if it was unblocked and
            // the thread has become runnable it needs to be added to the queue
            thread_state_t expected = THREAD_BLOCKING;
            if(!(__atomic_compare_exchange_n(&prev->state, &expected, THREAD_BLOCKED, false, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE))) {
                llist_t* queue = CPU_LOCAL_GET_RUN_QUEUE_PTR();
                llist_push_back(queue, &prev->node);
            }
            break;
        }
        case THREAD_REAPING: {
            int lock1r = irqlock_lock(&reaper_waiter.lock);
            llist_push_back(&reaper_list, &prev->node);
            llist_node_t* node = llist_pop(&reaper_waiter.threads);
            if (node != nullptr) {
                thread_t* reaper = CONTAINER_OF(node, waitable_token_t, linkage)->thread;
                thread_state_t prev_state = __atomic_exchange_n(&reaper->state, THREAD_RUNABLE, __ATOMIC_RELEASE);
                if (prev_state == THREAD_BLOCKED) llist_push_back(CPU_LOCAL_GET_RUN_QUEUE_PTR(), &reaper->node);
            }
            irqlock_unlock(&reaper_waiter.lock, lock1r);
            break;
        }
        case THREAD_IDLE_THREAD: break;
        case THREAD_BLOCKED: panic("scheduler reached invalid state");
    }

    // is a user thread
    if (next->addrspace != nullptr) {
        arch_load_page_table(next->addrspace->pagetable);
        arch_finalize_user_switch(next);
    } else {
        arch_load_page_table(kernel_page_table);
    }
}
