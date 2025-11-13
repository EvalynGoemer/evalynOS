#pragma once
#include "stdint.h"

__attribute__((no_caller_saved_registers))
__attribute__((target("general-regs-only")))
extern void play_sound(uint16_t ax);
extern void stop_sound();
