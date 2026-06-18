#pragma once
#include <arch/riscv64/intrin/csr.h>

static inline int interrupts_enabled() {
    return (csrr(CSR_SSTATUS) & SSTATUS_SIE) != 0;
}

static inline void enable_interrupts() {
    csrs(CSR_SSTATUS, SSTATUS_SIE);
}

static inline void disable_interrupts() {
    csrc(CSR_SSTATUS, SSTATUS_SIE);
}

static inline void wfi() {
    asm volatile ("wfi");
}
