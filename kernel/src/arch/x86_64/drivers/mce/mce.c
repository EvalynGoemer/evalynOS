#include <arch/x86_64/drivers/mce/mce.h>
#include <arch/generic/panic.h>
#include <arch/x86_64/panic.h>
#include <arch/x86_64/cpu/msr.h>
#include <arch/x86_64/cpu/CRx.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/cpu/interrupts.h>
#include <stdint.h>
#include <stdio.h>

bool mce_supported = false;
bool mca_supported = false;

static uint32_t fatal_mce_flag = 0;
static inline void do_fatal_mce(interrupt_frame_t* frame) {
    if (__atomic_exchange_n(&fatal_mce_flag, 1, __ATOMIC_SEQ_CST) != 0)
        hcf();
    // allow fatal #MCE's to recurse other panics once
    __atomic_store_n(&panic_flag, 0, __ATOMIC_SEQ_CST);
    panic_interrupt("Fatal Machine Check Exception Received", frame);
}

static inline void mce_clear_banks() {
    uint8_t count = rdmsr(MSR_MCE_CAP) & 0xFF;
    for (int i = 0; i < count; i++)
        wrmsr(MSR_MCEn_STATUS(i), 0);
}

void setup_mce() {
    mce_supported = cpuid_check(CPUID_HAS_MCE);
    mca_supported = cpuid_check(CPUID_HAS_MCA);

    if (mce_supported && mca_supported) {
        uint64_t mcg_cap = rdmsr(MSR_MCE_CAP);
        uint8_t count = mcg_cap & 0xFF;

        LOG_TAGGED("MCE", ANSI_BMAGENTA, "CPU has %d MCE Banks", count)

        if (mcg_cap & MCE_CAP_BIT_CTL_REG_PRESENT)
            wrmsr(MSR_MCE_CTL, 0xFFFFFFFFFFFFFFFF);

        for (int i = 0; i < count; i++) {
            wrmsr(MSR_MCEn_CTL(i), 0xFFFFFFFFFFFFFFFF);
            wrmsr(MSR_MCEn_STATUS(i), 0);
        }

        set_cr4_bit(CR4_BIT_MCE);
        LOG_TAGGED("MCE", ANSI_BMAGENTA, "MCE+MCA Exception MSRs Setup")
        return;
    }

    if (mce_supported) {
        set_cr4_bit(CR4_BIT_MCE);
        LOG_TAGGED("MCE", ANSI_BMAGENTA, "MCE Exception MSRs Setup")
    }
}

void handle_exception_mce(interrupt_frame_t* frame) {
    if (__atomic_load_n(&fatal_mce_flag, __ATOMIC_SEQ_CST) != 0)
        hcf();
    if (!mca_supported)
        do_fatal_mce(frame);

    uint64_t mce_status = rdmsr(MSR_MCE_STATUS);
    bool can_continue = mce_status & MCE_STATUS_RESTART_IP_VALID;

    // TODO; get all MCE exceptions and log it before clearing the banks
    mce_clear_banks();

    if (!can_continue)
        do_fatal_mce(frame);

    // TODO; make this goto an NMI safe log buffer
    LOG_TAGGED("MCE", ANSI_BMAGENTA, "Got a non fatal #MCE")

    // tell the CPU the #MCE was handled and to not die on the next one
    wrmsr(MSR_MCE_STATUS, mce_status & ~MCE_STATUS_IN_PROGRESS_FLAG);
}
