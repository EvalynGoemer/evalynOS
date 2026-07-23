#pragma once

#include <acpi/tables/sdt.h>
#include <stdint.h>
#include <stdbool.h>

#define RHCT_SDT_SIGNATURE "RHCT"

struct [[gnu::packed]] RHCT {
    struct SDTHeader header;
    uint32_t flags;
    uint64_t timebase_freq;
    uint32_t node_count;
    uint32_t node_array_offset;
};

extern bool acpi_parse_rhct();
