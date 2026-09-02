#include <acpi/tables/sdt.h>
#include <stdint.h>
#include <utils/limine.h>
#include <utils/lib.h>
#include <stdio.h>
#include <string.h>

static inline int acpi_rsdt_entry_size() {
    struct XSDP* xsdp = (struct XSDP*)rsdp_request.response->address;
    return (xsdp->revision < 2) ? sizeof(uint32_t) : sizeof(uint64_t);
}

static inline void* acpi_get_rsdt() {
    struct XSDP* xsdp = (struct XSDP*)rsdp_request.response->address;
    return (xsdp->revision < 2) ? TO_HHDM_PTR(xsdp->rsdtAddress) : TO_HHDM_PTR(xsdp->xsdtAddress);
}

int acpi_table_count() {
    int sz = acpi_rsdt_entry_size();
    struct RSDT* rsdt = acpi_get_rsdt();
    return (rsdt->header.length - sizeof(rsdt->header)) / sz;
}

void* acpi_get_sdt(int n) {
    struct SDTHeader* sdt = acpi_get_rsdt();
    int sz = acpi_rsdt_entry_size();
    uint64_t phys = (sz == sizeof(uint32_t))
    ? ((struct RSDT*)sdt)->pointerSDTs[n]
    : ((struct XSDT*)sdt)->pointerSDTs[n];
    return TO_HHDM_PTR(phys);
}

void* acpi_find_sdt(const char* signature) {
    if (!rsdp_request.response || !rsdp_request.response->address)
        return nullptr;
    int entries = acpi_table_count();
    for (int i = 0; i < entries; i++) {
        struct SDTHeader* header = acpi_get_sdt(i);
        if (!strncmp(header->signature, signature, 4))
            return header;
    }
    return nullptr;
}

bool acpi_check_sdt_checksum(struct SDTHeader* header) {
    uint8_t checksum = 0;
    uint8_t *bytes = (uint8_t*)header;
    for (uint32_t i = 0; i < header->length; i++)
        checksum += bytes[i];
    return checksum == 0;
}

bool acpi_verify_rsdp() {
    if (!rsdp_request.response || !rsdp_request.response->address) {
        LOG_TAGGED("ACPI", ANSI_BMAGENTA, "ACPI is not present")
        return false;
    }

    struct RSDP* rsdp = (struct RSDP*)rsdp_request.response->address;
    LOG_TAGGED("ACPI", ANSI_BMAGENTA, "RSDP revision %d found at 0x%016llx", rsdp->revision, (uint64_t)rsdp)

    uint8_t checksumV1 = 0;
    uint8_t* bytes = (uint8_t*)rsdp;
    for (size_t i = 0; i < sizeof(struct RSDP); i++)
        checksumV1 += bytes[i];

    if (checksumV1 != 0)
        LOG_TAGGED("ACPI", ANSI_BMAGENTA, "RSDP ChecksumV1 is bad")

    if (rsdp->revision < 2)
        return true;

    struct XSDP* xsdp = rsdp_request.response->address;

    uint8_t checksumV2 = 0;
    for (size_t i = 0; i < xsdp->length; i++)
        checksumV2 += bytes[i];

    if (checksumV2 != 0)
        LOG_TAGGED("ACPI", ANSI_BMAGENTA, "RSDP ChecksumV2 is bad")

    return true;
}
