#include <arch/generic/paging/helpers.h>

static bool paging_walk(uint64_t page_table, uint64_t vaddr, uint64_t* leaf_pte, int* leaf_level, uint64_t* pte_phys) {
    int top = MMU_CONFIG_TOP_LEVEL(mmu_config);
    uint64_t table = page_table;
    for (int level = top; level >= 1; level--) {
        uint64_t pte = paging_get_pte(table, vaddr, level);
        uint64_t entry = *(uint64_t*)TO_HHDM_PTR(pte);
        if (!ARCH_PTE_PRESENT(entry))
            return false;
        if (ARCH_PTE_IS_LEAF(entry, level)) {
            *leaf_pte = entry;
            *leaf_level = level;
            *pte_phys = pte;
            return true;
        }
        table = ARCH_DECODE_PTE(entry);
    }
    return false;
}

uint64_t paging_get_paddr(uint64_t page_table, uint64_t vaddr, int* level_out) {
    uint64_t leaf; int level; uint64_t pte_phys;
    if (!paging_walk(page_table, vaddr, &leaf, &level, &pte_phys))
        return 0;
    if (level_out) *level_out = level;
    return (level == 1) ? ARCH_DECODE_PTE(leaf) : ARCH_DECODE_LARGE_PTE(leaf);
}

uint64_t paging_get_prot(uint64_t page_table, uint64_t vaddr) {
    uint64_t leaf; int level; uint64_t pte_phys;
    if (!paging_walk(page_table, vaddr, &leaf, &level, &pte_phys))
        return 0;
    return mmu_flags_to_prot(leaf, level);
}
