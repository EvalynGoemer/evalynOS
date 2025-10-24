#include "pic.h"
#include "ports.h"
#include "../interupts/pit.h"

#define PIT_CONTROL_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_FREQUENCY 1193182
#define IRQ0_VECTOR 32

static inline void io_wait() {
    outb(0x80, 0);
}

int pitFrequency;
void pit_sleep_ms(unsigned int ms) {
    int startTicks = pitInteruptsTriggered;
    int targetTicks = startTicks + (ms * pitFrequency) / 1000;

    while (pitInteruptsTriggered < targetTicks) {
        int currentTicks = pitInteruptsTriggered;

        if (currentTicks < startTicks) {
            startTicks = currentTicks;
            targetTicks = startTicks + (ms * pitFrequency) / 1000;
        }

        asm volatile("hlt");
    }
}

void setup_pit(unsigned int frequency) {
    __asm__ __volatile__("cli");
    pitFrequency = frequency;
    unsigned int divisor = PIT_FREQUENCY / frequency;
    outb(PIT_CONTROL_PORT, 0x36 | 0x02);
    io_wait();
    outb(PIT_CHANNEL0_PORT, divisor & 0xFF);
    io_wait();
    outb(PIT_CHANNEL0_PORT, divisor >> 8);
    io_wait();

    unmask_irq(0);
    __asm__ __volatile__("sti");
}
