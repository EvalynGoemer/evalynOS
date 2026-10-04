#pragma once
#include <arch/intrin/mmio.h>
#include <arch/x86_64/cpu/msr.h>
#include <stdint.h>
#include <assert.h>

extern bool x2apic;
extern uint64_t lapic_vbase;

static inline void write_lapic_register(uint32_t reg, uint32_t data) {
    if (x2apic) {
        reg = (reg >> 4) + 0x800;
        assert(0x800 <= reg && reg <= 0x8FF);
        wrmsr(reg, data);
        return;
    }

    mmio_write_offset_32(lapic_vbase, reg, data);
}

static inline uint32_t read_lapic_register(uint32_t reg) {
    if (x2apic) {
        reg = (reg >> 4) + 0x800;
        assert(0x800 <= reg && reg <= 0x8FF);
        return rdmsr(reg);
    }

    return mmio_read_offset_32(lapic_vbase, reg);
}
