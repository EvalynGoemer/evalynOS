#pragma once

#include "stdint.h"

extern void setup_ps2();
extern volatile uint8_t kbd_buffer_index;
extern volatile char kbd_buffer[256];
