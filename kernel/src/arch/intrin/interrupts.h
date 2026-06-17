#pragma once
#include <utils/lib.h>

#include STRINGIFY(arch/TARGET_ARCH/intrin/interrupts.h)

static inline void restore_interrupts(int irqs) {
    if (irqs)
        enable_interrupts();
    else
        disable_interrupts();
}

[[noreturn]]
static inline void hcf() {
    disable_interrupts();
    while (true)
        wfi();
}
