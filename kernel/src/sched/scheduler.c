#include <assert.h>
#include <arch/generic/paging/paging.h>
#include <mem/address_space.h>
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

void enqueue_thread(thread_t* thread) {
    int irqs = interrupts_enabled();
    disable_interrupts();
    spinlock_t* lock = CPU_LOCAL_GET_SCHED_LOCK_PTR();
    spinlock_lock(lock);
    llist_push_back(CPU_LOCAL_GET_RUN_QUEUE_PTR(), &thread->node);
    spinlock_unlock(lock);
    restore_interrupts(irqs);
}

bool schedule() {
    thread_t* current = CPU_LOCAL_GET_CURRENT_THREAD();
    if (current->preempt_disable_counter > 0)
        return false;

    disable_interrupts();

    spinlock_t* lock = CPU_LOCAL_GET_SCHED_LOCK_PTR();
    spinlock_lock(lock);

    llist_t* queue = CPU_LOCAL_GET_RUN_QUEUE_PTR();

    llist_node_t* next_node = llist_pop_front(queue);

    if (next_node == nullptr) {
        spinlock_unlock(lock);
        enable_interrupts();
        return false;
    }

    thread_t* next = CONTAINER_OF(next_node, thread_t, node);

    assert(next != current);

    arch_thread_switch(current, next);
    return true;
}

void schedule_finalize(thread_t* prev, thread_t* next) {
    CPU_LOCAL_SET_CURRENT_THREAD(next);
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
        //
        case THREAD_BLOCKED: panic("scheduler reached invalid state");
        // TODO: add to the reaper threads queue when a thread is in this state
        case THREAD_REAPING: break;
    }

    // is a user thread
    if (next->addrspace != nullptr) {
        arch_load_page_table(next->addrspace->pagetable);
        arch_finalize_user_switch(next);
    }
}
