#include <arch/intrin/interrupts.h>
#include "utils/lib.h"
#include <stdint.h>
#include <stdio.h>
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

static uint32_t find_reciprocal_shift(uint64_t numerator, uint64_t denominator) {
    __uint128_t lim = denominator * (__uint128_t)UINT64_MAX;

    uint32_t lo = 0, hi = 127;
    while (lo < hi) {
        uint32_t mid = (lo + hi + 1) >> 1;
        if (((__uint128_t)numerator << mid) <= lim)
            lo = mid;
        else
            hi = mid - 1;
    }

    return lo;
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

    freq_shift = find_reciprocal_shift(1000000000ull, timer_frequency);
    tick_shift = find_reciprocal_shift(timer_frequency, 1000000000ull);
    freq_mult  = ((__uint128_t)1000000000ull << freq_shift) / timer_frequency;
    tick_mult  = ((__uint128_t)timer_frequency << tick_shift) / 1000000000ull;

    csrxchg(CSR_ECFG, CSR_ECFG_TIMER_EN, CSR_ECFG_TIMER_EN);
    timer_set_timeout_ms(5);
    enable_interrupts();

    LOG_TAGGED("TIME", ANSI_BBLUE, "Timer interrupt setup");
}

static uint64_t timer_get_ticks_from_ns(uint64_t ns) {
    if (!tick_mult && !tick_shift)
        return 0;
    return ((__uint128_t)ns * tick_mult) >> tick_shift;
}

void timer_set_timeout_ms(uint64_t ms) {
    uint64_t time = timer_get_ticks_from_ns(ms * 1000000);
    // time must be a multiple of 4 due to the config bits
    time = ALIGN_UP(time, 4);
    // TCFG[0] == 1 enables the timer
    // TCFG[1] == 0 means oneshot mode
    csrwr(CSR_TCFG, time | 0b01);
    // clear the old timer irq
    csrwr(CSR_TICLR, 1);
}

uint64_t timer_get_ns() {
    if (!freq_mult && !freq_shift)
        return 0;
    uint64_t ticks = rdtime();
    return ((__uint128_t)ticks * freq_mult) >> freq_shift;
}
