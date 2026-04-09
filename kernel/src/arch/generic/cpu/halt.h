#pragma once
#include <arch/generic/cpu/interrupts.h>

#if defined(__x86_64__)
static inline void wfi() {
    asm ("hlt");
}

static inline void spin() {
    asm ("pause");
}
#elif defined(__loongarch64)

static inline void wfi() {
    asm ("idle 0");
}

static inline void spin() {
    asm volatile("ibar 0");
}

#else
_Static_assert(0, "unimplemented");
#endif

[[noreturn]]
static inline void hcf() {
    disable_interrupts();
    while (true)
        wfi();
}
