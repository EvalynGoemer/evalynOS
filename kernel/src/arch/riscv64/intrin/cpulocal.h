#pragma once

#include <stdint.h> // IWYU pragma: keep
#include <arch/riscv64/cpu/cpulocal.h>

#define __CPU_LOCAL_OFFSET(VAR) \
    ((uintptr_t)&(VAR) - (uintptr_t)__cpu_local_start)

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

#define CPU_LOCAL_READ8(VAR)  __CPU_LOCAL_RV_READ("lb", uint8_t,  VAR)
#define CPU_LOCAL_READ16(VAR) __CPU_LOCAL_RV_READ("lh", uint16_t, VAR)
#define CPU_LOCAL_READ32(VAR) __CPU_LOCAL_RV_READ("lw", uint32_t, VAR)
#define CPU_LOCAL_READ64(VAR) __CPU_LOCAL_RV_READ("ld", uint64_t, VAR)

#define CPU_LOCAL_WRITE8(VAR, VAL)  __CPU_LOCAL_RV_WRITE("sb", uint8_t,  VAR, VAL)
#define CPU_LOCAL_WRITE16(VAR, VAL) __CPU_LOCAL_RV_WRITE("sh", uint16_t, VAR, VAL)
#define CPU_LOCAL_WRITE32(VAR, VAL) __CPU_LOCAL_RV_WRITE("sw", uint32_t, VAR, VAL)
#define CPU_LOCAL_WRITE64(VAR, VAL) __CPU_LOCAL_RV_WRITE("sd", uint64_t, VAR, VAL)

#define SET_CPU_LOCAL(v) asm volatile("mv tp, %0" : : "r"(v) : "tp")

#define CPU_LOCAL_PTR(VAR)                                                          \
    ({ uintptr_t __ptr;                                                             \
       asm volatile("add %0, tp, %1" : "=r"(__ptr) : "r"(__CPU_LOCAL_OFFSET(VAR))); \
       (typeof(&(VAR)))__ptr; })

#define CPU_LOCAL_FIXED_READ8(FIELD) \
    ({ uint8_t __v; asm volatile("lb %0, %1(tp)" : "=r"(__v) : "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD))); __v; })
#define CPU_LOCAL_FIXED_READ16(FIELD) \
    ({ uint16_t __v; asm volatile("lh %0, %1(tp)" : "=r"(__v) : "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD))); __v; })
#define CPU_LOCAL_FIXED_READ32(FIELD) \
    ({ uint32_t __v; asm volatile("lw %0, %1(tp)" : "=r"(__v) : "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD))); __v; })
#define CPU_LOCAL_FIXED_READ64(FIELD) \
    ({ uint64_t __v; asm volatile("ld %0, %1(tp)" : "=r"(__v) : "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD))); __v; })

#define CPU_LOCAL_FIXED_WRITE8(FIELD, VAL) \
    asm volatile("sb %0, %1(tp)" :: "r"((uint8_t)(VAL)), "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD)) : "memory")
#define CPU_LOCAL_FIXED_WRITE16(FIELD, VAL) \
    asm volatile("sh %0, %1(tp)" :: "r"((uint16_t)(VAL)), "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD)) : "memory")
#define CPU_LOCAL_FIXED_WRITE32(FIELD, VAL) \
    asm volatile("sw %0, %1(tp)" :: "r"((uint32_t)(VAL)), "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD)) : "memory")
#define CPU_LOCAL_FIXED_WRITE64(FIELD, VAL) \
    asm volatile("sd %0, %1(tp)" :: "r"((uint64_t)(VAL)), "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD)) : "memory")
