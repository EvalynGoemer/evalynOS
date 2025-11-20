#pragma once
#include <stdint.h>

void setup_keyboard();

extern volatile uint8_t keyboard_buffer_index;
extern volatile char keyboard_buffer[256];
