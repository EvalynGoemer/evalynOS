#include <acpi/tables/sdt.h>
#include <arch/riscv64/acpi/rhct.h>
#include <arch/riscv64/timer/timer.h>
#include <stdint.h>
#include <stdio.h>

bool acpi_parse_rhct() {
    struct RHCT* rhct = acpi_find_sdt(RHCT_SDT_SIGNATURE);
    if (!rhct) {
        LOG_TAGGED("ACPI/RHCT", ANSI_BMAGENTA, "RHCT table is not present");
        return false;
    }

    timebase_frequency = rhct->timebase_freq;

    if (timebase_frequency >= 1000000) {
        LOG_TAGGED("ACPI/RHCT", ANSI_BMAGENTA, "Timebase: %u MHz", timebase_frequency / 1000000);
    } else {
        LOG_TAGGED("ACPI/RHCT", ANSI_BMAGENTA, "Timebase: %u Hz", timebase_frequency);
    }

    return true;
}

struct RHCTEntryHeader* rhct_first_entry(struct RHCT* rhct) {
    uint8_t* base = (uint8_t*)rhct;
    uint8_t* end  = base + rhct->header.length;
    uint8_t* ptr  = base + sizeof(struct RHCT);

    if (ptr + sizeof(struct RHCTEntryHeader) > end)
        return NULL;

    struct RHCTEntryHeader* entry = (void*)ptr;

    if (entry->length < sizeof(struct RHCTEntryHeader))
        return NULL;

    if (ptr + entry->length > end)
        return NULL;

    return entry;
}

struct RHCTEntryHeader* rhct_next_entry(struct RHCT* rhct, struct RHCTEntryHeader* entry) {
    if (entry == NULL)
        return rhct_first_entry(rhct);

    uint8_t* base = (uint8_t*)rhct;
    uint8_t* end  = base + rhct->header.length;
    uint8_t* ptr  = (uint8_t*)entry + entry->length;

    if (ptr + sizeof(struct RHCTEntryHeader) > end)
        return NULL;

    struct RHCTEntryHeader* next = (struct RHCTEntryHeader*)ptr;

    if (next->length < sizeof(struct RHCTEntryHeader))
        return NULL;

    if (ptr + next->length > end)
        return NULL;

    return next;
}

rhct_hart_info_t rhct_get_hart_info(uint32_t acpi_uid) {
    uint8_t* rhct_base = acpi_find_sdt(RHCT_SDT_SIGNATURE);

    rhct_hart_info_t info;
    bool found_isa_string = false;
    bool found_mmu = false;

    struct RHCTEntryHeader* entry;
    RHCT_FOR_EACH_ENTRY(rhct_base, entry) {
        struct RHCT_hart* hart = (void*)entry;
        if (entry->type != RHCT_TYPE_HART) continue;
        if (hart->acpi_uid != acpi_uid) continue;

        for (int i = 0; i < hart->noffsets; i++) {
            uint32_t offset = hart->offsets[i];
            struct RHCTEntryHeader* hentry = (void*)(rhct_base + offset);
            switch (hentry->type) {
                case RHCT_TYPE_ISA_STRING: {
                    found_isa_string = true;
                    struct RHCT_isa_string* isa = (void*)hentry;
                    info.isa_string = isa->string;
                    break;
                }
                case RHCT_TYPE_MMU: {
                    found_mmu = true;
                    struct RHCT_mmu* mmu = (void*)hentry;
                    info.mmu_type = mmu->mmu_type;
                    break;
                }
                default: break;
            }
        }

        break;
    }

    if (!found_isa_string || !found_mmu)
        return (rhct_hart_info_t){0};

    return info;
}





