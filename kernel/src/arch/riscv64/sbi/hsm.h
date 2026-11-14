#pragma once
#include "sbi.h"

static inline bool sbi_hart_start(unsigned long hartid, unsigned long start_paddr, unsigned long opaque) {
    sbiret_t ret = sbi_ecall3(SBI_HART_START, hartid, start_paddr, opaque);
    if (ret.error != SBI_SUCCESS) return false;
    return true;
}
