#include <arch/intrin/cpulocal.h>
#include <arch/loongarch64/cpu/cpulocal.h>
#include <arch/loongarch64/intrin/csr.h>
#include <stdint.h>

[[gnu::section(".cpu_local_fixed")]]
fixed_cpu_local_t fixed_cpu_local = {0};

void setup_cpulocal_bsp() {
    SET_CPU_LOCAL((uint64_t)&__cpu_local_start);
    csrwr(CSR_SAVE0, (uint64_t)&__cpu_local_start);
}
