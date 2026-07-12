#pragma once

#include <stdint.h>
#include <arch/intrin/spin.h>
#include <sched/preempt.h>

typedef struct {
    uint32_t flag;
} preemptlock_t;

static inline void preemptlock_init(preemptlock_t* preemptlock) {
    __atomic_store_n(&preemptlock->flag, 0, __ATOMIC_RELAXED);
}

static inline void preemptlock_lock(preemptlock_t* preemptlock) {
    disable_preemption();
    while (true) {
        while (__atomic_load_n(&preemptlock->flag, __ATOMIC_RELAXED))
            spin();
        if (!__atomic_exchange_n(&preemptlock->flag, 1, __ATOMIC_ACQUIRE))
            break;
    }
}

static inline void preemptlock_unlock(preemptlock_t* preemptlock) {
    __atomic_store_n(&preemptlock->flag, 0, __ATOMIC_RELEASE);
    enable_preemption();
}
