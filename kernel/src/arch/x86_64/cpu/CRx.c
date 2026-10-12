#include <arch/x86_64/cpu/CRx.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <stdint.h>
#include <stdio.h>

bool smep_enabled = false;
bool smap_enabled = false;
bool xsave_enabled = false;
uint32_t xsave_size = 0;

static void setup_extended_control_regs(bool bsp) {
    if (xsave_enabled) {
        // this cpuid leaf reports the max valid state for xcr0 within edx:eax
        cpuid_regs_t xsave_state = cpuid(CPUID_GET_XSAVE_STATE, CPUID_NO_SUBLEAF);
        uint64_t xcr0 = xsave_state.eax + ((uint64_t)xsave_state.edx << 32);

        // dont save AMX/Tile Registers due to size
        xcr0 &= ~((1ull << XCR0_TILE_DATA_BIT) | (1ull << XCR0_TILE_CONF_BIT));
        // dont save PKRU since the bit in cr4 isnt set
        xcr0 &= ~(1ull << XCR0_PKRU_BIT);

        write_xcr(0, xcr0);
        if (bsp) {
            // refresh state after modifying XCR0; ebx contains the current save size
            xsave_state = cpuid(CPUID_GET_XSAVE_STATE, CPUID_NO_SUBLEAF);
            LOG_TAGGED("ARCH EARLY INIT", ANSI_BYELLOW, "XSAVE Save Size: %lu bytes", xsave_state.ebx);
            xsave_size = xsave_state.ebx;
        }
    }
}

void setup_control_regs(bool bsp) {
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

    if (bsp) {
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
        if (cpuid_check(CPUID_HAS_XSAVE)) {
            xsave_enabled = true;
            cr4 |= (1ull << CR4_BIT_OSXSAVE);
            LOG_TAGGED_OK("ARCH EARLY INIT", ANSI_BYELLOW, "XSAVE Enabled")
        }
    } else {
        if (smep_enabled)  cr4 |= (1ull << CR4_BIT_SMEP);
        if (smap_enabled)  cr4 |= (1ull << CR4_BIT_SMAP);
        if (xsave_enabled) cr4 |= (1ull << CR4_BIT_OSXSAVE);
    }

    write_cr4(cr4);
    setup_extended_control_regs(bsp);

    if (bsp) LOG_TAGGED_OK("ARCH EARLY INIT", ANSI_BYELLOW, "Control Registers Init");
}
