#include <stdint.h>

uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;

    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return low + ((uint64_t) high << 32);
}

void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;

    asm volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}
