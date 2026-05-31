#include <stdio.h>
#include <acpi/tables/madt.h>

static uint32_t madt_entries = 0;
static uint32_t unknown_madt_entries = 0;

void arch_madt_parse_start() {

}

void arch_madt_parse_end() {
    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d known entries in the MADT", madt_entries - unknown_madt_entries);
    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d unknown entries in the MADT", unknown_madt_entries);
}

void arch_madt_handle_entry(struct MADTEntryHeader* entry) {
    madt_entries++;
    switch (entry->type) {
        // TODO: parse loongarch64 specific entries
        default: {
            unknown_madt_entries++;
            break;
        }
    }
}
