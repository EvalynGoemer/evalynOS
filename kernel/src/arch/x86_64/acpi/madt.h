#pragma once
#include <stdint.h>
#include <acpi/tables/madt.h>
#include <utils/dstruct/llist.h>
#include <utils/dstruct/bstree.h>

struct [[gnu::packed]] MADT_apic {
    struct MADTEntryHeader header;
    uint8_t  acpi_id;
    uint8_t  apic_id;
    uint32_t flags;
};

struct [[gnu::packed]] MADT_x2apic {
    struct MADTEntryHeader header;
    uint16_t reserved;
    uint32_t x2apic_id;
    uint32_t flags;
    uint32_t acpi_id;
};

struct [[gnu::packed]] MADT_ioapic {
    struct MADTEntryHeader header;
    uint8_t  ioapic_id;
    uint8_t  reserved;
    uint32_t ioapic_address;
    uint32_t gsi_base;
};

struct [[gnu::packed]] MADT_irq_override {
    struct MADTEntryHeader header;
    uint8_t  bus;
    uint8_t  irq;
    uint32_t gsi;
    uint16_t flags;
};

// parsed entries

typedef struct detected_apic {
    uint32_t apic_id;
    uint32_t acpi_id;
    uint32_t flags;
    bstree_node_t node;
} detected_apic_t;

typedef struct detected_ioapic {
    uint8_t  ioapic_id;
    uint32_t gsi_base;
    uint32_t phys_addr;
    uint64_t virt_addr;
    llist_node_t node;
} detected_ioapic;

typedef struct acpi_irq_override {
    uint8_t  irq;
    uint32_t gsi;
    uint16_t flags;
    llist_node_t node;
} acpi_irq_override_t;

extern bstree_t detected_apics;
extern llist_t  detected_ioapics;
extern llist_t  irq_overrides;

extern uint32_t detected_cpus;

extern void acpi_parse_madt();

// ensure these can be allocated by the 64b internal allocator
_Static_assert(sizeof(detected_apic_t)     <= 64, "detected_apic_t too large");
_Static_assert(sizeof(detected_ioapic)     <= 64, "detected_ioapic too large");
_Static_assert(sizeof(acpi_irq_override_t) <= 64, "acpi_irq_override_t too large");
_Static_assert(alignof(detected_apic_t)     == 8, "detected_apic_t alignment must be 8");
_Static_assert(alignof(detected_ioapic)     == 8, "detected_ioapic alignment must be 8");
_Static_assert(alignof(acpi_irq_override_t) == 8, "acpi_irq_override_t alignment must be 8");
