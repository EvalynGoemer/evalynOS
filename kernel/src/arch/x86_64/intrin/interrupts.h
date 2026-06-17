#pragma once
#include <stdint.h>

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

static inline void wfi() {
    asm volatile ("hlt");
}
