#pragma once

#include <stdint.h>

#define CSR_CRMD    0x00
#define CSR_CRMD_IE (1 << 2)

#define CSR_ECFG   0x04
#define CSR_EENTRY 0x0C

#define CSR_SAVE0 0x30

#define CSR_TLBRENTRY 0x88

#define CSR_DMW0  0x180
#define CSR_DMW1  0x181
#define CSR_DMW2  0x182
#define CSR_DMW3  0x183

static inline uint64_t csrrd(uint32_t csr) {
    uint64_t value;
    asm volatile("csrrd %0, %1" : "=r"(value) : "i"(csr) : "memory");
    return value;
}

static inline uint64_t csrwr(uint32_t csr, uint64_t value) {
    asm volatile("csrwr %0, %1" : "+r"(value) : "i"(csr) : "memory");
    return value;
}

static inline uint64_t csrxchg(uint32_t csr, uint64_t value, uint64_t mask) {
    asm volatile("csrxchg %0, %1, %2" : "+r"(value) : "r"(mask), "i"(csr) : "memory");
    return value;
}
