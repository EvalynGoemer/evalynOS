#include <assert.h>
#include <stdint.h>
#include <utils/lib.h>
#include <utils/locks/mutex.h>

#define MUTEX_UNLOCKED         0 // mutex is fully unlocked with no waiters
#define MUTEX_LOCKED           1 // mutex is locked with no waiters
#define MUTEX_LOCKED_WAITERS   2 // mutex is locked and may have waiters
#define MUTEX_UNLOCKED_WAITERS 3 // mutex is unlocked but has waiters pending

static bool mutex_claim(void* ctx) {
    mutex_t* mutex = ctx;

    // lock it with potential waiters pending, if it wasnt unlocked we must block
    if (__atomic_exchange_n(&mutex->locked, MUTEX_LOCKED_WAITERS, __ATOMIC_ACQUIRE) != MUTEX_UNLOCKED)
        return false;

    // if there are no waiters here set it to be locked normally
    if (mutex->waiters.threads.head == nullptr)
        __atomic_store_n(&mutex->locked, MUTEX_LOCKED, __ATOMIC_RELEASE);

    return true;
}

void mutex_lock(mutex_t* mutex) {
    // if there is no contention grab it directly
    if (__atomic_load_n(&mutex->locked, __ATOMIC_RELAXED) == MUTEX_UNLOCKED) {
        uint32_t expected = MUTEX_UNLOCKED;
        if (__atomic_compare_exchange_n(&mutex->locked, &expected, MUTEX_LOCKED, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
            return;
    }

    if (wait_on_cond(&mutex->waiters, mutex_claim, mutex, 0) == WAIT_STATUS_CONDITION_MET)
        return;

    __atomic_store_n(&mutex->locked, MUTEX_LOCKED_WAITERS, __ATOMIC_RELEASE);
}

void mutex_unlock(mutex_t* mutex) {
    // no waiters unlock directly
    uint32_t expected = MUTEX_LOCKED;
    if (__atomic_compare_exchange_n(&mutex->locked, &expected, MUTEX_UNLOCKED, false, __ATOMIC_RELEASE, __ATOMIC_RELAXED))
        return;

    // there may be waiters pending so reserve it for them and wake them
    __atomic_store_n(&mutex->locked, MUTEX_UNLOCKED_WAITERS, __ATOMIC_RELEASE);
    if (sched_wakeup_n_threads(&mutex->waiters, 1))
        return;

    // if nothing was woken up we need to check if something had locked it between our operations
    expected = MUTEX_UNLOCKED_WAITERS;
    if (__atomic_compare_exchange_n(&mutex->locked, &expected, MUTEX_UNLOCKED, false, __ATOMIC_RELEASE, __ATOMIC_RELAXED))
        return;

    dbg_assert(expected == MUTEX_LOCKED_WAITERS);
    sched_wakeup_n_threads(&mutex->waiters, 1);
}
