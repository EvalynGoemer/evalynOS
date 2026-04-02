#pragma once
#include <stdint.h>

static inline int interrupts_enabled() {
    #if defined (__x86_64__)
        uint64_t rflags;
        asm volatile (
            "pushfq;"
            "pop %0;"
            : "=r" (rflags)
        );
    return (rflags >> 9) & 1;
    #elif defined (__loongarch64)
        uint64_t tmp;
        asm volatile("csrrd %0, 0" : "=r"(tmp));
        return (tmp >> 2) & 1;
    #else
        _Static_assert(0, "unimplemented");
    #endif
}

static inline void enable_interrupts() {
    #if defined (__x86_64__)
        asm volatile ("sti");
    #elif defined (__loongarch64)
        uint64_t tmp = (1 << 2);
        asm volatile("csrxchg %0, %0, 0" : "+r"(tmp));
    #else
        _Static_assert(0, "unimplemented");
    #endif
}

static inline void disable_interrupts() {
    #if defined (__x86_64__)
        asm volatile ("cli");
    #elif defined (__loongarch64)
        uint64_t val  = 0;
        uint64_t mask = (1 << 2);
        asm volatile("csrxchg %0, %1, 0" : "+r"(val) : "r"(mask));
    #else
        _Static_assert(0, "unimplemented");
    #endif
}

static inline void restore_interrupts(int irqs) {
    #if defined (__x86_64__) || defined (__loongarch64)
    if (irqs)
        enable_interrupts();
    else
        disable_interrupts();
    #else
    _Static_assert(0, "unimplemented");
    #endif
}
