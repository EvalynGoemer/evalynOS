#include "ports.h"
#include "../interupts/pit.h"

#define PIT_CONTROL_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_FREQUENCY 1193182
#define IRQ0_VECTOR 32

int pitFrequency;
void pit_sleep_ms(unsigned int ms) {
    int targetTicks = pitInteruptsTriggered + (ms * pitFrequency) / 1000;
    while (pitInteruptsTriggered < targetTicks) {
        asm volatile("hlt");
    }
}

void setup_pit(unsigned int frequency) {
    __asm__ __volatile__("cli");
    pitFrequency = frequency;
    unsigned int divisor = PIT_FREQUENCY / frequency;
    outb(PIT_CONTROL_PORT, 0x36 | 0x02);
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);
    outb(PIT_CHANNEL0_PORT, divisor >> 8);
    __asm__ __volatile__("sti");
}
