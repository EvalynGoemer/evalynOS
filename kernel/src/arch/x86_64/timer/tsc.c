#include <stdint.h>

#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/timer/timer.h>

uint64_t setup_tsc() {
    if (cpu_vendor == CPU_VENDOR_INTEL && cpuid(0, 0).eax >= CPUID_GET_FREQ_INFO1) {
        cpuid_regs_t r = cpuid(CPUID_GET_FREQ_INFO1, 0);
        if (r.eax != 0 && r.ebx != 0 && r.ecx != 0)
            return ((uint64_t)r.ecx * r.ebx) / r.eax;
    }

    uint64_t total = 0;

    for (int i = 0; i < 3; i++) {
        uint64_t start = tsc_get_ticks();
        timer_spin_wait_ms(10);
        uint64_t end = tsc_get_ticks();
        total += (end - start) * 100;
    }

    return total / 3;
}
