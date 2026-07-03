#include <arch/intrin/cpulocal.h>
#include <arch/riscv64/cpu/cpulocal.h>
#include <arch/riscv64/intrin/csr.h>
#include <stdint.h>

[[gnu::section(".cpu_local_fixed")]]
fixed_cpu_local_t fixed_cpu_local = {0};

void setup_cpulocal_bsp() {
    SET_CPU_LOCAL((uint64_t)&__cpu_local_start);
    csrw(CSR_SSCRATCH, (uint64_t)&__cpu_local_start);
}
