#pragma once
#include <stdint.h>

#if defined(__x86_64__)
static inline int interrupts_enabled() {
    uint64_t rflags;
    asm volatile (
        "pushfq;"
        "pop %0;"
        : "=r" (rflags)
    );
    return (rflags >> 9) & 1;
}

static inline void enable_interrupts() {
    asm volatile ("sti");
}

static inline void disable_interrupts() {
    asm volatile ("cli");
}
#elif defined(__loongarch64)
static inline int interrupts_enabled() {
    uint64_t tmp;
    asm volatile("csrrd %0, 0" : "=r"(tmp));
    return (tmp >> 2) & 1;
}

static inline void enable_interrupts() {
    uint64_t tmp = (1 << 2);
    asm volatile("csrxchg %0, %0, 0" : "+r"(tmp));
}

static inline void disable_interrupts() {
    uint64_t val  = 0;
    uint64_t mask = (1 << 2);
    asm volatile("csrxchg %0, %1, 0" : "+r"(val) : "r"(mask));
}
#else
_Static_assert(0, "unimplemented");
#endif

static inline void restore_interrupts(int irqs) {
    if (irqs)
        enable_interrupts();
    else
        disable_interrupts();
}
