#include "arch/loongarch64/acpi/madt.h"
#include "arch/loongarch64/cpu/ap.h"
#include "arch/loongarch64/intrin/csr.h"
#include <arch/generic/cpu/ap.h>
#include <acpi/tables/madt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void acpi_parse_madt() {
    uint8_t* madt_base = acpi_find_sdt(MADT_SDT_SIGNATURE);

    if (!madt_base) {
        LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "MADT table is not present")
        return;
    }

    uint32_t madt_entries = 0;
    uint32_t unknown_madt_entries = 0;

    struct MADTEntryHeader* entry;
    MADT_FOR_EACH_ENTRY(madt_base, entry) {
        madt_entries++;
        switch (entry->type) {
            case MADT_TYPE_CORE_PIC: {
                struct MADT_corepic* corepic = (void*)entry;
                if (corepic->core_id == 0xFFFFFFFF) break;
                if (!(corepic->flags & 1)) break;

                detected_core_t* core = kmalloc(sizeof(detected_core_t));
                memset(core, 0, sizeof(detected_core_t));
                core->bsp = corepic->core_id == csrrd(CSR_CPUID) ? true : false;
                core->core_id = corepic->core_id;
                total_cpus++;
                llist_push(&detected_cores, &core->node);
                break;
            }
            case MADT_TYPE_LIO_PIC:
            case MADT_TYPE_HT_PIC:
            case MADT_TYPE_EIO_PIC:
            case MADT_TYPE_MSI_PIC:
            case MADT_TYPE_BIO_PIC:
            case MADT_TYPE_LPC_PIC: break;
            default: {
                unknown_madt_entries++;
                break;
            }
        }
    }

    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d known entries in the MADT", madt_entries - unknown_madt_entries);
    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d unknown entries in the MADT", unknown_madt_entries);
}
