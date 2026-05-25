#pragma once

static inline int log2ull(unsigned long long val) {
    if(val == 0) return 0;
    return 63 - __builtin_clzll(val);
}

static inline bool isPow2ull(unsigned long long val) {
    return val != 0 && (val & (val - 1)) == 0;
}
