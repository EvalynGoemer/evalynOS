#pragma once

#include <stdint.h>

struct [[gnu::packed]] RSDP {
    char     signature[8];
    uint8_t  checksum;
    char     oemID[6];
    uint8_t  revision;
    uint32_t rsdtAddress;
};

struct [[gnu::packed]] XSDP {
    char     signature[8];
    uint8_t  checksum;
    char     oemID[6];
    uint8_t  revision;
    uint32_t rsdtAddress;
    uint32_t length;
    uint64_t xsdtAddress;
    uint8_t  extendedChecksum;
    uint8_t  reserved[3];
};

struct [[gnu::packed]] SDTHeader {
    char     signature[4];
    uint32_t length;
    uint8_t  revision;
    uint8_t  checksum;
    char     oemID[6];
    char     oemTableID[8];
    uint32_t oemRevision;
    uint32_t creatorID;
    uint32_t creatorRevision;
};

struct [[gnu::packed]] RSDT {
    struct SDTHeader header;
    [[gnu::aligned(4)]] uint32_t pointerSDTs[];
};

struct [[gnu::packed]] XSDT {
    struct SDTHeader header;
    [[gnu::aligned(4)]] uint64_t pointerSDTs[];
};

typedef enum: uint8_t {
    ACPI_ADDRESS_TYPE_MMIO                           = 0x00,
    ACPI_ADDRESS_TYPE_PORT_IO                        = 0x01,
    ACPI_ADDRESS_TYPE_PCI_CONFIGURATION_SPACE        = 0x02,
    ACPI_ADDRESS_TYPE_EMBEDDED_CONTROLLER            = 0x03,
    ACPI_ADDRESS_TYPE_SYSTEM_MANAGEMENT_BUS          = 0x04,
    ACPI_ADDRESS_TYPE_SYSTEM_CMOS                    = 0x05,
    ACPI_ADDRESS_TYPE_PCI_DEVICE_BAR_TARGET          = 0x06,
    ACPI_ADDRESS_TYPE_IPMI                           = 0x07,
    ACPI_ADDRESS_TYPE_GENERAL_PURPOSE_IO             = 0x08,
    ACPI_ADDRESS_TYPE_GENERIC_SERIAL_BUS             = 0x09,
    ACPI_ADDRESS_TYPE_PLATFORM_COMMUNICATION_CHANNEL = 0x0A,
} ACPIAddrType;

struct [[gnu::packed]] ACPIaddr {
    ACPIAddrType address_space_id;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
};

extern bool acpi_verify_rsdp();

extern void* acpi_find_sdt(const char* signature);
extern void* acpi_get_sdt(int n);
extern bool acpi_check_sdt_checksum(struct SDTHeader* header);
extern int acpi_table_count();
