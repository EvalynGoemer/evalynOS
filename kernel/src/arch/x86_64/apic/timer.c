#include <math.h>
#include <arch/generic/timer/timer.h>
#include <arch/intrin/spin.h>
#include <arch/intrin/cpulocal.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/apic/helpers.h>

static uint64_t tick_shift = 0;
static uint64_t tick_mult  = 0;

void calibrate_lapic_timer() {
    write_lapic_register(APIC_REGISTER_TIMER, LAPIC_TIMER_VECTOR | LAPIC_TIMER_MODE_MASKED);
    write_lapic_register(APIC_REGISTER_DIVIDE, LAPIC_TIMER_DIVIDE_1);
    uint64_t total = 0;
    for (int i = 0; i < 3; i++) {
        write_lapic_register(APIC_REGISTER_ICOUNT, 0xFFFFFFFF);
        timer_spin_wait_ms(10);
        uint32_t remaining = read_lapic_register(APIC_REGISTER_CCOUNT);
        total += (uint64_t)(0xFFFFFFFF - remaining) * 100;
    }
    uint64_t frequency = total / 3;

    tick_shift = find_reciprocal_shift(frequency, 1000);
    tick_mult  = ((__uint128_t)frequency << tick_shift) / 1000;
}

void timer_set_timeout_ms(int ms) {
    uint64_t ticks = ((__uint128_t)ms * tick_mult) >> tick_shift;
    write_lapic_register(APIC_REGISTER_ICOUNT, ticks);
}

void arch_send_eoi() {
    write_lapic_register(APIC_REGISTER_EOI, 0);
}
