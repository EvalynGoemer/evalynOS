#pragma once

#include <stdint.h>
#include <utils/atomic.h>
#include <arch/intrin/interrupts.h>
#include <arch/intrin/spin.h>

#define IRQLOCK_IRQS_DISABLED 0

typedef struct {
    uint32_t flag;
} irqlock_t;

static inline void irqlock_init(irqlock_t* irqlock) {
    atomic_store32(&irqlock->flag, 0, AMO_RELAXED);
}

[[nodiscard]]
static inline int irqlock_lock(irqlock_t* irqlock) {
    int irqs = interrupts_enabled();
    disable_interrupts();
    while (true) {
        while (atomic_load32(&irqlock->flag, AMO_RELAXED))
            spin();
        if (!atomic_xchg32(&irqlock->flag, 1, AMO_ACQUIRE))
            break;
    }
    return irqs;
}

static inline void irqlock_unlock(irqlock_t* irqlock, int irqs) {
    atomic_store32(&irqlock->flag, 0, AMO_RELEASE);
    restore_interrupts(irqs);
}
