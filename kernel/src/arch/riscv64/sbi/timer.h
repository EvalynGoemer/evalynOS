#pragma once
#include <stdint.h>
#include "sbi.h"

static inline bool sbi_set_timer(uint64_t stime) {
    sbiret_t ret = sbi_ecall1(SBI_SET_TIMER, stime);
    if (ret.error < 0)
        return false;
    return true;
}

static inline bool legacy_sbi_set_timer(uint64_t stime) {
    long lret = legacy_sbi_ecall1(LEGACY_SBI_SET_TIMER, stime);
    if (lret < 0)
        return false;
    return true;
}
