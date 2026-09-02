#include <stdint.h>
#include <arch/generic/paging/helpers.h>
#include <utils/lib.h>

uint64_t paging_split_leaf(uint64_t pte_phys, int level) {
    uint64_t* pslot = TO_HHDM_PTR(pte_phys);
    uint64_t entry = *pslot;
    uint64_t base = ARCH_DECODE_LARGE_PTE(entry);
    uint64_t prot = mmu_flags_to_prot(entry, level);

    uint64_t child = pmm_alloc_page();
    page_t* pg = pfndb_get_entry(child);
    pg->type = PFNDB_PAGE_PTABLE;
    pg->ref_count = 0;

    uint64_t flags = prot_to_mmu_flags(prot);
    if (level > 2) flags = arch_large_page_fixup(flags);

    uint64_t* vchild = TO_HHDM_PTR(child);
    uint64_t entry_size = PAGING_ENTRY_SIZE(level - 1);
    for (int i = 0; i < PAGING_TABLE_ENTRIES; i++)
        vchild[i] = ARCH_ENCODE_PTE(base + i * entry_size, flags);

    update_refcnt(child, PAGING_TABLE_ENTRIES);
    *pslot = ARCH_ENCODE_PTE(child, ARCH_INTERMEDIATE_MMU_FLAGS(prot));
    return child;
}
