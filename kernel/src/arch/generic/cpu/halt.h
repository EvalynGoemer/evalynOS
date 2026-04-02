#pragma once
#include <arch/generic/cpu/interrupts.h>

static inline void wfi() {
    #if defined (__x86_64__) || defined(__i386__)
        asm ("hlt");
    #elif defined (__aarch64__) || defined (__riscv)
        asm ("wfi");
    #elif defined (__loongarch64)
        asm ("idle 0");
    #else
        _Static_assert(0, "unimplemented");
    #endif
}

static inline void spin() {
    #if defined (__x86_64__) || defined(__i386__)
        asm ("pause");
    #elif defined (__loongarch64)
        asm volatile("ibar 0");
    #else
        _Static_assert(0, "unimplemented");
    #endif
}

[[noreturn]]
static inline void hcf() {
    disable_interrupts();
    while (true)
        wfi();
}
