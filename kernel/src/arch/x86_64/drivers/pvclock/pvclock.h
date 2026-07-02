#pragma once
#include <stdint.h>

extern bool setup_pvclock(uint32_t core_id);
extern uint64_t pvclock_get_ns();

static inline uint64_t pvclock_get_us() {
    return pvclock_get_ns() / 1000;
}

static inline uint64_t pvclock_get_ms() {
    return pvclock_get_ns() / 1000000;
}

static inline uint64_t pvclock_get_sec() {
    return pvclock_get_ns() / 1000000000;
}
