#include <arch/generic/panic.h>
#include <arch/intrin/interrupts.h>
#include <arch/riscv64/timer/timer.h>
#include <arch/riscv64/intrin/spin.h>
#include <arch/riscv64/intrin/csr.h>
#include <arch/riscv64/sbi/timer.h>
#include <arch/riscv64/sbi/probe.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
    TIMER_INTERFACE_UNKNOWN,
    TIMER_INTERFACE_MODERN_SBI,
    TIMER_INTERFACE_LEGACY_SBI,
    TIMER_INTERFACE_STIMECMP,
} timer_interface_t;

static timer_interface_t type = TIMER_INTERFACE_UNKNOWN;
uint64_t timebase_frequency = 0;

static uint64_t freq_shift;
static uint64_t freq_mult;
static uint64_t tick_shift;
static uint64_t tick_mult;

static inline uint64_t rdtime() {
    uint64_t t;
    asm volatile ("rdtime %0" : "=r"(t));
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
    if (timebase_frequency == 0)
        panic("Unable to setup timer: No known frequency");

    freq_shift = find_reciprocal_shift(1000000000ull, timebase_frequency);
    tick_shift = find_reciprocal_shift(timebase_frequency, 1000000000ull);
    freq_mult  = ((__uint128_t)1000000000ull << freq_shift) / timebase_frequency;
    tick_mult  = ((__uint128_t)timebase_frequency << tick_shift) / 1000000000ull;

    // TODO: check for Sstc / stimecmp support

    if (sbi_probe_extension(SBI_SET_TIMER.eid)) {
        type = TIMER_INTERFACE_MODERN_SBI;
        LOG_TAGGED("TIME", ANSI_BBLUE, "Timer interrupt setup using SBI 0.2+");
    } else if (legacy_sbi_set_timer(~0ull)) {
        type = TIMER_INTERFACE_LEGACY_SBI;
        LOG_TAGGED("TIME", ANSI_BBLUE, "Timer interrupt setup using SBI 0.1");
    }

    if (type == TIMER_INTERFACE_UNKNOWN)
        panic("Unable to setup timer: No way to set stimecmp");

    timer_set_timeout_ms(1);
    csrs(CSR_SIE, SIE_STIE);
    enable_interrupts();
}

uint64_t timer_get_ns() {
    if (timebase_frequency == 0)
        return 0;
    uint64_t ticks = rdtime();
    return ((__uint128_t)ticks * freq_mult) >> freq_shift;
}

static uint64_t timer_get_ticks_from_ns(uint64_t ns) {
    if (timebase_frequency == 0)
        return 0;
    return ((__uint128_t)ns * tick_mult) >> tick_shift;
}

void timer_set_timeout_ms(uint64_t ms) {
    uint64_t curr_time = rdtime();
    uint64_t next_time = curr_time + timer_get_ticks_from_ns(ms * 1000000);

    switch (type) {
        case TIMER_INTERFACE_LEGACY_SBI: legacy_sbi_set_timer(next_time); break;
        case TIMER_INTERFACE_MODERN_SBI: sbi_set_timer(next_time);        break;
        case TIMER_INTERFACE_STIMECMP:   csrw(CSR_STIMECMP, next_time);   break;
        default: panic("timer_set_timeout_ms() called before setup");
    }
}

void timer_spin_wait_ms(int ms) {
    if (timebase_frequency == 0)
        return;
    uint64_t start = rdtime();
    uint64_t ticks = timer_get_ticks_from_ns(ms * 1000000);
    while ((rdtime() - start) < ticks)
        spin();
}
