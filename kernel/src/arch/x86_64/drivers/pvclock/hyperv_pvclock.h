#pragma once
#include <stdint.h>

typedef struct [[gnu::packed]] {
    volatile uint32_t tsc_sequence;
    uint32_t          _reserved0;
    volatile uint64_t tsc_scale;
    volatile int64_t  tsc_offset;
    uint64_t          _reserved1[509];
} hyperv_pvclock_t;

extern bool setup_hyperv_pvclock();
extern uint64_t hyperv_pvclock_get_ns();
