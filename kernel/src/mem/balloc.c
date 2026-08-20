#include <stdint.h>
#include <string.h>
#include <utils/lib.h>
#include <utils/limine.h>
#include <arch/intrin/paging.h>

static uint64_t balloc_entry = 0;
static uint64_t balloc_offset = 0;
uint64_t balloc_last_addr = 0;

uint64_t balloc_alloc_page() {
    while (balloc_entry < memmap_request.response->entry_count) {
        struct limine_memmap_entry* entry = memmap_request.response->entries[balloc_entry];
        if (entry->type != LIMINE_MEMMAP_USABLE) {
            balloc_entry++;
            balloc_offset = 0;
            continue;
        }

        uint64_t aligned_start = MAX(ALIGN_UP(entry->base, PAGE_SIZE), 0x100000ull);
        uint64_t aligned_end   = ALIGN_DOWN(entry->base + entry->length, PAGE_SIZE);

        if (balloc_offset == 0)
            balloc_offset = aligned_start - entry->base;

        if (entry->base + balloc_offset + PAGE_SIZE <= aligned_end) {
            uint64_t paddr = entry->base + balloc_offset;
            balloc_offset += PAGE_SIZE;
            balloc_last_addr = paddr;
            memset((void*)TO_HHDM(paddr), 0, PAGE_SIZE);
            return paddr;
        }

        balloc_entry++;
        balloc_offset = 0;
    }

    panic("Not enough memory to boot system");
    return 0;
}
