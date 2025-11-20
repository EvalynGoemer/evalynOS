#pragma once

#include <stdint.h>

extern void setup_ps2();
extern volatile uint8_t ps2Kbd_buffer_index;
extern volatile char ps2Kbd_buffer[256];
