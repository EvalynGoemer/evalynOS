#pragma once

#define CSR_CRMD    0x00
#define CSR_CRMD_IE (1 << 2)

#define CSR_ECFG          0x04
#define CSR_ECFG_TIMER_EN (1 << 11)

#define CSR_EENTRY 0x0C

#define CSR_PGD       0x1B
#define CSR_PWCL      0x1C
#define CSR_PWCH      0x1D
#define CSR_PGDL      0x19
#define CSR_PGDH      0x1A
#define CSR_STLBPS    0x1E
#define CSR_RVACFG    0x1F
#define CSR_PRCFG2    0x22
#define CSR_TLBRENTRY 0x88
#define CSR_TLBRSAVE  0x8B
#define CSR_TLBREHI   0x8E

#define CSR_SAVE0 0x30

#define CSR_TCFG  0x41
#define CSR_TICLR 0x44

#define CSR_DMW0  0x180
#define CSR_DMW1  0x181
#define CSR_DMW2  0x182
#define CSR_DMW3  0x183

#ifndef __ASSEMBLER__

#define csrrd(csr) ({                                                                  \
    uint64_t __val;                                                                    \
    asm volatile("csrrd %0, %1" : "=r"(__val) : "i"(csr) : "memory");                  \
    __val;                                                                             \
})

#define csrwr(csr, value) ({                                                           \
    uint64_t __val = (value);                                                          \
    asm volatile("csrwr %0, %1" : "+r"(__val) : "i"(csr) : "memory");                  \
    __val;                                                                             \
})

#define csrxchg(csr, value, mask) ({                                                   \
    uint64_t __val = (value);                                                          \
    asm volatile("csrxchg %0, %1, %2" : "+r"(__val) : "r"(mask), "i"(csr) : "memory"); \
    __val;                                                                             \
})

#endif
