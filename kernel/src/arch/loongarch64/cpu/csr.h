#pragma once

#include <stdint.h>

#define CSR_DMW0  0x180
#define CSR_DMW1  0x181
#define CSR_DMW2  0x182
#define CSR_DMW3  0x183

static inline void csrwr(uint32_t csr, uint64_t value) {
    asm volatile("csrwr %0, %1" : "+r"(value) : "i"(csr) : "memory");
}
