#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <acpi/tables/madt.h>

void acpi_parse_madt() {
    uint8_t* madt_base = acpi_find_sdt("APIC");

    if (!madt_base) {
        LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "MADT table is not present")
        return;
    }

    arch_madt_parse_start();

    struct MADT* madt = (struct MADT*)madt_base;
    uint8_t* ptr = madt_base + sizeof(struct MADT);
    uint8_t* end = madt_base + madt->header.length;

    while (ptr + sizeof(struct MADTEntryHeader) <= end) {
        struct MADTEntryHeader* entry = (struct MADTEntryHeader*)ptr;
        if (ptr + entry->length > end)
            break;
        if (entry->length < sizeof(struct MADTEntryHeader))
            break;

        arch_madt_handle_entry(entry);

        ptr += entry->length;
    }

    arch_madt_parse_end();
}
