#pragma once

#include <stdint.h>
#include <arch/intrin/interrupts.h>
#include <arch/intrin/spin.h>

#define IRQLOCK_IRQS_DISABLED 0

typedef struct {
    uint32_t flag;
} irqlock_t;

static inline void irqlock_init(irqlock_t* irqlock) {
    __atomic_store_n(&irqlock->flag, 0, __ATOMIC_RELAXED);
}

[[nodiscard]]
static inline int irqlock_lock(irqlock_t* irqlock) {
    int irqs = interrupts_enabled();
    disable_interrupts();
    while (true) {
        while (__atomic_load_n(&irqlock->flag, __ATOMIC_RELAXED))
            spin();
        if (!__atomic_exchange_n(&irqlock->flag, 1, __ATOMIC_ACQUIRE))
            break;
    }
    return irqs;
}

static inline void irqlock_unlock(irqlock_t* irqlock, int irqs) {
    __atomic_store_n(&irqlock->flag, 0, __ATOMIC_RELEASE);
    restore_interrupts(irqs);
}
