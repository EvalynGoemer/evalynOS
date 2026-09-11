#include "arch/generic/paging/paging.h"
#include "mem/pfndb.h"
#include "mem/pmm.h"
#include "mem/vmem.h"
#include <mem/address_space.h>
#include <string.h>
#include <stdlib.h>

address_space_t* new_address_space() {
    address_space_t* addrspace = malloc(sizeof(address_space_t));
    memset(addrspace, 0, sizeof(address_space_t));

    // leave some space at the top and bottom reserved
    vmem_allocator_init(&addrspace->valloc, PAGE_SIZE, VADDR_LOWER_HALF_TOP - PAGE_SIZE, PAGE_SIZE);

    uint64_t newpt = pmm_alloc_page();
    page_t* entry = pfndb_get_entry(newpt);
    entry->type = PFNDB_PAGE_PTABLE;
    entry->ref_count = 0;

#ifndef ARCH_SEPARATE_PAGING_ROOTS
    void* dst = (void*)TO_HHDM(newpt + (PAGE_SIZE >> 1));
    void* src = (void*)TO_HHDM(kernel_page_table + (PAGE_SIZE >> 1));
    memcpy(dst, src, PAGE_SIZE >> 1);
#endif

    addrspace->pagetable = newpt;

    return addrspace;
}
