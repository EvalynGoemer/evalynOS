#pragma once
#include <stdint.h>
#include <utils/atomic.h>
#include <sched/preempt.h>
#include <arch/intrin/spin.h>

typedef struct {
    uint32_t next;
    uint32_t owner;
} ticketlock_t;

static inline void ticketlock_init(ticketlock_t* lock) {
    atomic_store32(&lock->next,  0, AMO_RELAXED);
    atomic_store32(&lock->owner, 0, AMO_RELAXED);
}

static inline void ticketlock_lock(ticketlock_t* lock) {
    disable_preemption();
    uint32_t my = atomic_fetch_add32(&lock->next, 1, AMO_RELAXED);
    while (atomic_load32(&lock->owner, AMO_ACQUIRE) != my)
        spin();
}

static inline void ticketlock_unlock(ticketlock_t* lock) {
    atomic_fetch_add32(&lock->owner, 1, AMO_RELEASE);
    enable_preemption();
}
