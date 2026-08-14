#pragma once
#include <sched/preempt.h>
#include <stdint.h>
#include <arch/intrin/spin.h>

typedef struct {
    uint32_t next;
    uint32_t owner;
} ticketlock_t;

static inline void ticketlock_init(ticketlock_t* lock) {
    __atomic_store_n(&lock->next,  0, __ATOMIC_RELAXED);
    __atomic_store_n(&lock->owner, 0, __ATOMIC_RELAXED);
}

static inline void ticketlock_lock(ticketlock_t* lock) {
    disable_preemption();
    uint32_t my = __atomic_fetch_add(&lock->next, 1, __ATOMIC_RELAXED);
    while (__atomic_load_n(&lock->owner, __ATOMIC_ACQUIRE) != my)
        spin();
}

static inline void ticketlock_unlock(ticketlock_t* lock) {
    __atomic_fetch_add(&lock->owner, 1, __ATOMIC_RELEASE);
    enable_preemption();
}
