#define LIMINE_API_REVISION 3
#include <limine.h>

#include "../renderer/fb_renderer.h"
#include "../kernel.h"

char* int_to_hex(unsigned long num) {
    static const char hex[] = "0123456789ABCDEF";
    static char out[17];

    int i = 16;
    out[i] = '\0';

    if (num == 0) {
        out[--i] = '0';
        return &out[i];
    }

    while (num && i > 0) {
        out[--i] = hex[num & 0xF];
        num >>= 4;
    }

    return &out[i];
}

void printMemoryMap() {
    const int column_spacing = 150;
    const int row_height = 10;
    const uint64_t entry_count = memmap_request.response->entry_count;

    int start_y = FB_HEIGHT - (row_height * (entry_count + 1)) - 10;
    int start_x = FB_WIDTH - ((column_spacing * 2) + 176) - 10;

    printString("Memory Map:", start_x, start_y - row_height);
    printString("VIRTUAL ADDRESS", start_x, start_y);
    printString("LENGTH IN BYTES", start_x + column_spacing, start_y);
    printString("MEMORY TYPE", start_x + (2 * column_spacing), start_y);

    for (uint64_t i = 0; i < entry_count; i++) {
        int y = start_y + (i + 1) * row_height;

        printString(int_to_hex(memmap_request.response->entries[i]->base + hhdm_request.response->offset),
                    start_x, y);

        printString(int_to_hex(memmap_request.response->entries[i]->length),
                    start_x + column_spacing, y);

        switch (memmap_request.response->entries[i]->type) {
            case LIMINE_MEMMAP_USABLE:
                printString("Usable", start_x + (2 * column_spacing), y);
                break;
            case LIMINE_MEMMAP_RESERVED:
                printString("Reserved", start_x + (2 * column_spacing), y);
                break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                printString("ACPI Reclaimable", start_x + (2 * column_spacing), y);
                break;
            case LIMINE_MEMMAP_ACPI_NVS:
                printString("ACPI NVS", start_x + (2 * column_spacing), y);
                break;
            case LIMINE_MEMMAP_BAD_MEMORY:
                printString("Bad Memory", start_x + (2 * column_spacing), y);
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                printString("Bootloader Reclaimable", start_x + (2 * column_spacing), y);
                break;
            case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
                printString("Executable and Modules", start_x + (2 * column_spacing), y);
                break;
            case LIMINE_MEMMAP_FRAMEBUFFER:
                printString("Framebuffer", start_x + (2 * column_spacing), y);
                break;
            default:
                printString("Unknown", start_x + (2 * column_spacing), y);
                break;
        }
    }
}
