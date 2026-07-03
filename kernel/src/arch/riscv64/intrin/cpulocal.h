#pragma once

#include <stdint.h>  // IWYU pragma: keep

#define __CPU_LOCAL_OFFSET(VAR) \
    ((uintptr_t)&(VAR) - (uintptr_t)__cpu_local_start)

#define __CPU_LOCAL_RV_FIXED_READ(INSN, TYPE, VAR)                   \
    ({                                                               \
        TYPE __v;                                                    \
        asm volatile(INSN " %0, %1(tp)"                              \
                     : "=r"(__v) : "n"(__CPU_LOCAL_OFFSET(VAR)));    \
        __v;                                                         \
    })

#define __CPU_LOCAL_RV_FIXED_WRITE(INSN, TYPE, VAR, VAL)             \
    ({                                                               \
        TYPE __v = (VAL);                                            \
        asm volatile(INSN " %1, %0(tp)"                              \
                     : : "n"(__CPU_LOCAL_OFFSET(VAR)), "r"(__v)      \
                     : "memory");                                    \
        (void)0;                                                     \
    })

#define __CPU_LOCAL_RV_READ(INSN, TYPE, VAR)                         \
    ({                                                               \
        TYPE __v;                                                    \
        asm volatile("add  %0, tp, %1\n\t"                           \
                     INSN " %0, 0(%0)"                               \
                     : "=&r"(__v) : "r"(__CPU_LOCAL_OFFSET(VAR)));   \
        __v;                                                         \
    })

#define __CPU_LOCAL_RV_WRITE(INSN, TYPE, VAR, VAL)                   \
    ({                                                               \
        TYPE __v = (VAL);                                            \
        uintptr_t __addr;                                            \
        asm volatile("add  %0, tp, %1\n\t"                           \
                     INSN " %2, 0(%0)"                               \
                     : "=&r"(__addr)                                 \
                     : "r"(__CPU_LOCAL_OFFSET(VAR)), "r"(__v)        \
                     : "memory");                                    \
        (void)0;                                                     \
    })

#define CPU_LOCAL_FIXED_READ8(VAR)  __CPU_LOCAL_RV_FIXED_READ("lb", uint8_t,  VAR)
#define CPU_LOCAL_FIXED_READ16(VAR) __CPU_LOCAL_RV_FIXED_READ("lh", uint16_t, VAR)
#define CPU_LOCAL_FIXED_READ32(VAR) __CPU_LOCAL_RV_FIXED_READ("lw", uint32_t, VAR)
#define CPU_LOCAL_FIXED_READ64(VAR) __CPU_LOCAL_RV_FIXED_READ("ld", uint64_t, VAR)

#define CPU_LOCAL_FIXED_WRITE8(VAR, VAL)  __CPU_LOCAL_RV_FIXED_WRITE("sb", uint8_t,  VAR, VAL)
#define CPU_LOCAL_FIXED_WRITE16(VAR, VAL) __CPU_LOCAL_RV_FIXED_WRITE("sh", uint16_t, VAR, VAL)
#define CPU_LOCAL_FIXED_WRITE32(VAR, VAL) __CPU_LOCAL_RV_FIXED_WRITE("sw", uint32_t, VAR, VAL)
#define CPU_LOCAL_FIXED_WRITE64(VAR, VAL) __CPU_LOCAL_RV_FIXED_WRITE("sd", uint64_t, VAR, VAL)

#define CPU_LOCAL_READ8(VAR)  __CPU_LOCAL_RV_READ("lb", uint8_t,  VAR)
#define CPU_LOCAL_READ16(VAR) __CPU_LOCAL_RV_READ("lh", uint16_t, VAR)
#define CPU_LOCAL_READ32(VAR) __CPU_LOCAL_RV_READ("lw", uint32_t, VAR)
#define CPU_LOCAL_READ64(VAR) __CPU_LOCAL_RV_READ("ld", uint64_t, VAR)

#define CPU_LOCAL_WRITE8(VAR, VAL)  __CPU_LOCAL_RV_WRITE("sb", uint8_t,  VAR, VAL)
#define CPU_LOCAL_WRITE16(VAR, VAL) __CPU_LOCAL_RV_WRITE("sh", uint16_t, VAR, VAL)
#define CPU_LOCAL_WRITE32(VAR, VAL) __CPU_LOCAL_RV_WRITE("sw", uint32_t, VAR, VAL)
#define CPU_LOCAL_WRITE64(VAR, VAL) __CPU_LOCAL_RV_WRITE("sd", uint64_t, VAR, VAL)

#define SET_CPU_LOCAL(v) asm volatile("mv tp, %0" : : "r"(v) : "tp")
