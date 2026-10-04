#include <assert.h>
#include <sched/wait.h>
#include <arch/generic/timer/timer.h>
#include <utils/lib.h>
#include <stdint.h>
#include <utils/dstruct/pheap.h>
#include <utils/locks/irqlock.h>
#include <sched/preempt.h>
#include <sched/scheduler.h>
#include <utils/dstruct/llist.h>

static uint64_t sleeping_threads_von(pheap_node_t* node) {
    thread_t* thread = CONTAINER_OF(node, thread_t, timeout_pheap_node);
    return thread->wakeup_time;
}

static pheap_t timeouts = {.value_of_node = sleeping_threads_von, .head = nullptr};
static irqlock_t timeouts_lock = {0};

static wait_status_t wait_impl(waitable_t* waitable_object, wait_cond_t cond, void* ctx, uint64_t timeout) {
    assert(waitable_object != nullptr || timeout != 0);

    thread_t* cur = CPU_LOCAL_GET_CURRENT_THREAD();
    assert(cur->preempt_disable_counter == 0);

    disable_preemption();

    // set our state to be blocking in progress
    __atomic_store_n(&cur->state, THREAD_BLOCKING, __ATOMIC_RELAXED);

    waitable_token_t token = {0};
    token.thread = cur;

    // if there is a waitable object put ourselves on it
    if (waitable_object != nullptr) {
        int lock1r = irqlock_lock(&waitable_object->lock);

        // the condition is met; dont block
        if (cond != nullptr && cond(ctx)) {
            irqlock_unlock(&waitable_object->lock, lock1r);
            __atomic_store_n(&cur->state, THREAD_RUNNING, __ATOMIC_RELAXED);
            enable_preemption();
            return WAIT_STATUS_CONDITION_MET;
        }

        llist_push_back(&waitable_object->threads, &token.linkage);
        irqlock_unlock(&waitable_object->lock, lock1r);
    }

    // if there is a timeout add it to the timeout pheap
    if (timeout != 0) {
        cur->wakeup_time = timeout + timer_get_ns();
        int lock1r = irqlock_lock(&timeouts_lock);
        pheap_insert(&timeouts, &cur->timeout_pheap_node);
        irqlock_unlock(&timeouts_lock, lock1r);
    }

    // reschedule and preempt away
    enable_preemption();
    schedule();

    // we have now woken up and need to cleanup
    // use the "try" methods because something else may have already removed us

    bool timed_out = false;
    if (timeout != 0) {
        int lock1r = irqlock_lock(&timeouts_lock);
        if (pheap_try_remove(&timeouts, &cur->timeout_pheap_node) == nullptr)
            timed_out = true;
        irqlock_unlock(&timeouts_lock, lock1r);
    }

    if (waitable_object != nullptr) {
        int lock1r = irqlock_lock(&waitable_object->lock);
        timed_out = llist_node_try_delete(&waitable_object->threads, &token.linkage);
        irqlock_unlock(&waitable_object->lock, lock1r);
    }

    return timed_out ? WAIT_STATUS_TIMED_OUT : WAIT_STATUS_WOKEN;
}

wait_status_t sched_wait_on(waitable_t* waitable_object, uint64_t timeout) {
    return wait_impl(waitable_object, nullptr, nullptr, timeout);
}

wait_status_t wait_on_cond(waitable_t* waitable_object, wait_cond_t cond, void* ctx, uint64_t timeout) {
    assert(waitable_object != nullptr);
    assert(cond != nullptr);
    return wait_impl(waitable_object, cond, ctx, timeout);
}

int sched_wakeup_n_threads(waitable_t* waitable_object, int threads) {
    int woken = 0;
    while (threads > woken) {
        disable_preemption();
        int lock1r = irqlock_lock(&waitable_object->lock);
        llist_node_t* node = llist_pop_front(&waitable_object->threads);

        // no more threads to wakeup
        if (!node) {
            irqlock_unlock(&waitable_object->lock, lock1r);
            enable_preemption();
            break;
        }

        thread_t* thread = CONTAINER_OF(node, waitable_token_t, linkage)->thread;
        irqlock_unlock(&waitable_object->lock, lock1r);
        enable_preemption();

        thread_state_t prev = __atomic_exchange_n(&thread->state, THREAD_RUNABLE, __ATOMIC_RELEASE);

        // if the previous state was it being blocked enqueue it
        if (prev == THREAD_BLOCKED)
            enqueue_thread(thread);

        woken++;
    }

    return woken;
}

void sched_handle_timeouts() {
    uint64_t time = timer_get_ns();

    while (true) {
        int lock1r = irqlock_lock(&timeouts_lock);
        pheap_node_t* node = pheap_peek(&timeouts);
        thread_t* thread = CONTAINER_OF(node, thread_t, timeout_pheap_node);

        // no threads to wakeup
        if (node == nullptr || thread->wakeup_time > time) {
            irqlock_unlock(&timeouts_lock, lock1r);
            return;
        }

        pheap_pop(&timeouts);
        irqlock_unlock(&timeouts_lock, lock1r);

        thread_state_t prev = __atomic_exchange_n(&thread->state, THREAD_RUNABLE, __ATOMIC_RELEASE);

        // if the previous state was it being blocked enqueue it
        if (prev == THREAD_BLOCKED)
            enqueue_thread(thread);
    }
}
