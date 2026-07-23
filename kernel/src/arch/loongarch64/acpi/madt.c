#include <stdio.h>
#include <acpi/tables/madt.h>

static uint32_t madt_entries = 0;
static uint32_t unknown_madt_entries = 0;

static void handle_entry(struct MADTEntryHeader* entry) {
    madt_entries++;
    switch (entry->type) {
        // TODO: parse loongarch64 specific entries
        default: {
            unknown_madt_entries++;
            break;
        }
    }
}

void acpi_parse_madt() {
    uint8_t* madt_base = acpi_find_sdt(MADT_SDT_SIGNATURE);

    if (!madt_base) {
        LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "MADT table is not present")
        return;
    }

    struct MADTEntryHeader* entry;
    MADT_FOR_EACH_ENTRY(madt_base, entry) {
        handle_entry(entry);
    }

    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d known entries in the MADT", madt_entries - unknown_madt_entries);
    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d unknown entries in the MADT", unknown_madt_entries);
}
