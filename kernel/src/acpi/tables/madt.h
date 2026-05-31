#pragma once

#include <acpi/tables/sdt.h>
#include <stdint.h>

typedef enum: uint8_t {
    MADT_TYPE_APIC          = 0x00,
    MADT_TYPE_x2APIC        = 0x09,
    MADT_TYPE_IOAPIC        = 0x01,
    MADT_TYPE_IRQ_OVERRIDE  = 0x02,
    MADT_TYPE_APIC_NMI      = 0x04, // ignored
    MADT_TYPE_x2APIC_NMI    = 0x0A, // ignored
    MADT_TYPE_IOAPIC_NMI    = 0x03, // ignored
    MADT_TYPE_APIC_ADDR_OVR = 0x05, // ignored
} MADTType;

struct [[gnu::packed]] MADT {
    struct SDTHeader header;
    uint32_t lapic_address;
    uint32_t flags;
};

struct [[gnu::packed]] MADTEntryHeader {
    MADTType type;
    uint8_t length;
};

extern void acpi_parse_madt();

extern void arch_madt_parse_start();
extern void arch_madt_parse_end();
extern void arch_madt_handle_entry(struct MADTEntryHeader* entry);
