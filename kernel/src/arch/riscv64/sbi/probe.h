#pragma once
#include "sbi.h"

static inline bool sbi_probe_extension(long ext_id) {
    sbiret_t ret = sbi_ecall1(SBI_PROBE_SBI_EXT, ext_id);
    if (ret.error != SBI_SUCCESS)
        return false;
    if (ret.value == 1)
        return true;
    return false;
}
