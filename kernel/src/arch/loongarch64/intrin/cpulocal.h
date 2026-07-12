#pragma once

#include <stdint.h> // IWYU pragma: keep
#include <arch/loongarch64/cpu/cpulocal.h>

#define __CPU_LOCAL_OFFSET(VAR) \
    ((uintptr_t)&(VAR) - (uintptr_t)__cpu_local_start)

#define __CPU_LOCAL_LA_READ(INSN, TYPE, VAR)                         \
    ({                                                               \
        TYPE __v;                                                    \
        asm volatile("add.d %0, $r21, %1\n\t"                        \
                     INSN " %0, %0, 0"                               \
                     : "=&r"(__v) : "r"(__CPU_LOCAL_OFFSET(VAR)));   \
        __v;                                                         \
    })

#define __CPU_LOCAL_LA_WRITE(INSN, TYPE, VAR, VAL)                   \
    ({                                                               \
        TYPE __v = (VAL);                                            \
        uintptr_t __addr;                                            \
        asm volatile("add.d %0, $r21, %1\n\t"                        \
                     INSN " %2, %0, 0"                               \
                     : "=&r"(__addr)                                 \
                     : "r"(__CPU_LOCAL_OFFSET(VAR)), "r"(__v)        \
                     : "memory");                                    \
        (void)0;                                                     \
    })

#define CPU_LOCAL_READ8(VAR)  __CPU_LOCAL_LA_READ("ld.b", uint8_t,  VAR)
#define CPU_LOCAL_READ16(VAR) __CPU_LOCAL_LA_READ("ld.h", uint16_t, VAR)
#define CPU_LOCAL_READ32(VAR) __CPU_LOCAL_LA_READ("ld.w", uint32_t, VAR)
#define CPU_LOCAL_READ64(VAR) __CPU_LOCAL_LA_READ("ld.d", uint64_t, VAR)

#define CPU_LOCAL_WRITE8(VAR, VAL)  __CPU_LOCAL_LA_WRITE("st.b", uint8_t,  VAR, VAL)
#define CPU_LOCAL_WRITE16(VAR, VAL) __CPU_LOCAL_LA_WRITE("st.h", uint16_t, VAR, VAL)
#define CPU_LOCAL_WRITE32(VAR, VAL) __CPU_LOCAL_LA_WRITE("st.w", uint32_t, VAR, VAL)
#define CPU_LOCAL_WRITE64(VAR, VAL) __CPU_LOCAL_LA_WRITE("st.d", uint64_t, VAR, VAL)

#define SET_CPU_LOCAL(v) asm volatile("move $r21, %0" : : "r"(v) : "r21")

#define CPU_LOCAL_PTR(VAR)                                                              \
    ({ uintptr_t __ptr;                                                                 \
       asm volatile("add.d %0, $r21, %1" : "=r"(__ptr) : "r"(__CPU_LOCAL_OFFSET(VAR))); \
       (typeof(&(VAR)))__ptr; })

#define CPU_LOCAL_FIXED_READ8(FIELD) \
    ({ uint8_t __v; asm volatile("ld.b %0, $r21, %1" : "=r"(__v) : "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD))); __v; })
#define CPU_LOCAL_FIXED_READ16(FIELD) \
    ({ uint16_t __v; asm volatile("ld.h %0, $r21, %1" : "=r"(__v) : "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD))); __v; })
#define CPU_LOCAL_FIXED_READ32(FIELD) \
    ({ uint32_t __v; asm volatile("ld.w %0, $r21, %1" : "=r"(__v) : "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD))); __v; })
#define CPU_LOCAL_FIXED_READ64(FIELD) \
    ({ uint64_t __v; asm volatile("ld.d %0, $r21, %1" : "=r"(__v) : "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD))); __v; })

#define CPU_LOCAL_FIXED_WRITE8(FIELD, VAL) \
    asm volatile("st.b %0, $r21, %1" :: "r"((uint8_t)(VAL)), "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD)) : "memory")
#define CPU_LOCAL_FIXED_WRITE16(FIELD, VAL) \
    asm volatile("st.h %0, $r21, %1" :: "r"((uint16_t)(VAL)), "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD)) : "memory")
#define CPU_LOCAL_FIXED_WRITE32(FIELD, VAL) \
    asm volatile("st.w %0, $r21, %1" :: "r"((uint32_t)(VAL)), "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD)) : "memory")
#define CPU_LOCAL_FIXED_WRITE64(FIELD, VAL) \
    asm volatile("st.d %0, $r21, %1" :: "r"((uint64_t)(VAL)), "i"(__builtin_offsetof(fixed_cpu_local_t, FIELD)) : "memory")
