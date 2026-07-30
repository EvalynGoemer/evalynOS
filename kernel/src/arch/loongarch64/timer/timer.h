#pragma once
#include <stdint.h>

extern void setup_timer();
extern uint64_t timer_get_ns();
extern void timer_set_timeout_ms(uint64_t ms);
