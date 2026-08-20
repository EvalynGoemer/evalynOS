#pragma once
#include <arch/generic/paging/paging.h>
#include <mem/buddy.h>
#include <stdint.h>
#include <string.h>

static inline uint64_t pmm_alloc_page() {
    uint64_t paddr = buddy_allocate(MIN_ORDER, ALLOC_FLAG_NOFAIL);
    memset((void*)TO_HHDM(paddr), 0, PAGE_SIZE);
    return paddr;
}

static inline void pmm_free_page(uint64_t paddr) {
    buddy_free(paddr, MIN_ORDER);
}
