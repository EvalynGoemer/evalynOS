#define LIMINE_API_REVISION 3
#include <limine.h>

#include "../renderer/fb_renderer.h"
#include "../kernel.h"
#include "../libc/stdio.h"

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
    uint64_t entry_count = memmap_request.response->entry_count;

    printf("Memory Map:\n");
    printf("VIRTUAL ADDRESS | LENGTH IN BYTES | MEMORY TYPE\n");

    for (uint64_t i = 0; i < entry_count; i++) {
        uint64_t base = memmap_request.response->entries[i]->base + hhdm_request.response->offset;
        uint64_t length = memmap_request.response->entries[i]->length;
        const char *type_str;

        switch (memmap_request.response->entries[i]->type) {
            case LIMINE_MEMMAP_USABLE:                  type_str = "Usable"; break;
            case LIMINE_MEMMAP_RESERVED:                type_str = "Reserved"; break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:        type_str = "ACPI Reclaimable"; break;
            case LIMINE_MEMMAP_ACPI_NVS:                type_str = "ACPI NVS"; break;
            case LIMINE_MEMMAP_BAD_MEMORY:              type_str = "Bad Memory"; break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:  type_str = "Bootloader Reclaimable"; break;
            case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:  type_str = "Executable and Modules"; break;
            case LIMINE_MEMMAP_FRAMEBUFFER:             type_str = "Framebuffer"; break;
            default:                                    type_str = "Unknown"; break;
        }

        printf("0x%llx  0x%llx  %s\n", (unsigned long long)base, (unsigned long long)length, type_str);
    }
}
