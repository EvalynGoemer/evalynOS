#pragma once
#include <arch/generic/paging/paging.h>
#include <stdint.h>

// PFNDB_PAGE_INVALID represents if a page is either
// - not a valid physical frame
// - was used for inital page tables and pfndb and shouldnt be touched
// - has yet to be toched by an allocator

typedef enum pagetype : uint8_t {
    PFNDB_PAGE_INVALID   = 0,
    PFNDB_PAGE_FREE      = 1,
    PFNDB_PAGE_ALLOCATED = 2,
} pagetype_t;

typedef struct page page_t;
typedef struct page {
    pagetype_t type;
    uint8_t    order;
    uint16_t   flags;
    uint32_t   ref_count;
} page_t;

extern page_t*  pfndb;
extern uint64_t pfndb_size;
extern void pfndb_init();

static inline page_t* pfndb_get_entry(uint64_t paddr) {
    return &pfndb[paddr >> PAGE_SIZE_SHIFT];
}

static inline uint64_t pfndb_get_paddr(page_t* entry) {
    return (uint64_t)(entry - pfndb) << PAGE_SIZE_SHIFT;
}
