#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <utils/lib.h>
#include <utils/dstruct/stack.h>
#include <mem/buddy.h>

[[gnu::weak]] void arch_reclaim_ap_memory() {}

typedef struct {
    stack_node_t node;
    uint64_t base;
    uint64_t length;
} boot_range_t;

void reclaim_boot_memory() {
    arch_reclaim_ap_memory();

    stack_t ranges = {0};
    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry* e = memmap_request.response->entries[i];
        if (e->type != LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE && e->type != LIMINE_MEMMAP_ACPI_RECLAIMABLE)
            continue;
        if (e->base + e->length <= 0x100000)
            continue;

        boot_range_t* range = malloc(sizeof(boot_range_t));
        range->base = e->base;
        range->length = e->length;
        stack_push(&ranges, &range->node);
    }

    stack_node_t* node;
    while ((node = stack_pop(&ranges))) {
        boot_range_t* range = CONTAINER_OF(node, boot_range_t, node);
        buddy_add_range(range->base, range->length);
        free(range);
    }

    LOG_TAGGED("MEMORY", ANSI_BGREEN, "Boot memory reclaimed");
}
