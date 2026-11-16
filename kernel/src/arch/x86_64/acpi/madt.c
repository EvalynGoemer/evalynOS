#include "arch/generic/cpu/ap.h"
#include <utils/dstruct/llist.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <acpi/tables/madt.h>
#include <arch/x86_64/acpi/madt.h>
#include <utils/lib.h>

static uint64_t detected_apic_get_value(rbtree_node_t* node) {
    detected_apic_t* x = CONTAINER_OF(node, detected_apic_t, node);
    return x->apic_id;
}

rbtree_t detected_apics   = {.root = nullptr, .value_of_node = detected_apic_get_value};
llist_t  detected_ioapics = LLIST_INIT;
llist_t  irq_overrides    = LLIST_INIT;

static void madt_register_lapic(uint32_t apic_id, uint32_t acpi_id, uint32_t flags) {
    // bit one not being set means this CPU is not usable
    if (!(flags & 0x1))
        return;

    rbtree_node_t* lnode = rbtree_search(&detected_apics, apic_id, RB_SEARCH_TYPE_EXACT);
    if (lnode) {
        LOG_TAGGED_WARN("ACPI/MADT", ANSI_BMAGENTA, "Firmware Bug: Duplicate Entry for LAPIC 0x%08x", apic_id);
        detected_apic_t* node = CONTAINER_OF(lnode, detected_apic_t, node);
        node->acpi_id = acpi_id;
        node->flags   = flags;
        return;
    }

    detected_apic_t* node = kmalloc(sizeof(detected_apic_t));
    memset(node, 0, sizeof(detected_apic_t));
    node->apic_id = apic_id;
    node->acpi_id = acpi_id;
    node->flags   = flags;
    rbtree_insert(&detected_apics, &node->node);
    total_cpus++;
}

void acpi_parse_madt() {
    uint8_t* madt_base = acpi_find_sdt(MADT_SDT_SIGNATURE);

    if (!madt_base) {
        LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "MADT table is not present")
        return;
    }

    uint32_t madt_entries = 0;
    uint32_t unknown_madt_entries = 0;

    struct MADTEntryHeader* entry;
    MADT_FOR_EACH_ENTRY(madt_base, entry) {
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
                detected_ioapic* entry = kmalloc(sizeof(detected_ioapic));
                memset(entry, 0, sizeof(detected_ioapic));
                entry->ioapic_id = ioapic->ioapic_id;
                entry->phys_addr = ioapic->ioapic_address;
                entry->gsi_base  = ioapic->gsi_base;
                llist_push(&detected_ioapics, &entry->node);
                break;
            }
            case MADT_TYPE_IRQ_OVERRIDE: {
                struct MADT_irq_override* irq_override = (struct MADT_irq_override*)entry;
                acpi_irq_override_t* entry = kmalloc(sizeof(acpi_irq_override_t));
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

    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d known entries in the MADT", madt_entries - unknown_madt_entries);
    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d unknown entries in the MADT", unknown_madt_entries);
    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d CPU(s)", total_cpus);

    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Detected IOAPICs:");
    LLIST_FOR_EACH(detected_ioapics, ioapic_node) {
        detected_ioapic* ioapic = CONTAINER_OF(ioapic_node, detected_ioapic, node);
        LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "  IOAPIC id=%u phys=0x%08x gsi_base=%u",
                   ioapic->ioapic_id, ioapic->phys_addr, ioapic->gsi_base);
    }

    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Detected IRQ overrides:");
    LLIST_FOR_EACH_REVERSE(irq_overrides, irq_node) {
        acpi_irq_override_t* ovr = CONTAINER_OF(irq_node, acpi_irq_override_t, node);
        LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "  IRQ %u -> GSI %u flags=0x%02x",
                   ovr->irq, ovr->gsi, ovr->flags);
    }
}
