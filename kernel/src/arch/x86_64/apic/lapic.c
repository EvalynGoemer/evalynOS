#include <arch/intrin/spin.h>
#include <arch/generic/paging/paging.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <mem/pmm.h>
#include <mem/vmem.h>
#include <utils/lib.h>
#include <arch/intrin/mmio.h>
#include <arch/x86_64/cpu/msr.h>
#include <arch/x86_64/apic/lapic.h>
#include <assert.h>
#include <stdint.h>

static uint64_t lapic_pbase = 0;
static uint64_t lapic_vbase = 0;
bool x2apic = false;

#define APIC_REGISTER_ID     0x020
#define APIC_REGISTER_EOI    0x0B0
#define APIC_REGISTER_SVR    0x0F0
#define APIC_REGISTER_TIMER  0x320
#define APIC_REGISTER_DIVIDE 0x3E0
#define APIC_REGISTER_ICOUNT 0x380
#define APIC_REGISTER_CCOUNT 0x390
#define APIC_REGISTER_ICRL   0x300 // always use this in x2apic mode
#define APIC_REGISTER_ICRH   0x310

static void write_lapic_register(uint32_t reg, uint32_t data) {
    if (x2apic) {
        reg = (reg >> 4) + 0x800;
        assert(0x800 <= reg && reg <= 0x8FF);
        wrmsr(reg, data);
        return;
    }

    mmio_write_offset_32(lapic_vbase, reg, data);
}

static uint32_t read_lapic_register(uint32_t reg) {
    if (x2apic) {
        reg = (reg >> 4) + 0x800;
        assert(0x800 <= reg && reg <= 0x8FF);
        return rdmsr(reg);
    }

    return mmio_read_offset_32(lapic_vbase, reg);
}

void setup_lapic() {
    x2apic = cpuid_check(CPUID_HAS_x2APIC);

    if (x2apic) {
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

    if (!x2apic && lapic_pbase == 0) {
        lapic_pbase = rdmsr(MSR_APIC_BASE) & ~0xFFF;
        assert(lapic_pbase != 0);
    }

    if(!x2apic && lapic_vbase == 0) {
        lapic_vbase = vmem_alloc(&kernel_vmem_allocator, PAGE_SIZE, 0);
        paging_map_page(kernel_page_table, lapic_vbase, lapic_pbase, PAGE_KRW_UC, PAGE_SIZE_NORM);
    }

    // ensure the lapic pbase is the same on all APs
    if (!x2apic) {
        uint64_t lapic_conf = rdmsr(MSR_APIC_BASE) & 0xFFF;
        wrmsr(MSR_APIC_BASE, lapic_conf | lapic_pbase);
    }

    uint32_t apic_svr = read_lapic_register(APIC_REGISTER_SVR);
    apic_svr |= 0x1FF; // enable lapic & enable spurious vector on vector 0xFF
    write_lapic_register(APIC_REGISTER_SVR, apic_svr);

    // TODO: setup lapic timer; needs other time drivers to be finished for calibration
}

uint32_t arch_get_local_coreid() {
    if (x2apic)
        return read_lapic_register(APIC_REGISTER_ID);
    return read_lapic_register(APIC_REGISTER_ID) >> 24;
}

static void await_ipideliver() {
    if (x2apic) return;
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
