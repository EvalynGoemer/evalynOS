#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <utils/lib.h>
#include <mem/pfndb.h>
#include <mem/pmm.h>
#include <arch/generic/paging/helpers.h>
#include <arch/generic/paging/paging.h>

static inline bool paging_try_free_table(uint64_t* tables, int level, uint64_t vaddr, int top_level) {
    if (level >= top_level || get_refcnt(tables[level]) != 0) return false;
    uint64_t* pparent = TO_HHDM_PTR(tables[level + 1]);
    pparent[PAGING_INDEX(vaddr, level + 1)] = 0;
    update_refcnt(tables[level + 1], -1);
    pmm_free_page(tables[level]);
    return true;
}

void paging_unmap_range(uint64_t page_table, uint64_t vaddr, size_t len) {
    int top_level = MMU_CONFIG_TOP_LEVEL(mmu_config);
    uint64_t orig_vaddr = vaddr;
    size_t orig_len = len;

    dbg_assert(IS_ALIGNED(vaddr, PAGE_SIZE));
    dbg_assert(IS_ALIGNED(len, PAGE_SIZE));

    uint64_t tables[6];
    tables[top_level] = page_table;
    int level = top_level;
    int index = PAGING_INDEX(vaddr, level);

    while (true) {
        // at the end of the level try to free and go up
        if (index == PAGING_TABLE_ENTRIES) {
            if (level == top_level) break;
            paging_try_free_table(tables, level, vaddr - PAGING_ENTRY_SIZE(level + 1), top_level);
            level++;
            index = PARENT_PAGING_INDEX(vaddr, level);
            continue;
        }

        if (len == 0) {
            // when we are done unmapping go up and free things
            while (paging_try_free_table(tables, level++, vaddr, top_level));
            break;
        }

        uint64_t entry_size = PAGING_ENTRY_SIZE(level);
        uint64_t pte_phys = paging_get_pte(tables[level], vaddr, level);
        uint64_t* vpte = TO_HHDM_PTR(pte_phys);
        uint64_t entry = *vpte;

        // skip non present portions
        if (!ARCH_PTE_PRESENT(entry)) {
            uint64_t remaining = entry_size - MISALIGNMENT(vaddr, entry_size);
            uint64_t advance = MIN(len, remaining);
            vaddr += advance;
            len   -= advance;
            if (advance == remaining) index++;
            continue;
        }

        // at the base bulk unmap pages
        if (level == 1) {
            size_t run = MIN((size_t)(PAGING_TABLE_ENTRIES - index), len >> PAGE_SIZE_SHIFT);
            int removed = 0;
            for (size_t i = 0; i < run; i++) {
                if (ARCH_PTE_PRESENT(vpte[i])) removed++;
                vpte[i] = 0;
            }
            update_refcnt(tables[level], -removed);
            vaddr += run << PAGE_SIZE_SHIFT;
            len   -= run << PAGE_SIZE_SHIFT;
            index += run;
            continue;
        }

        uint64_t child;
        // if its a leaf
        if (ARCH_PTE_IS_LEAF(entry, level)) {
            // and if it covers the entire entry unmap it
            if (IS_ALIGNED(vaddr, entry_size) && len >= entry_size) {
                *vpte = 0;
                update_refcnt(tables[level], -1);
                vaddr += entry_size;
                len   -= entry_size;
                index++;
                continue;
            }
            // else split it
            child = paging_split_leaf(pte_phys, level);
        } else {
            child = ARCH_DECODE_PTE(entry);
        }

        // go down a level
        tables[--level] = child;
        index = PAGING_INDEX(vaddr, level);
    }

    arch_tlb_flush_range(orig_vaddr, orig_len);
}

void paging_unmap_page(uint64_t page_table, uint64_t vaddr, int attr) {
    int leaf_level = PAGE_LEAF_LEVEL(attr);
    dbg_assert(IS_ALIGNED(vaddr, PAGING_ENTRY_SIZE(leaf_level)));
    paging_unmap_range(page_table, vaddr, PAGING_ENTRY_SIZE(leaf_level));
}
