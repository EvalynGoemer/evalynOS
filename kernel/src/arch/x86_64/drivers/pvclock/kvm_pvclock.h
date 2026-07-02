#pragma once
#include <stdint.h>

typedef struct [[gnu::packed]] {
    uint32_t version;
    uint32_t _pad0;
    uint64_t tsc_timestamp;
    uint64_t system_time;
    uint32_t tsc_to_system_mul;
    int8_t   tsc_shift;
    uint8_t  flags;
    uint8_t  _pad2[2];
} kvm_pvclock_t;

extern bool setup_kvm_pvclock();
extern uint64_t kvm_pvclock_get_ns();
