#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <acpi/tables/sdt.h>
#include <acpi/tables/fadt.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MAX_FADT_REVISION 4
#define FADT_FLAG_HW_REDUCED_ACPI (1 << 20)

const uint16_t fadt_version_sizes[MAX_FADT_REVISION + 1] = { 0, 116, 132, 244, 244 };
_Static_assert(fadt_version_sizes[MAX_FADT_REVISION] <= sizeof(struct FADT), "FADT struct too small for max revision size");
struct FADT clean_fadt = {0};

void acpi_parse_fadt() {
    struct FADT* firmware_fadt = (struct FADT*)acpi_find_sdt("FACP");
    if (!firmware_fadt) {
        LOG_TAGGED("ACPI/FADT", ANSI_BMAGENTA, "FADT table is not present");
        return;
    }

    uint8_t reported_rev = firmware_fadt->header.revision;

    if (reported_rev > MAX_FADT_REVISION) {
        LOG_TAGGED("ACPI/FADT", ANSI_BMAGENTA, "FADT revision %d is newer than max supported, using as revision %d", reported_rev, MAX_FADT_REVISION);
    }

    uint8_t real_rev;
    for (real_rev = MAX_FADT_REVISION; real_rev > 0; real_rev--) {
        if (firmware_fadt->header.length >= fadt_version_sizes[real_rev])
            break;
    }

    if (real_rev == 0) {
        LOG_TAGGED_WARN("ACPI/FADT", ANSI_BMAGENTA, "Firmware Bug: FADT length %u too small for any known revision, ignoring table", firmware_fadt->header.length);
        return;
    }

    uint8_t clamped_rev = MIN(reported_rev, MAX_FADT_REVISION);
    if (fadt_version_sizes[clamped_rev] != fadt_version_sizes[real_rev]) {
        LOG_TAGGED_WARN("ACPI/FADT", ANSI_BMAGENTA, "Firmware Bug: FADT length %u doesn't match revision %d, assuming revision %d",
                   firmware_fadt->header.length, reported_rev, real_rev);
    }

    memcpy(&clean_fadt, firmware_fadt, fadt_version_sizes[real_rev]);
    clean_fadt.header.revision = real_rev;

    if (acpi_is_hw_reduced()) {
        LOG_TAGGED("ACPI/FADT", ANSI_BMAGENTA, "System has hardware reduced ACPI");
    } else {
        LOG_TAGGED("ACPI/FADT", ANSI_BMAGENTA, "System has full ACPI hardware");
    }
}
