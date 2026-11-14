#pragma once
#include <acpi/tables/madt.h>
#include <stdint.h>

extern bool acpi_parse_madt();

struct [[gnu::packed]] MADT_rintc {
    struct MADTEntryHeader header;
    uint8_t version;
    uint8_t _reserved;
    uint32_t flags;
    uint64_t hart_id;
    uint32_t acpi_uid;
    uint32_t extintc_id;
    uint64_t imsic_paddr;
    uint32_t imsic_size;
};
