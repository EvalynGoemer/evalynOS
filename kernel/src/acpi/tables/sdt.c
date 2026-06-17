#include <acpi/tables/sdt.h>
#include <stdint.h>
#include <utils/limine.h>
#include <utils/lib.h>
#include <stdio.h>
#include <string.h>

static void* acpi_find_sdt_rsdt(const char* signature) {
    struct RSDP* rsdp = (struct RSDP*)rsdp_request.response->address;
    struct RSDT* rsdt = TO_HHDM_PTR(rsdp->rsdtAddress);
    int entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

    for (int i = 0; i < entries; i++) {
        struct SDTHeader* header = TO_HHDM_PTR(rsdt->pointerSDTs[i]);
        if (!strncmp(header->signature, signature, 4))
            return (void*)header;
    }

    return nullptr;
}

static void* acpi_find_sdt_xsdt(const char* signature) {
    struct XSDP* xsdp = (struct XSDP*)rsdp_request.response->address;
    struct XSDT *xsdt = TO_HHDM_PTR(xsdp->xsdtAddress);
    int entries = (xsdt->header.length - sizeof(xsdt->header)) / 8;

    for (int i = 0; i < entries; i++) {
        struct SDTHeader *header = TO_HHDM_PTR(xsdt->pointerSDTs[i]);
        if (!strncmp(header->signature, signature, 4))
            return (void*)header;
    }

    return nullptr;
}

void* acpi_find_sdt(const char* signature) {
    if (!rsdp_request.response || !rsdp_request.response->address)
        return nullptr;

    struct RSDP* rsdp = (struct RSDP*)rsdp_request.response->address;
    if(rsdp->revision < 2)
        return acpi_find_sdt_rsdt(signature);
    return acpi_find_sdt_xsdt(signature);
}

static int acpi_table_count_rsdt() {
    struct RSDP *rsdp = (struct RSDP *)rsdp_request.response->address;
    struct RSDT *rsdt = TO_HHDM_PTR(rsdp->rsdtAddress);
    return (rsdt->header.length - sizeof(rsdt->header)) / 4;
}

static int acpi_table_count_xsdt() {
    struct XSDP *xsdp = (struct XSDP *)rsdp_request.response->address;
    struct XSDT *xsdt = TO_HHDM_PTR(xsdp->xsdtAddress);
    return (xsdt->header.length - sizeof(xsdt->header)) / 8;
}

int acpi_table_count() {
    struct RSDP *rsdp = (struct RSDP *)rsdp_request.response->address;
    if (rsdp->revision < 2)
        return acpi_table_count_rsdt();
    return acpi_table_count_xsdt();
}

static void* acpi_get_sdt_rsdt(int n) {
    struct RSDP *rsdp = (struct RSDP *)rsdp_request.response->address;
    struct RSDT *rsdt = TO_HHDM_PTR(rsdp->rsdtAddress);
    int entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;
    if (n < 0 || n >= entries)
        return nullptr;
    return TO_HHDM_PTR(rsdt->pointerSDTs[n]);
}

static void* acpi_get_sdt_xsdt(int n) {
    struct XSDP *xsdp = (struct XSDP *)rsdp_request.response->address;
    struct XSDT *xsdt = TO_HHDM_PTR(xsdp->xsdtAddress);
    int entries = (xsdt->header.length - sizeof(xsdt->header)) / 8;
    if (n < 0 || n >= entries)
        return nullptr;
    return TO_HHDM_PTR(xsdt->pointerSDTs[n]);
}

void* acpi_get_sdt(int n) {
    struct RSDP *rsdp = (struct RSDP *)rsdp_request.response->address;
    if (rsdp->revision < 2)
        return acpi_get_sdt_rsdt(n);
    return acpi_get_sdt_xsdt(n);
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

    uint64_t checksumV1 = 0;
    uint8_t *bytes = (uint8_t *)rsdp;
    for (size_t i = 0; i < sizeof(struct RSDP); i++)
        checksumV1 += bytes[i];

    if ((uint8_t)checksumV1 != 0)
        LOG_TAGGED("ACPI", ANSI_BMAGENTA, "RSDP ChecksumV1 is bad")

    if (rsdp->revision < 2)
        return true;

    struct XSDP* xsdp = (struct XSDP*)rsdp_request.response->address;

    uint64_t checksumV2 = 0;
    for (size_t i = 0; i < xsdp->length; i++)
        checksumV2 += bytes[i];

    if ((uint8_t)checksumV2 != 0)
        LOG_TAGGED("ACPI", ANSI_BMAGENTA, "RSDP ChecksumV2 is bad")

    return true;
}
