#include <stdint.h>

uint32_t find_reciprocal_shift(uint64_t numerator, uint64_t denominator) {
    __uint128_t lim = denominator * (__uint128_t)UINT64_MAX;

    uint32_t lo = 0, hi = 127;
    while (lo < hi) {
        uint32_t mid = (lo + hi + 1) >> 1;
        if (((__uint128_t)numerator << mid) <= lim)
            lo = mid;
        else
            hi = mid - 1;
    }

    return lo;
}
