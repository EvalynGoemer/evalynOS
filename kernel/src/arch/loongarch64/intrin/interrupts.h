#pragma once
#include <arch/loongarch64/cpu/csr.h>

static inline int interrupts_enabled() {
    return (csrrd(CSR_CRMD) & CSR_CRMD_IE) != 0;
}

static inline void enable_interrupts() {
    csrxchg(CSR_CRMD, CSR_CRMD_IE, CSR_CRMD_IE);
}

static inline void disable_interrupts() {
    csrxchg(CSR_CRMD, 0, CSR_CRMD_IE);
}

static inline void wfi() {
    asm volatile ("idle 0");
}
