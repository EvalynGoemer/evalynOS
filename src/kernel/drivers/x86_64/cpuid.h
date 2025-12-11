#pragma once
#include <stdint.h>

enum {
    CPUID_HYPERVISOR = 31,
};

struct cpuid_regs {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
};

extern struct cpuid_regs cpuid(uint32_t leaf, uint32_t subleaf);
extern int cpu_feature_bit(uint32_t leaf, uint32_t subleaf, char reg, int bit);
extern char* get_cpu_vendor();
extern char* get_hypervisor_id();
extern char* get_cpu_name();
