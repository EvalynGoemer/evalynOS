#pragma once
#include "acpi/tables/madt.h"
#include <stdint.h>

extern void acpi_parse_madt();

struct [[gnu::packed]] MADT_corepic {
    struct MADTEntryHeader header;
    uint8_t version;
    uint32_t acpi_id;
    uint32_t core_id;
    uint32_t flags;
};
