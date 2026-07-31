#pragma once
#include <stdint.h>

extern bool setup_pvclock();
extern uint64_t pvclock_get_ns();
