#include <stdint.h>
#include <utils/limine.h>
#include <utils/lib.h>
#include <stdio.h>

void memmap_print() {
    LOG_TAGGED("MEMORY", ANSI_BGREEN, "Got Limine Memory Map")
    printf(ANSI_BWHITE  "+-------+--------------------+--------------------+--------------------+------------------------+\n" ANSI_RESET);
    printf(ANSI_BWHITE  "| Index | Start              | End                | Size               | Type                   |\n" ANSI_RESET);
    printf(ANSI_BWHITE  "+-------+--------------------+--------------------+--------------------+------------------------+\n" ANSI_RESET);

    uint64_t usable_ram_pre_reclaim = 0;
    uint64_t usable_ram_post_reclaim = 0;

    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
        uint64_t base = TO_HHDM(memmap_request.response->entries[i]->base);
        uint64_t length = memmap_request.response->entries[i]->length;
        uint64_t type = memmap_request.response->entries[i]->type;

        const char *type_str;
        switch (type) {
            case LIMINE_MEMMAP_USABLE:
                type_str = "Usable";
                usable_ram_pre_reclaim += length;
                usable_ram_post_reclaim += length;
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                type_str = "Bootloader Reclaimable";
                usable_ram_post_reclaim += length;
                break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                type_str = "ACPI Reclaimable";
                usable_ram_post_reclaim += length;
                break;
            case LIMINE_MEMMAP_RESERVED:                type_str = "Reserved"; break;
            case LIMINE_MEMMAP_ACPI_NVS:                type_str = "ACPI NVS"; break;
            case LIMINE_MEMMAP_BAD_MEMORY:              type_str = "Bad Memory"; break;
            case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:  type_str = "Executable and Modules"; break;
            case LIMINE_MEMMAP_FRAMEBUFFER:             type_str = "Framebuffer"; break;
            case LIMINE_MEMMAP_RESERVED_MAPPED:         type_str = "Reserved Mapped"; break;
            default:                                    type_str = "Unknown"; break;
        }

        printf(ANSI_BWHITE "| " ANSI_BYELLOW "%05d" ANSI_BWHITE
        " | " ANSI_BGREEN "0x%016llx" ANSI_BWHITE
        " | " ANSI_BBLUE  "0x%016llx" ANSI_BWHITE
        " | " ANSI_BCYAN  "0x%016llx" ANSI_BWHITE
        " | " ANSI_BMAGENTA "%-22s" ANSI_BWHITE " |\n" ANSI_RESET,
        i, base, base + length, length, type_str);
    }
    printf(ANSI_BWHITE  "+-------+--------------------+--------------------+--------------------+------------------------+\n" ANSI_RESET);

    LOG_TAGGED("MEMORY", ANSI_BGREEN, "Usable memory before reclaim: " ANSI_BYELLOW "%llu MB", usable_ram_pre_reclaim / 1024 / 1024)
    LOG_TAGGED("MEMORY", ANSI_BGREEN, "Usable memory after reclaim: " ANSI_BYELLOW "%llu MB", usable_ram_post_reclaim / 1024 / 1024)
}
