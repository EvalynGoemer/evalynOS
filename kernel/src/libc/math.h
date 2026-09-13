#pragma once

#include <stdint.h>
#include <stdbool.h>

#define COUNT_TRAILING_ZEROS(v) ((v) ? __builtin_ctzll((v)) : 64)

static inline int log2ull(unsigned long long val) {
    if(val == 0) return 0;
    return 63 - __builtin_clzll(val);
}

static inline int log2ullceil(unsigned long long val) {
    if (val <= 1) return 0;
    return log2ull(val - 1) + 1;
}

static inline bool isPow2ull(unsigned long long val) {
    return val != 0 && (val & (val - 1)) == 0;
}

uint32_t find_reciprocal_shift(uint64_t numerator, uint64_t denominator);
