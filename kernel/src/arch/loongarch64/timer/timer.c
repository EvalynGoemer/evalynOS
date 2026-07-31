#include <arch/intrin/interrupts.h>
#include "utils/lib.h"
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <arch/loongarch64/intrin/csr.h>
#include <arch/loongarch64/cpu/cpucfg.h>
#include <arch/generic/panic.h>
#include <arch/loongarch64/timer/timer.h>

static uint64_t freq_shift = 0;
static uint64_t freq_mult = 0;
static uint64_t tick_shift = 0;
static uint64_t tick_mult = 0;

static inline uint64_t rdtime() {
    uint64_t t;
    asm volatile ("rdtime.d %0, $zero" : "=r"(t));
    return t;
}

void setup_timer() {
    if (!cpucfg_check(CPUCFG_HAS_TIMER))
        panic("CPU does not have a timer");

    uint32_t freq = cpucfg(CPUCFG_INDEX_TIMER_FREQ);
    uint32_t mudv = cpucfg(CPUCFG_INDEX_TIMER_MUDV);
    uint16_t mul = mudv & 0xFFFF;
    uint16_t div = mudv >> 16;

    if (freq == 0 || mul == 0 || div == 0)
        panic("CPU does not enumerate a valid timer frequency");

    uint64_t timer_frequency = ((uint64_t)freq * mul) / div;

    if (timer_frequency >= 1000000) {
        LOG_TAGGED("TIME", ANSI_BBLUE, "Timer Frequency: ((%dhz * %d) / %d) = %lld MHz", freq, mul, div, timer_frequency / 1000000);
    } else {
        LOG_TAGGED("TIME", ANSI_BBLUE, "Timer Frequency: ((%dhz * %d) / %d) = %lld Hz", freq, mul, div, timer_frequency);
    }

    // ticks -> ns
    freq_shift = find_reciprocal_shift(1000000000ull, timer_frequency);
    freq_mult  = ((__uint128_t)1000000000ull << freq_shift) / timer_frequency;
    // ms -> ticks
    tick_shift = find_reciprocal_shift(timer_frequency, 1000);
    tick_mult  = ((__uint128_t)timer_frequency << tick_shift) / 1000ull;

    csrxchg(CSR_ECFG, CSR_ECFG_TIMER_EN, CSR_ECFG_TIMER_EN);
    timer_set_timeout_ms(5);
    enable_interrupts();

    LOG_TAGGED("TIME", ANSI_BBLUE, "Timer interrupt setup");
}

void timer_set_timeout_ms(int ms) {
    uint64_t time = ((__uint128_t)ms * tick_mult) >> tick_shift;
    // time must be a multiple of 4 due to the config bits
    time = ALIGN_UP(time, 4);
    // TCFG[0] == 1 enables the timer
    // TCFG[1] == 0 means oneshot mode
    csrwr(CSR_TCFG, time | 0b01);
    // clear the old timer irq
    csrwr(CSR_TICLR, 1);
}

uint64_t timer_get_ns() {
    uint64_t ticks = rdtime();
    return ((__uint128_t)ticks * freq_mult) >> freq_shift;
}

void timer_spin_wait_ms(int ms) {
    uint64_t start = rdtime();
    uint64_t ticks = ((__uint128_t)ms * tick_mult) >> tick_shift;
    while ((rdtime() - start) < ticks)
        spin();
}
