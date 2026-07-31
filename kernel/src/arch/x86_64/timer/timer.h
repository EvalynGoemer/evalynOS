#pragma once
#include <stdint.h>
#include <stdbool.h>

#define ACPI_TIMER_FREQUENCY 3579545

#define HPET_TIMER_ROLLOVER  (1ull << 32)
#define ACPI_TIMER_ROLLOVER  (1ull << 24)

typedef enum {
    x86_WALLCLOCK_TSC,
    x86_WALLCLOCK_ACPI_PMT,
    x86_WALLCLOCK_HPET,
    x86_WALLCLOCK_PVCLOCK,
} x86_wallclock_t;

extern x86_wallclock_t current_wallclock;

extern void setup_acpi_pmt();
extern uint32_t acpi_pmt_get_ticks();

extern bool setup_hpet();
extern uint64_t hpet_get_frequency();
extern uint32_t hpet_get_ticks();

extern uint64_t setup_tsc();
#define tsc_get_ticks() __builtin_ia32_rdtsc()

extern void setup_timers(uint32_t core_id);
extern uint64_t timer_get_ns();
extern void timer_spin_wait_ms(int ms);
