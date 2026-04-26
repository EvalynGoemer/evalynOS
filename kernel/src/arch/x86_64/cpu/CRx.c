#include <arch/x86_64/cpu/CRx.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <stdint.h>
#include <stdio.h>

bool smep_enabled = false;
bool smap_enabled = false;

void setup_control_regs() {
    uint64_t cr0 = read_cr0();
    cr0 &= ~(1ull << CR0_BIT_EM);
    cr0 &= ~(1ull << CR0_BIT_TS);
    cr0 |= (1ull << CR0_BIT_MP);
    cr0 |= (1ull << CR0_BIT_NE);
    write_cr0(cr0);

    uint64_t cr4 = read_cr4();
    cr4 |= (1ull << CR4_BIT_PSE);
    cr4 |= (1ull << CR4_BIT_OSFXSR);
    cr4 |= (1ull << CR4_BIT_OSXMMEXCPT);

    if (cpuid_check(CPUID_HAS_SMEP)) {
        smep_enabled = true;
        cr4 |= (1ull << CR4_BIT_SMEP);
        LOG_TAGGED_OK("ARCH EARLY INIT", ANSI_BYELLOW, "SMEP Enabled")
    }
    if (cpuid_check(CPUID_HAS_SMAP)) {
        smap_enabled = true;
        cr4 |= (1ull << CR4_BIT_SMAP);
        LOG_TAGGED_OK("ARCH EARLY INIT", ANSI_BYELLOW, "SMAP Enabled")
    }

    write_cr4(cr4);

    LOG_TAGGED_OK("ARCH EARLY INIT", ANSI_BYELLOW, "Control Registers Init")
}
