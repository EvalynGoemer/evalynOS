#pragma once

#include <stdint.h>
#include <utils/atomic.h>
#include <arch/intrin/spin.h>
#include <sched/preempt.h>

typedef struct {
    uint32_t flag;
} preemptlock_t;

static inline void preemptlock_init(preemptlock_t* preemptlock) {
    atomic_store32(&preemptlock->flag, 0, AMO_RELAXED);
}

static inline void preemptlock_lock(preemptlock_t* preemptlock) {
    disable_preemption();
    while (true) {
        while (atomic_load32(&preemptlock->flag, AMO_RELAXED))
            spin();
        if (!atomic_xchg32(&preemptlock->flag, 1, AMO_ACQUIRE))
            break;
    }
}

static inline void preemptlock_unlock(preemptlock_t* preemptlock) {
    atomic_store32(&preemptlock->flag, 0, AMO_RELEASE);
    enable_preemption();
}
