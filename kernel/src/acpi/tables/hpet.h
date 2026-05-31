#pragma once

#include <acpi/tables/sdt.h>
#include <stdint.h>

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

extern uint64_t hpet_phys;
extern void acpi_parse_hpet();
