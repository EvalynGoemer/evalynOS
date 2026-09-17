#pragma once

#define CSR_SSTATUS   0x100
#define CSR_SIE       0x104
#define CSR_SIP       0x144
#define CSR_STVEC     0x105
#define CSR_SSCRATCH  0x140
#define CSR_STIMECMP  0x14D
#define CSR_SEPC      0x141
#define CSR_SCAUSE    0x142
#define CSR_STVAL     0x143
#define CSR_SATP      0x180
#define CSR_TIME      0xC01

#define SIE_STIE      (1ull << 5)
#define SSTATUS_SIE   (1ull << 1)
#define SSTATUS_SPP   (1ull << 8)

#ifndef __ASSEMBLER__

#define csrw(csr, value) asm volatile("csrw %0, %1" : : "i"(csr), "r"(value) : "memory")
#define csrs(csr, mask) asm volatile("csrs %0, %1" : : "i"(csr), "r"(mask) : "memory")
#define csrc(csr, mask) asm volatile("csrc %0, %1" : : "i"(csr), "r"(mask) : "memory")

#define csrr(csr) ({                                                                  \
    uint64_t __val;                                                                   \
    asm volatile("csrr %0, %1" : "=r"(__val) : "i"(csr) : "memory");                  \
    __val;                                                                            \
})

#define csrrw(csr, value) ({                                                          \
    uint64_t __val;                                                                   \
    asm volatile("csrrw %0, %1, %2" : "=r"(__val) : "i"(csr), "r"(value) : "memory"); \
    __val;                                                                            \
})

#define csrrs(csr, mask) ({                                                          \
    uint64_t __val;                                                                  \
    asm volatile("csrrs %0, %1, %2" : "=r"(__val) : "i"(csr), "r"(mask) : "memory"); \
    __val;                                                                           \
})

#define csrrc(csr, mask) ({                                                          \
    uint64_t __val;                                                                  \
    asm volatile("csrrc %0, %1, %2" : "=r"(__val) : "i"(csr), "r"(mask) : "memory"); \
    __val;                                                                           \
})

#endif
