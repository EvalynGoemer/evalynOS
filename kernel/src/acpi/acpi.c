#include <acpi/tables/sdt.h>
#include <acpi/tables/madt.h>
#include <acpi/tables/fadt.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

bool acpi_works = false;

bool verify_acpi() {
    if(!acpi_verify_rsdp())
        return false;

    int count = acpi_table_count();
    LOG_TAGGED("ACPI", ANSI_BMAGENTA, "Found %d tables in the RSDT", count)
    for (int i = 0; i < count; i++) {
        struct SDTHeader* header = acpi_get_sdt(i);

        char signature[5]  = {0};
        char oemID[7]      = {0};
        char oemTableID[9] = {0};
        memcpy(signature,  header->signature, 4);
        memcpy(oemID,      header->oemID,     6);
        memcpy(oemTableID, header->oemTableID, 8);

        if (acpi_check_sdt_checksum(header))
            LOG_TAGGED_PASS("ACPI", ANSI_BMAGENTA, "%4s 0x%016llx %08x v%02d {%6s %8s}", signature, (uint64_t)header, header->length, header->revision, oemID, oemTableID)
        else
            LOG_TAGGED_WARN("ACPI", ANSI_BMAGENTA, "%4s 0x%016llx %08x v%02d {%6s %8s}", signature, (uint64_t)header, header->length, header->revision, oemID, oemTableID)
    }

    acpi_verify_fadt();

    acpi_works = true;
    return true;
}
