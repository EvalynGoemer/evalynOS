#include <mem/pfndb.h>
#include <mem/pmm.h>
#include <utils/lib.h>
#include <arch/generic/paging/paging.h>
#include <assert.h>
#include <stdint.h>
#include <mem/memmap.h>
#include <string.h>
#include <stdio.h>

page_t*  pfndb;
uint64_t pfndb_size;

void pfndb_init() {
    uint64_t last_paddr = memmap_last_paddr();

    pfndb_size = sizeof(page_t) * (last_paddr >> PAGE_SIZE_SHIFT);
    pfndb_size = ALIGN_UP(pfndb_size, PAGE_SIZE);
    uint64_t vbase = executable_address_request.response->virtual_base;

    #ifndef ARCH_UNPAGED_HHDM
    uint64_t hhdm_base = hhdm_request.response->offset;
    uint64_t hhdm_top = last_paddr + hhdm_base;

    assert(hhdm_base >= VADDR_HIGHER_HALF_BASE && vbase > hhdm_top);

    uint64_t candidateA = hhdm_base - VADDR_HIGHER_HALF_BASE;
    uint64_t candidateB = vbase - hhdm_top;

    if (candidateA > candidateB) {
        assert(candidateA >= pfndb_size);
        pfndb = (void*)VADDR_HIGHER_HALF_BASE;
    } else {
        assert(candidateB >= pfndb_size);
        pfndb = (void*)hhdm_top;
    }
    #else
    pfndb = (void*)VADDR_HIGHER_HALF_BASE;
    assert(VADDR_HIGHER_HALF_BASE + pfndb_size <= vbase);
    #endif

    uint64_t last_mapped = 0;
    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap_request.response->entries[i];
        if (entry->type == LIMINE_MEMMAP_RESERVED)   continue;
        if (entry->type == LIMINE_MEMMAP_BAD_MEMORY) continue;
        uint64_t start_frame = entry->base >> PAGE_SIZE_SHIFT;
        uint64_t end_frame   = (entry->base + entry->length + PAGE_SIZE - 1) >> PAGE_SIZE_SHIFT;
        uint64_t meta_first = (uintptr_t)pfndb + ALIGN_DOWN(start_frame * sizeof(page_t), PAGE_SIZE);
        uint64_t meta_last  = (uintptr_t)pfndb + ALIGN_DOWN(end_frame * sizeof(page_t) - 1, PAGE_SIZE);

        for (uint64_t vaddr = meta_first; vaddr <= meta_last; vaddr += PAGE_SIZE) {
            if (entry->length == 0) continue;
            if (vaddr < last_mapped) continue;
            uint64_t paddr = pmm_alloc_page();
            paging_map_page(kernel_page_table, vaddr, paddr, PAGE_KRW);
            memset((void*)vaddr, 0, PAGE_SIZE);
            last_mapped = vaddr + PAGE_SIZE;
        }
    }

    LOG_TAGGED_OK("MEMORY", ANSI_BGREEN, "PFNdb Init");
}
