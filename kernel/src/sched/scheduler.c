#include "sched/wait.h"
#include "utils/atomic.h"
#include <sched/idle.h>
#include <arch/generic/paging/paging.h>
#include <arch/generic/thread/init.h>
#include <mem/address_space.h>
#include <sched/scheduler.h>
#include <sched/reaper.h>
#include <arch/intrin/interrupts.h>
#include <arch/generic/thread/switch.h>
#include <stdint.h>
#include <string.h>
#include <utils/dstruct/llist.h>
#include <utils/locks/spinlock.h>
#include <arch/intrin/cpulocal.h>
#include <utils/defer.h>

CPU_LOCAL thread_t idle_thread = {};

void early_sched_init_bsp() {
    thread_t* idle = CPU_LOCAL_PTR(idle_thread);
    idle->state = THREAD_IDLE_THREAD;
    CPU_LOCAL_SET_CURRENT_THREAD(idle);
}

void early_sched_init_ap(thread_t* init_thread, uint64_t idle_stack, uint64_t idle_stack_size) {
    init_thread->state = THREAD_RUNNING;
    CPU_LOCAL_SET_CURRENT_THREAD(init_thread);

    memset(CPU_LOCAL_GET_RUN_QUEUE_PTR(), 0, sizeof(llist_t));
    spinlock_unlock(CPU_LOCAL_GET_SCHED_LOCK_PTR());

    thread_t* idle = CPU_LOCAL_PTR(idle_thread);
    memset(idle, 0, sizeof(thread_t));

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

    int irqs = interrupts_enabled();
    defer restore_interrupts(irqs);

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
            return;
        }
        arch_thread_switch(current, idle);
        return;
    }

    thread_t* next = CONTAINER_OF(next_node, thread_t, node);
    arch_thread_switch(current, next);
    return;
}

static void switch_to_next(thread_t* next) {
    // next is a user thread
    if (next->addrspace != nullptr) {
        arch_fpu_restore(next->fpu_save);
        arch_load_page_table(next->addrspace->pagetable);
        arch_finalize_user_switch(next);
    } else {
        arch_load_page_table(kernel_page_table);
    }
}

void schedule_finalize(thread_t* prev, thread_t* next) {
    CPU_LOCAL_SET_CURRENT_THREAD(next);
    if (atomic_load32(&next->state, AMO_RELAXED) != THREAD_IDLE_THREAD)
        atomic_store32(&next->state, THREAD_RUNNING, AMO_RELAXED);

    switch (atomic_load32(&prev->state, AMO_RELAXED)) {
        // if the thread was running it should always be added back to the run queue
        case THREAD_RUNNING: {
            atomic_store32(&prev->state, THREAD_RUNABLE, AMO_RELAXED);
            llist_t* queue = CPU_LOCAL_GET_RUN_QUEUE_PTR();
            llist_push_back(queue, &prev->node);
            break;
        }

        case THREAD_BLOCKING:
        case THREAD_RUNABLE: {
            // attempt to commit the state as blocking; if it was unblocked and
            // the thread has become runnable it needs to be added to the queue
            thread_state_t expected = THREAD_BLOCKING;
            if(!(atomic_cas32(&prev->state, &expected, THREAD_BLOCKED, AMO_RELEASE, AMO_ACQUIRE))) {
                llist_t* queue = CPU_LOCAL_GET_RUN_QUEUE_PTR();
                llist_push_back(queue, &prev->node);
            }
            break;
        }
        case THREAD_REAPING: {
            // do the higher level context switch first before reaping to
            // not have a UAF with the previous threads data being freed
            switch_to_next(next);

            int lock1r = irqlock_lock(&reaper_waiter.lock);
            llist_push_back(&reaper_list, &prev->node);
            llist_node_t* node = llist_pop(&reaper_waiter.threads);
            if (node != nullptr) {
                thread_t* reaper = CONTAINER_OF(node, waitable_token_t, linkage)->thread;
                thread_state_t prev_state = atomic_xchg32(&reaper->state, THREAD_RUNABLE, AMO_RELEASE);
                if (prev_state == THREAD_BLOCKED) llist_push_back(CPU_LOCAL_GET_RUN_QUEUE_PTR(), &reaper->node);
            }
            irqlock_unlock(&reaper_waiter.lock, lock1r);
            return;
        }
        case THREAD_IDLE_THREAD: break;
        case THREAD_BLOCKED: panic("scheduler reached invalid state");
    }

    // prev was a user thread
    if (prev->addrspace != nullptr) {
        arch_fpu_save(prev->fpu_save);
    }

    switch_to_next(next);
}
