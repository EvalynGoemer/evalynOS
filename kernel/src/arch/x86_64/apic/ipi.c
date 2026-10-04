#include <arch/intrin/spin.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/apic/helpers.h>

static void await_ipideliver() {
    if (x2apic) {
        asm volatile ("mfence; lfence" ::: "memory");
        return;
    }

    while (read_lapic_register(APIC_REGISTER_ICRL) & LAPIC_ICR_DELIVERY_STATUS)
        spin();
}


void arch_send_ipi(uint32_t target_coreid, uint16_t vector) {
    await_ipideliver();
    if (x2apic) {
        uint64_t icr = ((uint64_t)target_coreid << 32) | vector;
        wrmsr(x2APIC_REGISTER_ICR, icr);
    } else {
        write_lapic_register(APIC_REGISTER_ICRH, target_coreid << 24);
        write_lapic_register(APIC_REGISTER_ICRL, vector);
    }
}

void x86_send_sipi(uint32_t target_coreid, uint8_t starting_page) {
    await_ipideliver();
    if (x2apic) {
        uint64_t icr = ((uint64_t)target_coreid << 32) | LAPIC_ICR_DMODE_SIPI | starting_page;
        wrmsr(x2APIC_REGISTER_ICR, icr);
    } else {
        write_lapic_register(APIC_REGISTER_ICRH, target_coreid << 24);
        write_lapic_register(APIC_REGISTER_ICRL, LAPIC_ICR_DMODE_SIPI | starting_page);
    }
}

void x86_send_init(uint32_t target_coreid) {
    await_ipideliver();
    if (x2apic) {
        uint64_t icr = ((uint64_t)target_coreid << 32) | LAPIC_ICR_DMODE_INIT;
        wrmsr(x2APIC_REGISTER_ICR, icr);
    } else {
        write_lapic_register(APIC_REGISTER_ICRH, target_coreid << 24);
        write_lapic_register(APIC_REGISTER_ICRL, LAPIC_ICR_DMODE_INIT);
    }
}
