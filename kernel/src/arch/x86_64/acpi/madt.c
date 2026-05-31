#include "utils/dstruct/llist.h"
#include <stdio.h>
#include <string.h>
#include <acpi/tables/madt.h>
#include <arch/x86_64/acpi/madt.h>
#include <mem/spalloc.h>

bstree_t detected_apics   = BSTREE_INIT;
llist_t  detected_ioapics = LLIST_INIT;
llist_t  irq_overrides    = LLIST_INIT;

static spalloc_allocator_t alloc;
static uint32_t detected_cpus;
static uint32_t madt_entries = 0;
static uint32_t unknown_madt_entries = 0;

static uint64_t detected_apic_get_value(bstree_node_t* node) {
    detected_apic_t* x = CONTAINER_OF(node, detected_apic_t, node);
    return x->apic_id;
}

void arch_madt_parse_start() {
    spalloc_init(&alloc, 64, 8);
    detected_apics = (bstree_t){detected_apic_get_value, BST_TYPE_RB, 0};
}

void arch_madt_parse_end() {
    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d known entries in the MADT", madt_entries - unknown_madt_entries);
    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d unknown entries in the MADT", unknown_madt_entries);
    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d CPU(s)", detected_cpus);
    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Detected IOAPICs:");
    llist_node_t* ioapic_node = detected_ioapics.head;
    while (ioapic_node) {
        detected_ioapic* ioapic = CONTAINER_OF(ioapic_node, detected_ioapic, node);
        LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "  IOAPIC id=%u phys=0x%08x gsi_base=%u",
                   ioapic->ioapic_id, ioapic->phys_addr, ioapic->gsi_base);
        ioapic_node = ioapic_node->next;
    }

    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Detected IRQ overrides:");
    llist_node_t* irq_node = irq_overrides.tail;
    while (irq_node) {
        acpi_irq_override_t* ovr = CONTAINER_OF(irq_node, acpi_irq_override_t, node);
        LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "  IRQ %u -> GSI %u flags=0x%02x",
                   ovr->irq, ovr->gsi, ovr->flags);
        irq_node = irq_node->prev;
    }
}

static void madt_register_lapic(uint32_t apic_id, uint32_t acpi_id, uint32_t flags) {
    // bit one not being set means this CPU is not usable
    if (!(flags & 0x1))
        return;

    bstree_node_t* lnode = bstree_search(&detected_apics, apic_id, BST_SEARCH_TYPE_EXACT);
    if (lnode) {
        LOG_TAGGED_WARN("ACPI/MADT", ANSI_BMAGENTA, "Firmware Bug: Duplicate Entry for LAPIC 0x%08x", apic_id);
        detected_apic_t* node = CONTAINER_OF(lnode, detected_apic_t, node);
        node->acpi_id = acpi_id;
        node->flags   = flags;
        return;
    }
    detected_apic_t* node = spalloc_malloc(&alloc);
    memset(node, 0, sizeof(detected_apic_t));
    node->apic_id = apic_id;
    node->acpi_id = acpi_id;
    node->flags   = flags;
    bstree_insert(&detected_apics, &node->node);
    detected_cpus++;
}

void arch_madt_handle_entry(struct MADTEntryHeader* entry) {
    madt_entries++;
    switch (entry->type) {
        case MADT_TYPE_APIC: {
            struct MADT_apic* lapic = (struct MADT_apic*)entry;
            madt_register_lapic(lapic->apic_id, lapic->acpi_id, lapic->flags);
            break;
        }
        case MADT_TYPE_x2APIC: {
            struct MADT_x2apic* x2apic = (struct MADT_x2apic*)entry;
            madt_register_lapic(x2apic->x2apic_id, x2apic->acpi_id, x2apic->flags);
            break;
        }
        case MADT_TYPE_IOAPIC: {
            struct MADT_ioapic* ioapic = (struct MADT_ioapic*)entry;
            detected_ioapic* entry = spalloc_malloc(&alloc);
            memset(entry, 0, sizeof(detected_ioapic));
            entry->ioapic_id = ioapic->ioapic_id;
            entry->phys_addr = ioapic->ioapic_address;
            entry->gsi_base  = ioapic->gsi_base;
            llist_push(&detected_ioapics, &entry->node);
            break;
        }
        case MADT_TYPE_IRQ_OVERRIDE: {
            struct MADT_irq_override* irq_override = (struct MADT_irq_override*)entry;
            acpi_irq_override_t* entry = spalloc_malloc(&alloc);
            memset(entry, 0, sizeof(acpi_irq_override_t));
            entry->irq = irq_override->irq;
            entry->gsi = irq_override->gsi;
            entry->flags = irq_override->flags;
            llist_push(&irq_overrides, &entry->node);
            break;
        }
        // ignored entries
        case MADT_TYPE_APIC_NMI:
        case MADT_TYPE_x2APIC_NMI:
        case MADT_TYPE_IOAPIC_NMI:
        case MADT_TYPE_APIC_ADDR_OVR: {
            break;
        }
        default: {
            unknown_madt_entries++;
            break;
        }
    }
}
