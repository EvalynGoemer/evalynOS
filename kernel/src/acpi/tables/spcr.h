#pragma once

#include <acpi/tables/sdt.h>
#include <stdint.h>

#define ACPI_SPCR_TABLE_SIGNATURE "SPCR"

typedef enum: uint8_t {
    SPCR_IFACE_16550 = 0,
    SPCR_IFACE_16540 = 1,
} SPCRiface;

struct [[gnu::packed]] SPCR {
    struct SDTHeader header;
    SPCRiface interface_type;
    uint8_t reserved[3];
    struct ACPIaddr address;
    // TODO; fill in the rest of these fields
    // https://learn.microsoft.com/en-us/windows-hardware/drivers/bringup/serial-port-console-redirection-table
};
