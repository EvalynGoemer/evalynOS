#pragma once

#include <utils/lib.h>
#include <stdint.h> // IWYU pragma: keep

#define CPU_LOCAL [[gnu::section(".cpu_local")]]

extern unsigned char __cpu_local_start[];
extern unsigned char __cpu_local_end[];

#include STRINGIFY(arch/TARGET_ARCH/intrin/cpulocal.h)

_Static_assert(sizeof(bool) == 1, "bool must be 1 byte");
#define CPU_LOCAL_READ(VAR)              \
    _Generic((VAR),                      \
        bool:     CPU_LOCAL_READ8(VAR) , \
        uint8_t:  CPU_LOCAL_READ8(VAR) , \
        uint16_t: CPU_LOCAL_READ16(VAR), \
        uint32_t: CPU_LOCAL_READ32(VAR), \
        uint64_t: CPU_LOCAL_READ64(VAR)  \
    )

#define CPU_LOCAL_WRITE(VAR, VAL)              \
    _Generic((VAR),                            \
        bool:     CPU_LOCAL_WRITE8(VAR, VAL) , \
        uint8_t:  CPU_LOCAL_WRITE8(VAR, VAL) , \
        uint16_t: CPU_LOCAL_WRITE16(VAR, VAL), \
        uint32_t: CPU_LOCAL_WRITE32(VAR, VAL), \
        uint64_t: CPU_LOCAL_WRITE64(VAR, VAL)  \
    )
