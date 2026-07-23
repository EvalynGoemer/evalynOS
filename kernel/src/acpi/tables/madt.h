#pragma once

#include <acpi/tables/sdt.h>
#include <stdint.h>
#include <stddef.h>

#define MADT_SDT_SIGNATURE "APIC"

typedef enum: uint8_t {
    // x86
    MADT_TYPE_APIC          = 0x00,
    MADT_TYPE_x2APIC        = 0x09,
    MADT_TYPE_IOAPIC        = 0x01,
    MADT_TYPE_IRQ_OVERRIDE  = 0x02,
    MADT_TYPE_APIC_NMI      = 0x04, // ignored
    MADT_TYPE_x2APIC_NMI    = 0x0A, // ignored
    MADT_TYPE_IOAPIC_NMI    = 0x03, // ignored
    MADT_TYPE_APIC_ADDR_OVR = 0x05, // ignored

    // riscv
    MADT_TYPE_RINTC = 0x18,
    MADT_TYPE_IMSIC = 0x19,
    MADT_TYPE_APLIC = 0x1A,
    MADT_TYPE_PLIC  = 0x1B,
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

extern struct MADTEntryHeader* madt_first_entry(struct MADT *madt);
extern struct MADTEntryHeader* madt_next_entry(struct MADT* madt, struct MADTEntryHeader *entry);

#define MADT_FOR_EACH_ENTRY(madt, entry) for ((entry) = madt_first_entry((struct MADT*)(madt)); (entry) != NULL; (entry) = madt_next_entry((struct MADT*)(madt), (entry)))
