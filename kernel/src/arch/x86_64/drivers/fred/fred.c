#include <arch/generic/panic.h>
#include <arch/x86_64/cpu/interrupts.h>
#include <arch/x86_64/cpu/msr.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/cpu/CRx.h>
#include <arch/x86_64/drivers/fred/fred.h>
#include <arch/x86_64/descriptor_tables/gdt.h>
#include <stdio.h>

#define FRED_STKLVL(n, lvl)  ((uint64_t)(lvl) << ((n) * 2))

bool fred_enabled = false;

bool setup_fred_bsp() {
    if (!cpuid_check(CPUID_HAS_FRED))
        return false;

    set_cr4_bit(CR4_BIT_FRED);

    uint64_t star = ((uint64_t)(0x18 | 3) << 48) | ((uint64_t)0x08 << 32);
    wrmsr(MSR_STAR, star);
    wrmsr(MSR_FRED_CONFIG, (uint64_t)fred_ring3_entry_asm);
    wrmsr(MSR_FRED_RSP1,   (uint64_t)&bsp_df_stack     + sizeof(bsp_df_stack));
    wrmsr(MSR_FRED_RSP2,   (uint64_t)&bsp_nmi_stack    + sizeof(bsp_nmi_stack));
    wrmsr(MSR_FRED_RSP3,   (uint64_t)&bsp_mce_stack    + sizeof(bsp_mce_stack));
    wrmsr(MSR_FRED_STKLVLS,
        FRED_STKLVL(INTERRUPT_DOUBLE_FAULT,            1) |
        FRED_STKLVL(INTERRUPT_NON_MASKABLE_INTERRUPT,  2) |
        FRED_STKLVL(INTERRUPT_MACHINE_CHECK_EXCEPTION, 3)
    );

    printf(ANSI_BYELLOW "[ARCH EARLY INIT]" ANSI_RESET " FRED INIT " ANSI_BGREEN "[OK]\n" ANSI_RESET);

    fred_enabled = true;
    return true;
}
