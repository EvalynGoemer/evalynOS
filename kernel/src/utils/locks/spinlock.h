#pragma once
#include <stdint.h>
#include <arch/intrin/interrupts.h>
#include <arch/intrin/spin.h>

typedef struct {
    uint32_t flag;
} spinlock_t;

static inline void spinlock_init(spinlock_t* spinlock) {
    __atomic_store_n(&spinlock->flag, 0, __ATOMIC_RELAXED);
}

[[nodiscard]]
static inline int spinlock_lock(spinlock_t* spinlock) {
    int irqs = interrupts_enabled();
    disable_interrupts();
    while (true) {
        while (__atomic_load_n(&spinlock->flag, __ATOMIC_RELAXED))
            spin();
        if (!__atomic_exchange_n(&spinlock->flag, 1, __ATOMIC_ACQUIRE))
            break;
    }
    return irqs;
}

static inline void spinlock_unlock(spinlock_t* spinlock, int irqs) {
    __atomic_store_n(&spinlock->flag, 0, __ATOMIC_RELEASE);
    restore_interrupts(irqs);
}
