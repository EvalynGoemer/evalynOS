#pragma once
#include <stdint.h>

#define CSR_SSTATUS   0x100
#define CSR_SIE       0x104
#define CSR_SIP       0x144
#define CSR_STVEC     0x105
#define CSR_SSCRATCH  0x140
#define CSR_SEPC      0x141
#define CSR_SCAUSE    0x142
#define CSR_STVAL     0x143
#define CSR_SATP      0x180

#define SSTATUS_SIE   (1ull << 1)
#define SSTATUS_SPP   (1ull << 8)

static inline uint64_t csrr(uint64_t csr) {
    uint64_t val;
    asm volatile ("csrr %0, %1" : "=r"(val) : "i"(csr) : "memory");
    return val;
}

static inline void csrw(uint64_t csr, uint64_t val) {
    asm volatile ("csrw %0, %1" : : "i"(csr), "r"(val) : "memory");
}

static inline void csrs(uint64_t csr, uint64_t mask) {
    asm volatile ("csrs %0, %1" : : "i"(csr), "r"(mask) : "memory");
}

static inline void csrc(uint64_t csr, uint64_t mask) {
    asm volatile ("csrc %0, %1" : : "i"(csr), "r"(mask) : "memory");
}

static inline uint64_t csrrw(uint64_t csr, uint64_t val) {
    uint64_t old;
    asm volatile ("csrrw %0, %1, %2" : "=r"(old) : "i"(csr), "r"(val) : "memory");
    return old;
}

static inline uint64_t csrrs(uint64_t csr, uint64_t mask) {
    uint64_t old;
    asm volatile ("csrrs %0, %1, %2" : "=r"(old) : "i"(csr), "r"(mask) : "memory");
    return old;
}

static inline uint64_t csrrc(uint64_t csr, uint64_t mask) {
    uint64_t old;
    asm volatile ("csrrc %0, %1, %2" : "=r"(old) : "i"(csr), "r"(mask) : "memory");
    return old;
}
