#pragma once

#include <stdint.h>
#include <utils/atomic.h>
#include <arch/intrin/spin.h>

typedef struct {
    uint32_t flag;
} spinlock_t;

static inline void spinlock_init(spinlock_t* spinlock) {
    atomic_store32(&spinlock->flag, 0, AMO_RELAXED);
}

static inline void spinlock_lock(spinlock_t* spinlock) {
    while (true) {
        while (atomic_load32(&spinlock->flag, AMO_RELAXED))
            spin();
        if (!atomic_xchg32(&spinlock->flag, 1, AMO_ACQUIRE))
            break;
    }
}

static inline void spinlock_unlock(spinlock_t* spinlock) {
    atomic_store32(&spinlock->flag, 0, AMO_RELEASE);
}
