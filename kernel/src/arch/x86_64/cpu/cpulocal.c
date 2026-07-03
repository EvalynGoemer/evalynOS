#include <arch/intrin/cpulocal.h>
#include <arch/x86_64/cpu/cpulocal.h>

[[gnu::section(".cpu_local_fixed")]]
fixed_cpu_local_t fixed_cpu_local = {0};

void setup_cpulocal_bsp() {
    SET_CPU_LOCAL(0);
}
