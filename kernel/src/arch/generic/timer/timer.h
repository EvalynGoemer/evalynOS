#pragma once
#include <stdint.h>

extern uint64_t timer_get_ns();
extern void timer_spin_wait_ms(int ms);
extern void timer_set_timeout_ms(int ms);

static inline uint64_t timer_get_us() {
    return timer_get_ns() / 1000;
}

static inline uint64_t timer_get_ms() {
    return timer_get_ns() / 1000000;
}

static inline uint64_t timer_get_sec() {
    return timer_get_ns() / 1000000000;
}
