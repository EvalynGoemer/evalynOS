#include "ports.h"

#define PIT_CONTROL_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_FREQUENCY 1193182
#define IRQ0_VECTOR 32

void setup_pit(unsigned int frequency) {
    unsigned int divisor = PIT_FREQUENCY / frequency;
    outb(PIT_CONTROL_PORT, 0x36 | 0x02);
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);
    outb(PIT_CHANNEL0_PORT, divisor >> 8);
}
