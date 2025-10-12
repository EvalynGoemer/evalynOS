#include <stdbool.h>

#define LIMINE_API_REVISION 3
#include <limine.h>

#include "../kernel.h"
#include "../panic.h"
#include "../libc/string.h"

void* bump_alloc_base = (void *)0xDEADBEEF;
void kbump_alloc_init(unsigned long min_heap_size) {
    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {

        bool usable;

        switch (memmap_request.response->entries[i]->type) {
            case LIMINE_MEMMAP_USABLE:
                usable = true;
                break;
            case LIMINE_MEMMAP_RESERVED:
                usable = false;
                break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                usable = false;
                break;
            case LIMINE_MEMMAP_ACPI_NVS:
                usable = false;
                break;
            case LIMINE_MEMMAP_BAD_MEMORY:
                usable = false;
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                usable = false;
                break;
            case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
                usable = false;
                break;
            case LIMINE_MEMMAP_FRAMEBUFFER:
                usable = false;
                break;
            default:
                usable = false;
                break;
        }

        if (usable) {
            if (memmap_request.response->entries[i]->length >= min_heap_size) {
                bump_alloc_base = (void *)(uintptr_t)(memmap_request.response->entries[i]->base + hhdm_request.response->offset);
                memset(bump_alloc_base, 0, min_heap_size);
                return;
            }
        }
    }

    if(bump_alloc_base == (void *)0xDEADBEEF) {
        panic("OUT OF MEMORY", 0, 0, 0, 0);
    }
}

unsigned long bump_allocations;
void *kbump_alloc(unsigned long alloc_size) {
    unsigned long page_size = 4096;
    alloc_size = (alloc_size + page_size - 1) & ~(page_size - 1);

    void *ptr = (void *)(bump_alloc_base + bump_allocations);
    bump_allocations += alloc_size;
    return ptr;
}

void *kbump_alloc_phys(unsigned long alloc_size) {
    unsigned long page_size = 4096;
    alloc_size = (alloc_size + page_size - 1) & ~(page_size - 1);

    void *ptr = (void *)(bump_alloc_base + bump_allocations - hhdm_request.response->offset);
    bump_allocations += alloc_size;
    return ptr;
}
