#pragma once

#include <arch/x86_64/cpu/cpulocal.h>

#define __CPU_LOCAL_OFFSET(VAR) \
    ((uintptr_t)&(VAR) - (uintptr_t)__cpu_local_start)

#define CPU_LOCAL_X86_APPLY_GS(VAR) (*((typeof(VAR) __seg_gs*)(&VAR)))

#define CPU_LOCAL_READ8(VAR)  CPU_LOCAL_X86_APPLY_GS(VAR)
#define CPU_LOCAL_READ16(VAR) CPU_LOCAL_X86_APPLY_GS(VAR)
#define CPU_LOCAL_READ32(VAR) CPU_LOCAL_X86_APPLY_GS(VAR)
#define CPU_LOCAL_READ64(VAR) CPU_LOCAL_X86_APPLY_GS(VAR)
#define CPU_LOCAL_WRITE8(VAR, VAL)  (CPU_LOCAL_X86_APPLY_GS(VAR) = (VAL))
#define CPU_LOCAL_WRITE16(VAR, VAL) (CPU_LOCAL_X86_APPLY_GS(VAR) = (VAL))
#define CPU_LOCAL_WRITE32(VAR, VAL) (CPU_LOCAL_X86_APPLY_GS(VAR) = (VAL))
#define CPU_LOCAL_WRITE64(VAR, VAL) (CPU_LOCAL_X86_APPLY_GS(VAR) = (VAL))

#define CPU_LOCAL_PTR(VAR)                                                         \
    ({ uintptr_t __self = (uintptr_t)CPU_LOCAL_X86_APPLY_GS(fixed_cpu_local.self); \
       (typeof(&(VAR)))(__self + __CPU_LOCAL_OFFSET(VAR)); })

#define SET_CPU_LOCAL(v) asm volatile("wrmsr" : : "a"((uint64_t)(v) & 0xFFFFFFFF), "d"((uint64_t)(v) >> 32), "c"(0xC0000101) : "memory");

#define CPU_LOCAL_FIXED_READ8(FIELD)  CPU_LOCAL_X86_APPLY_GS(fixed_cpu_local.FIELD)
#define CPU_LOCAL_FIXED_READ16(FIELD) CPU_LOCAL_X86_APPLY_GS(fixed_cpu_local.FIELD)
#define CPU_LOCAL_FIXED_READ32(FIELD) CPU_LOCAL_X86_APPLY_GS(fixed_cpu_local.FIELD)
#define CPU_LOCAL_FIXED_READ64(FIELD) CPU_LOCAL_X86_APPLY_GS(fixed_cpu_local.FIELD)
#define CPU_LOCAL_FIXED_WRITE8(FIELD, VAL)  (CPU_LOCAL_X86_APPLY_GS(fixed_cpu_local.FIELD) = (VAL))
#define CPU_LOCAL_FIXED_WRITE16(FIELD, VAL) (CPU_LOCAL_X86_APPLY_GS(fixed_cpu_local.FIELD) = (VAL))
#define CPU_LOCAL_FIXED_WRITE32(FIELD, VAL) (CPU_LOCAL_X86_APPLY_GS(fixed_cpu_local.FIELD) = (VAL))
#define CPU_LOCAL_FIXED_WRITE64(FIELD, VAL) (CPU_LOCAL_X86_APPLY_GS(fixed_cpu_local.FIELD) = (VAL))
