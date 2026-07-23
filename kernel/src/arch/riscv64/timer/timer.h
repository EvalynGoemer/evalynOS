#pragma once
#include <stdint.h>

extern void setup_timer();
extern uint64_t timer_get_ns();
extern void timer_spin_wait_ms(int ms);

extern uint64_t timebase_frequency;
