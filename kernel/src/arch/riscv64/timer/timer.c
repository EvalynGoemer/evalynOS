#include <arch/riscv64/intrin/spin.h>
#include <stdint.h>

uint64_t timebase_frequency = 0;

static inline uint64_t rdtime() {
    uint64_t t;
    asm volatile ("rdtime %0" : "=r"(t));
    return t;
}

void setup_timer() {
    // TODO: setup the timer to give interrupts
}

uint64_t timer_get_ns() {
    if (timebase_frequency == 0)
        return 0;

    uint64_t ticks = rdtime();
    return ((__uint128_t)ticks * 1000000000ull) / timebase_frequency;
}

void timer_spin_wait_ms(int ms) {
    if (timebase_frequency == 0)
        return;

    uint64_t start = rdtime();
    uint64_t ticks = (timebase_frequency / 1000ull) * ms;

    while ((rdtime() - start) < ticks)
        spin();
}
