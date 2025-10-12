#pragma once
#include "ports.h"

#define PIT_CONTROL_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_FREQUENCY 1193182
#define IRQ0_VECTOR 32

extern void setup_pit(unsigned int frequency);
void pit_sleep_ms(unsigned int ms);
