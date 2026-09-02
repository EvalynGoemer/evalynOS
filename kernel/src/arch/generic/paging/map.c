#include <stdint.h>
#include <assert.h>
#include <utils/lib.h>
#include <mem/pfndb.h>
#include <arch/generic/paging/paging.h>
#include <arch/generic/paging/helpers.h>

void paging_early_map_page(uint64_t page_table, uint64_t vaddr, uint64_t paddr, int attr) {
    int top_level = MMU_CONFIG_TOP_LEVEL(mmu_config);
    int leaf_level = PAGE_LEAF_LEVEL(attr);
    uint64_t current_table = page_table;
    assert_alignment(vaddr, paddr, leaf_level);

    for (int level = top_level; level > leaf_level; level--) {
        uint64_t pte_phys = paging_get_pte(current_table, vaddr, level);
        current_table = get_next_level_and_bump_allocate(pte_phys, PAGE_KRWX);
    }

    uint64_t leaf_pte = paging_get_pte(current_table, vaddr, leaf_level);
    write_leaf(leaf_pte, paddr, leaf_level, attr);
    arch_tlb_flush(vaddr);
}

void paging_map_range(uint64_t page_table, uint64_t vaddr, uint64_t paddr, size_t len, int attr) {
    int top_level = MMU_CONFIG_TOP_LEVEL(mmu_config);
    uint64_t orig_vaddr = vaddr;
    size_t orig_len = len;

    dbg_assert(IS_ALIGNED(vaddr, PAGE_SIZE));
    dbg_assert(IS_ALIGNED(paddr, PAGE_SIZE));
    dbg_assert(IS_ALIGNED(len, PAGE_SIZE));

    uint64_t tables[6];
    tables[top_level] = page_table;
    int level = top_level;
    int index = PAGING_INDEX(vaddr, level);

    while (len > 0) {
        // at the end of the level go up
        if (index == PAGING_TABLE_ENTRIES) {
            if (level == top_level) break;
            level++;
            index = PARENT_PAGING_INDEX(vaddr, level);
            continue;
        }

        uint64_t entry_size = PAGING_ENTRY_SIZE(level);
        uint64_t pte_phys = paging_get_pte(tables[level], vaddr, level);
        uint64_t* vpte = TO_HHDM_PTR(pte_phys);
        uint64_t entry = *vpte;

        // at the base level bulk map pages
        if (level == 1) {
            size_t run = MIN((size_t)(PAGING_TABLE_ENTRIES - index), len >> PAGE_SIZE_SHIFT);
            uint64_t flags = prot_to_mmu_flags(attr);
            int added = 0;
            for (size_t i = 0; i < run; i++) {
                if (!ARCH_PTE_PRESENT(vpte[i])) added++;
                vpte[i] = ARCH_ENCODE_PTE(paddr + i * PAGE_SIZE, flags);
            }
            update_refcnt(tables[level], added);
            vaddr += run << PAGE_SIZE_SHIFT;
            paddr += run << PAGE_SIZE_SHIFT;
            len   -= run << PAGE_SIZE_SHIFT;
            index += run;
            continue;
        }

        bool leaf_supported = mmu_supports_leaf(level);
        bool leaf_valid = !ARCH_PTE_PRESENT(entry) || ARCH_PTE_IS_LEAF(entry, level);
        bool aligned = IS_ALIGNED(vaddr, entry_size) && IS_ALIGNED(paddr, entry_size);

        // do a large page if able to
        if (leaf_supported && leaf_valid && aligned && len >= entry_size) {
            if (!ARCH_PTE_PRESENT(entry)) update_refcnt(tables[level], 1);
            write_leaf(pte_phys, paddr, level, attr);
            vaddr += entry_size;
            paddr += entry_size;
            len   -= entry_size;
            index++;
            continue;
        }

        uint64_t child;
        if (!ARCH_PTE_PRESENT(entry)) {
            // pte is missing, allocate a new level
            child = pmm_alloc_page();
            page_t* pg = pfndb_get_entry(child);
            pg->type = PFNDB_PAGE_PTABLE;
            pg->ref_count = 0;
            *vpte = ARCH_ENCODE_PTE(child, ARCH_INTERMEDIATE_MMU_FLAGS(attr));
            update_refcnt(tables[level], 1);
        } else if (ARCH_PTE_IS_LEAF(entry, level)) {
            // else if its a leaf split it
            child = paging_split_leaf(pte_phys, level);
        } else {
            // otherwise get the next level
            child = ARCH_DECODE_PTE(entry);
        }

        // go down a level
        tables[--level] = child;
        index = PAGING_INDEX(vaddr, level);
    }

    arch_tlb_flush_range(orig_vaddr, orig_len);
}

void paging_map_page(uint64_t page_table, uint64_t vaddr, uint64_t paddr, int attr) {
    int leaf_level = PAGE_LEAF_LEVEL(attr);
    dbg_assert(IS_ALIGNED(vaddr, PAGING_ENTRY_SIZE(leaf_level)));
    paging_map_range(page_table, vaddr, paddr, PAGING_ENTRY_SIZE(leaf_level), attr);
}
