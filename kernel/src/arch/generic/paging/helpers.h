#pragma once
#include <stdint.h>
#include <assert.h>
#include <mem/pmm.h>
#include <mem/balloc.h>
#include <mem/pfndb.h>
#include <arch/generic/paging/paging.h>
#include <utils/lib.h>

#define PAGING_TABLE_ENTRIES (PAGE_INDEX_MASK + 1)
#define PAGING_ENTRY_SIZE(level) (1ull << (PAGE_SIZE_SHIFT + ((level) - 1) * PAGE_INDEX_BITS))
#define PAGING_ENTRY_SHIFT(level) (PAGE_SIZE_SHIFT + ((level) - 1) * PAGE_INDEX_BITS)
#define PAGING_INDEX(vaddr, level) (((vaddr) >> PAGING_ENTRY_SHIFT(level)) & PAGE_INDEX_MASK)
#define PARENT_PAGING_INDEX(vaddr, level) (PAGING_INDEX((vaddr) - PAGING_ENTRY_SIZE(level), level) + 1)

extern uint64_t paging_split_leaf(uint64_t pte_phys, int level);

static inline uint64_t paging_get_pte(uint64_t table, uint64_t vaddr, int level) {
    return table + PAGING_INDEX(vaddr, level) * sizeof(uint64_t);
}

static inline void update_refcnt(uint64_t paddr, int amount) {
    page_t* page = pfndb_get_entry(paddr);
    if (page->type == PFNDB_PAGE_INVALID) return;
    dbg_assert(page->type == PFNDB_PAGE_PTABLE);
    if (amount < 0) dbg_assert(page->ref_count >= (uint32_t)-amount);
    page->ref_count += amount;
}

static inline uint32_t get_refcnt(uint64_t paddr) {
    page_t* page = pfndb_get_entry(paddr);
    if (page->type == PFNDB_PAGE_INVALID) return UINT32_MAX;
    dbg_assert(page->type == PFNDB_PAGE_PTABLE);
    return page->ref_count;
}

static inline void assert_alignment(uint64_t vaddr, uint64_t paddr, int leaf_level) {
    uint64_t psz = PAGING_ENTRY_SIZE(leaf_level);
    assert(IS_ALIGNED(vaddr, psz));
    assert(IS_ALIGNED(paddr, psz));
}

static inline bool mmu_supports_leaf(int level) {
    if (level == 1) return true;
    if (level == 2 && (mmu_config & MMU_CONFIG_L2_LEAF)) return true;
    if (level == 3 && (mmu_config & MMU_CONFIG_L3_LEAF)) return true;
    return false;
}

static inline void write_leaf(uint64_t pte_phys, uint64_t paddr, int level, int attr) {
    uint64_t* pte_virt = TO_HHDM_PTR(pte_phys);
    uint64_t flags = prot_to_mmu_flags(attr);
    if (level != 1) flags = arch_large_page_fixup(flags);
    *pte_virt = ARCH_ENCODE_PTE(paddr, flags);
}

static inline uint64_t get_next_level_and_bump_allocate(uint64_t pte_phys, MAYBE_UNUSED int attr) {
    uint64_t* vpte = TO_HHDM_PTR(pte_phys);
    uint64_t entry = *vpte;

    if (!ARCH_PTE_PRESENT(entry)) {
        uint64_t new_table = balloc_alloc_page();
        entry = ARCH_ENCODE_PTE(new_table, ARCH_INTERMEDIATE_MMU_FLAGS(attr));
        *vpte = entry;
    }

    return ARCH_DECODE_PTE(entry);
}
