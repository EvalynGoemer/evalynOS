#include "arch/generic/paging/paging.h"
#include "mem/pfndb.h"
#include "mem/pmm.h"
#include "mem/vmem.h"
#include "utils/dstruct/rbtree.h"
#include <stdint.h>
#include <mem/address_space.h>
#include <string.h>
#include <stdlib.h>

address_space_t* new_address_space() {
    address_space_t* addrspace = malloc(sizeof(address_space_t));

    // leave some space at the top and bottom reserved
    addrspace->valloc = new_vmem_allocator(PAGE_SIZE, VADDR_LOWER_HALF_TOP - PAGE_SIZE, PAGE_SIZE);

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

void free_address_space(address_space_t* addrspace) {
    RBTREE_FOR_EACH(addrspace->valloc->segments_tree, node) {
        vmem_segment_t* seg = CONTAINER_OF(node, vmem_segment_t, segment_tree_node);
        uint64_t pages = seg->size >> PAGE_SIZE_SHIFT;

        for (uint64_t i = 0; i < pages; i++) {
            uint64_t addr = seg->base + (i << PAGE_SIZE_SHIFT);

            // TODO: check for paddrs that are from MMIO or other things to skip free
            // TODO: resolve the order from the pfndb to support large pages better here

            uint64_t paddr = paging_get_paddr(addrspace->pagetable, addr, nullptr);
            if (paddr == 0) continue;
            pmm_free_page(paddr);
        }

        paging_unmap_range(addrspace->pagetable, seg->base, seg->size);
    }

    pmm_free_page(addrspace->pagetable);
    free_vmem_allocator(addrspace->valloc);
    free(addrspace);
}
