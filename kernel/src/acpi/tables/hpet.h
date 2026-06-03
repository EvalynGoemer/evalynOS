#pragma once

#include <acpi/tables/sdt.h>
#include <stdint.h>

#define ACPI_HPET_TABLE_SIGNATURE "HPET"

struct [[gnu::packed]] HPET {
    struct SDTHeader header;
    uint8_t hardware_rev_id;
    uint8_t comparator_info; // [0:4] count | [5] size | [6] reserved | [7] legacy
    uint16_t pci_vendor_id;
    struct ACPIaddr address;
    uint8_t hpet_number;
    uint16_t minimum_tick;
    uint8_t page_protection;
};
