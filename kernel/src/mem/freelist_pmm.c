#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <arch/generic/panic.h>
#include <utils/align.h>
#include <utils/limine.h>
#include <utils/locks/ticketlock.h>

#include <mem/freelist_pmm.h>
#include <mem/pmm.h>

ticketlock_t freelist_pmm_lock = {0};
freelist_pmm_node_t* freelist_pmm_head = NULL;
uint64_t freelist_pmm_fill_entry = 0;
uint64_t freelist_pmm_fill_offset = 0;

// only call when the freelist_pmm_lock is held or when no threading/smp is online
void freelist_pmm_fill(uint64_t pages) {
    uint64_t pages_added = 0;

    while (freelist_pmm_fill_entry < memmap_request.response->entry_count && pages_added < pages) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[freelist_pmm_fill_entry];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            freelist_pmm_fill_entry++;
            freelist_pmm_fill_offset = 0;
            continue;
        }

        uint64_t aligned_start = ALIGN_UP(entry->base, PAGE_SIZE);
        uint64_t aligned_end   = ALIGN_DOWN(entry->base + entry->length, PAGE_SIZE);

        // dont allocate memory in the first 1MB
        if (aligned_start < 0x100000ULL)
            aligned_start = 0x100000ULL;

        if (aligned_end <= aligned_start) {
            freelist_pmm_fill_entry++;
            freelist_pmm_fill_offset = 0;
            continue;
        }

        if (freelist_pmm_fill_offset == 0)
            freelist_pmm_fill_offset = aligned_start - entry->base;

        while (entry->base + freelist_pmm_fill_offset + PAGE_SIZE <= aligned_end && pages_added < pages) {
            uint64_t page_addr = entry->base + freelist_pmm_fill_offset;
            freelist_pmm_node_t *node = (freelist_pmm_node_t*)(page_addr + hhdm_request.response->offset);
            node->next = freelist_pmm_head;
            freelist_pmm_head = node;
            pages_added++;
            freelist_pmm_fill_offset += PAGE_SIZE;
        }

        if (entry->base + freelist_pmm_fill_offset + PAGE_SIZE > aligned_end) {
            freelist_pmm_fill_entry++;
            freelist_pmm_fill_offset = 0;
        }
    }
}

uint64_t freelist_pmm_allocate_page() {
    int lock1r = ticketlock_lock(&freelist_pmm_lock);

    if (freelist_pmm_head == NULL) freelist_pmm_fill(512);
    if (freelist_pmm_head == NULL) panic("PMM: out of memory");

    freelist_pmm_node_t* node = freelist_pmm_head;
    freelist_pmm_head = node->next;
    uint64_t phys = (uint64_t)((uintptr_t)node - hhdm_request.response->offset);

    ticketlock_unlock(&freelist_pmm_lock, lock1r);

    memset(node, 0, PAGE_SIZE);
    return phys;
}

void freelist_pmm_free_page(uint64_t phys) {
    int lock1r = ticketlock_lock(&freelist_pmm_lock);

    freelist_pmm_node_t* node = (freelist_pmm_node_t*)(phys + hhdm_request.response->offset);
    node->next = freelist_pmm_head;
    freelist_pmm_head = node;

    ticketlock_unlock(&freelist_pmm_lock, lock1r);
}

void freelist_pmm_init() {
    freelist_pmm_fill(512);
    pmm_alloc_page = freelist_pmm_allocate_page;
    pmm_free_page = freelist_pmm_free_page;
    LOG_TAGGED_OK("MEMORY", ANSI_BGREEN, "Freelist PMM Init")
}
