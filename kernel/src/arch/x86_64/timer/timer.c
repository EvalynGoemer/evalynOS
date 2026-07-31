#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <utils/defer.h>

#include <arch/x86_64/timer/timer.h>
#include <arch/x86_64/drivers/pvclock/pvclock.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/generic/panic.h>
#include <arch/intrin/cpulocal.h>
#include <sched/preempt.h>
#include <stdio.h>

x86_wallclock_t current_wallclock;

static const uint64_t timer_rollover_point[] = {
    [x86_WALLCLOCK_ACPI_PMT] = ACPI_TIMER_ROLLOVER,
    [x86_WALLCLOCK_HPET]     = HPET_TIMER_ROLLOVER,
    [x86_WALLCLOCK_TSC]      = 0,
    [x86_WALLCLOCK_PVCLOCK]  = 0,
};

CPU_LOCAL static uint64_t last_tick;
CPU_LOCAL static uint64_t rollover;
CPU_LOCAL static uint64_t freq_shift;
CPU_LOCAL static uint64_t freq_mult;

void setup_timers(uint32_t core_id) {
    if (core_id == 0) {
        setup_acpi_pmt();

        uint64_t freq;
        if (is_hypervisor && setup_pvclock()) {
            current_wallclock = x86_WALLCLOCK_PVCLOCK;
            LOG_TAGGED("TIME", ANSI_BBLUE, "Setup PV clock for core %d; Current time %ldns", core_id, pvclock_get_ns());
            return;
        } else if (cpuid_check(CPUID_HAS_INVARIANT_TSC)) {
            freq = setup_tsc();
            current_wallclock = x86_WALLCLOCK_TSC;
            LOG_TAGGED("TIME", ANSI_BBLUE, "TSC calibrated for core %d at %lluHz", core_id, freq);
        } else if (setup_hpet()) {
            freq = hpet_get_frequency();
            current_wallclock = x86_WALLCLOCK_HPET;
            LOG_TAGGED("TIME", ANSI_BBLUE, "HPET setup running at %lluHz", freq);
        } else {
            freq = ACPI_TIMER_FREQUENCY;
            current_wallclock = x86_WALLCLOCK_ACPI_PMT;
            LOG_TAGGED("TIME", ANSI_BBLUE, "ACPI PM Timer setup running at %dHz", ACPI_TIMER_FREQUENCY);
        }

        uint32_t shift = find_reciprocal_shift(1000000000ull, freq);
        CPU_LOCAL_WRITE64(freq_shift, shift);
        CPU_LOCAL_WRITE64(freq_mult, ((__uint128_t)1000000000ull << shift) / freq);
        return;
    }

    // APs do not need to do extra setup when using the ACPI PMT/HPET
    // the info will get carried over when copying cpu local data

    if (current_wallclock == x86_WALLCLOCK_PVCLOCK) {
        setup_pvclock();
        LOG_TAGGED("TIME", ANSI_BBLUE, "Setup PV clock for core %d; Current time %ldns", core_id, pvclock_get_ns());
    }

    if (current_wallclock == x86_WALLCLOCK_TSC) {
        uint64_t freq = setup_tsc();
        LOG_TAGGED("TIME", ANSI_BBLUE, "TSC calibrated for core %d at %lluHz", core_id, freq);
        uint32_t shift = find_reciprocal_shift(1000000000ull, freq);
        CPU_LOCAL_WRITE64(freq_shift, shift);
        CPU_LOCAL_WRITE64(freq_mult, ((__uint128_t)1000000000ull << shift) / freq);
    }
}

static uint64_t timer_get_ticks() {
    uint64_t curr_tick;
    switch (current_wallclock) {
        case x86_WALLCLOCK_TSC: curr_tick = __builtin_ia32_rdtsc(); break;
        case x86_WALLCLOCK_HPET: curr_tick = hpet_get_ticks(); break;
        case x86_WALLCLOCK_ACPI_PMT: curr_tick = acpi_pmt_get_ticks(); break;
        default: return 0;
    }

    if (curr_tick < CPU_LOCAL_READ64(last_tick))
        CPU_LOCAL_WRITE64(rollover, CPU_LOCAL_READ64(rollover) + timer_rollover_point[current_wallclock]);

    CPU_LOCAL_WRITE64(last_tick, curr_tick);

    uint64_t result = CPU_LOCAL_READ64(rollover) + curr_tick;
    return result;
}

uint64_t timer_get_ns() {
    disable_preemption();
    defer enable_preemption();
    if (current_wallclock == x86_WALLCLOCK_PVCLOCK)
        return pvclock_get_ns();
    uint64_t ticks = timer_get_ticks();
    return ((__uint128_t)ticks * CPU_LOCAL_READ64(freq_mult)) >> CPU_LOCAL_READ64(freq_shift);
}
