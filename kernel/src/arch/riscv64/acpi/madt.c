#include "arch/riscv64/acpi/madt.h"
#include "arch/riscv64/acpi/rhct.h"
#include "utils/limine.h"
#include <arch/riscv64/cpu/isa.h>
#include <acpi/tables/madt.h>
#include <arch/generic/paging/paging.h>
#include <arch/riscv64/cpu/ap.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

bool acpi_parse_madt() {
    uint8_t* madt_base = acpi_find_sdt(MADT_SDT_SIGNATURE);

    if (!madt_base) {
        LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "MADT table is not present")
        return false;
    }

    bool bsp_valid = false;
    uint32_t harts = 0;
    uint32_t total_entries = 0;
    uint32_t unknown_entries = 0;

    uint64_t bsp_hart = riscv_bsp_hartid_request.response->bsp_hartid;

    struct MADTEntryHeader* entry;
    MADT_FOR_EACH_ENTRY(madt_base, entry) {
        total_entries++;
        switch (entry->type) {
            case MADT_TYPE_RINTC: {
                struct MADT_rintc* rintc = (void*)entry;
                if (!(rintc->flags & 1)) break;

                rhct_hart_info_t hart_info = rhct_get_hart_info(rintc->acpi_uid);

                if (!hart_info.isa_string) break;
                if (!isa_has_baseline(hart_info.isa_string, ISA_BASELINE_RV64GC)) break;
                if (MMU_CONFIG_TOP_LEVEL(mmu_config) > hart_info.mmu_type + 3) break;

                if (rintc->hart_id == bsp_hart) {
                    const char* end = strchr(hart_info.isa_string, '_');
                    int base_len = end ? (int)(end - hart_info.isa_string) : (int)strlen(hart_info.isa_string);
                    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "BSP Baseline ISA: %.*s", base_len, hart_info.isa_string);
                    bsp_valid = true;
                }

                detected_hart_t* detected_hart = malloc(sizeof(detected_hart_t));
                memset(detected_hart, 0, sizeof(detected_hart_t));
                detected_hart->hart_id = rintc->hart_id;
                llist_push(&detected_harts, &detected_hart->node);

                harts++;
                break;
            }
            case MADT_TYPE_IMSIC:
            case MADT_TYPE_APLIC:
            case MADT_TYPE_PLIC : break;
            default: {
                unknown_entries++;
                break;
            }
        }
    }

    if (harts == 0) {
        LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Could not find any valid harts");
        return false;
    }

    if (bsp_valid == false) {
        LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Could not find the BSP in the MADT");
        return false;
    }

    detected_cpus = harts;

    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d known entries in the MADT", total_entries - unknown_entries);
    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d unknown entries in the MADT", unknown_entries);
    LOG_TAGGED("ACPI/MADT", ANSI_BMAGENTA, "Found %d HART(s)", harts);
    return true;
}
