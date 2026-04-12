#include <arch/generic/panic.h>
#include <arch/x86_64/cpu/interrupts.h>
#include <arch/x86_64/cpu/msr.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/drivers/fred/fred.h>
#include <arch/x86_64/descriptor_tables/gdt.h>
#include <stdio.h>

#define FRED_STKLVL(n, lvl)  ((uint64_t)(lvl) << ((n) * 2))

bool fred_enabled = false;

bool setup_fred_bsp() {
    if (!cpuid_check(CPUID_HAS_FRED))
        return false;

    asm volatile (
        "mov %%cr4, %%rax\n"
        "bts $32, %%rax\n"    // set FRED
        "mov %%rax, %%cr4"
        :
        :
        : "rax", "memory"
    );

    uint64_t star = ((uint64_t)(0x18 | 3) << 48) | ((uint64_t)0x08 << 32);
    wrmsr(MSR_STAR, star);
    wrmsr(MSR_FRED_CONFIG, (uint64_t)fred_ring3_entry_asm_stub);
    wrmsr(MSR_FRED_RSP2,   (uint64_t)&bsp_df_stack     + sizeof(bsp_df_stack));
    wrmsr(MSR_FRED_RSP3,   (uint64_t)&bsp_nmi_stack    + sizeof(bsp_nmi_stack));
    wrmsr(MSR_FRED_STKLVLS,
        FRED_STKLVL(INTERRUPT_DOUBLE_FAULT,            2) |
        FRED_STKLVL(INTERRUPT_NON_MASKABLE_INTERRUPT,  3)
    );

    printf(ANSI_BYELLOW "[ARCH EARLY INIT]" ANSI_RESET " FRED INIT " ANSI_BGREEN "[OK]\n" ANSI_RESET);

    fred_enabled = true;
    return true;
}

void fred_ring3_entry(fred_frame_t* frame) {
    // uint8_t vector = (frame->cpu_frame.ss >> 32) & 0xFF;
    uint8_t type   = (frame->cpu_frame.ss >> 48) & 0xF;

    if (type == 7) {
        panic("Kernel should not be getting syscalls right now");
    } else {
        panic("Kernel should not be getting intterupts from ring3 right now");
    }
}

void fred_ring0_entry(fred_frame_t* frame) {
    uint8_t vector = (frame->cpu_frame.ss >> 32) & 0xFF;
    printf("GOT VECTOR 0x%02x VIA FRED!!!\n", vector);
    dispatch_interrupt(&frame->regs, &frame->cpu_frame, vector);
}
