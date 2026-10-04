#include "arch/generic/panic.h"
#include <arch/intrin/spin.h>
#include <arch/intrin/interrupts.h>
#include <arch/x86_64/timer/timer.h>
#include <arch/generic/paging/paging.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <mem/pmm.h>
#include <mem/vmem.h>
#include <utils/lib.h>
#include <arch/intrin/mmio.h>
#include <arch/x86_64/cpu/msr.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/apic/helpers.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

uint64_t lapic_vbase = 0;
bool x2apic = false;

static uint64_t lapic_pbase = 0;

static void enable_x2apic() {
    uint64_t apic_base = rdmsr(MSR_APIC_BASE);
    int apic_enabled = apic_base & LAPIC_BASE_x1ENABLE;
    int x2apic_enabled = apic_base & LAPIC_BASE_x2ENABLE;

    if (!apic_enabled) {
        apic_base |= LAPIC_BASE_x1ENABLE;
        wrmsr(MSR_APIC_BASE, apic_base);
    }

    if (!x2apic_enabled) {
        apic_base |= LAPIC_BASE_x2ENABLE;
        wrmsr(MSR_APIC_BASE, apic_base);
    }
}

static inline void lapic_setup_common() {
    uint32_t apic_svr = read_lapic_register(APIC_REGISTER_SVR);
    apic_svr |= 0x1FF; // enable lapic & enable spurious vector on vector 0xFF
    write_lapic_register(APIC_REGISTER_SVR, apic_svr);

    write_lapic_register(APIC_REGISTER_TIMER, LAPIC_TIMER_VECTOR | LAPIC_TIMER_MODE_ONESHOT);
    timer_set_timeout_ms(1);
    enable_interrupts();
}

void setup_lapic_bsp() {
    if (!cpuid_check(CPUID_HAS_APIC))
        panic("CPU does not have the APIC enabled");

    x2apic = cpuid_check(CPUID_HAS_x2APIC);

    if (x2apic) {
        enable_x2apic();
    } else {
        lapic_pbase = rdmsr(MSR_APIC_BASE) & ~0xFFF;
        lapic_vbase = vmem_alloc(&kernel_vmem_allocator, PAGE_SIZE, 0);

        assert(lapic_pbase != 0);
        paging_map_page(kernel_page_table, lapic_vbase, lapic_pbase, PAGE_KRW_UC);
    }

    calibrate_lapic_timer();
    lapic_setup_common();

    LOG_TAGGED("LAPIC", ANSI_BCYAN, "Local APIC Setup");
}

void setup_lapic_ap() {
    if (x2apic) {
        enable_x2apic();
    } else {
        uint64_t lapic_conf = rdmsr(MSR_APIC_BASE) & 0xFFF;
        wrmsr(MSR_APIC_BASE, lapic_conf | lapic_pbase);
    }

    lapic_setup_common();
}

uint32_t arch_get_local_coreid() {
    if (x2apic)
        return read_lapic_register(APIC_REGISTER_ID);
    return read_lapic_register(APIC_REGISTER_ID) >> 24;
}
