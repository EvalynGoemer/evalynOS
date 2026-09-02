#pragma once
#ifndef __ASSEMBLER__
#include <arch/generic/panic.h>
#include <arch/generic/paging/paging.h>
#include <arch/loongarch64/cpu/tlb/tlb.h>
#include <arch/loongarch64/cpu/cpucfg.h>
#include <arch/loongarch64/intrin/csr.h>
#include <utils/limine.h>
#include <utils/lib.h>
#include <stdint.h>
#include <assert.h>
#endif

#define ARCH_UNPAGED_HHDM
#define ARCH_SEPARATE_PAGING_ROOTS
#define ARCH_MMU_NEEDS_INIT

#define ARCH_PTE_MASK 0x000ffffffffff000
#define ARCH_LARGE_PTE_MASK 0x000fffffffffe000

#if defined(CONFIG_LA64_PAGESIZE_64KB)
    #define PAGE_SIZE_SHIFT       16
    #define PAGE_SIZE_LARGE_SHIFT 29
    #define PAGE_SIZE_GIANT_SHIFT 42
    #define PAGE_INDEX_BITS       13
    #define PAGE_INDEX_MASK       0x1fff
    #define LA64_VA_BITS          55
    #define LA64_PAGING_LEVELS    3
    #define LA64_PWCL        ((16ull << 0) | (13ull << 5) | (29ull << 20) | (13ull << 25))
    #define LA64_PWCH        ((42ull << 0) | (13ull << 6))
#elif defined(CONFIG_LA64_PAGESIZE_16KB)
    #define PAGE_SIZE_SHIFT       14
    #define PAGE_SIZE_LARGE_SHIFT 25
    #define PAGE_SIZE_GIANT_SHIFT 36
    #define PAGE_INDEX_BITS       11
    #define PAGE_INDEX_MASK       0x7ff
    #define LA64_VA_BITS          47
    #define LA64_PAGING_LEVELS    3
    #define LA64_PWCL        ((14ull << 0) | (11ull << 5) | (25ull << 20) | (11ull << 25))
    #define LA64_PWCH        ((36ull << 0) | (11ull << 6))
#elif defined(CONFIG_LA64_PAGESIZE_4KB)
    #define PAGE_SIZE_SHIFT       12
    #define PAGE_SIZE_LARGE_SHIFT 21
    #define PAGE_SIZE_GIANT_SHIFT 30
    #define PAGE_INDEX_BITS       9
    #define PAGE_INDEX_MASK       0x1ff
    #define LA64_PWCL         ((12ull << 0) | (9ull << 5) | (21ull << 10) | (9ull << 15) | (30ull << 20) | (9ull << 25))
    #define LA64_PWCH         ((39ull << 0) | (9ull << 6))
    #define LA64_VA_BITS       48
    #define LA64_PAGING_LEVELS 4
#endif

#define PAGE_SIZE       (1ull << PAGE_SIZE_SHIFT)
#define PAGE_SIZE_LARGE (1ull << PAGE_SIZE_LARGE_SHIFT)
#define PAGE_SIZE_GIANT (1ull << PAGE_SIZE_GIANT_SHIFT)

#define LOONGARCH_PTE_V        (1ull << 0)
#define LOONGARCH_PTE_D        (1ull << 1)
#define LOONGARCH_PTE_PLV0     (0ull << 2)
#define LOONGARCH_PTE_PLV3     (3ull << 2)
#define LOONGARCH_PTE_H        (1ull << 6)  // only on level 3/2
#define LOONGARCH_PTE_G        (1ull << 6)  // only on level 1
#define LOONGARCH_PTE_G_LARGE  (1ull << 12) // only on level 3/2
#define LOONGARCH_PTE_P        (1ull << 7)
#define LOONGARCH_PTE_W        (1ull << 8)
#define LOONGARCH_PTE_NX       (1ull << 62)

#define LOONGARCH_PTE_MAT_CACHED          (1ull << 4)
#define LOONGARCH_PTE_MAT_WEAK_UNCACHED   (2ull << 4)
#define LOONGARCH_PTE_MAT_STRONG_UNCACHED (0ull << 4)

#ifndef __ASSEMBLER__

#define ARCH_INTERMEDIATE_MMU_FLAGS(attr) 0
#define ARCH_PTE_PRESENT(pte) ((pte) != 0)
#define ARCH_PTE_IS_LEAF(pte, level) ((level) == 1 || ((level) <= 3 && ((pte) & LOONGARCH_PTE_H)))
#define ARCH_ENCODE_PTE(paddr, flags) (((paddr) & ARCH_PTE_MASK) | (flags))
#define ARCH_DECODE_PTE(pte) ((pte) & ARCH_PTE_MASK)
#define ARCH_DECODE_LARGE_PTE(pte) ((pte) & ARCH_LARGE_PTE_MASK)

static inline uint32_t arch_get_mmu_config() {
    uint32_t config = MMU_CONFIG_NX;
    if (LA64_PAGING_LEVELS == 3)
        config |= MMU_CONFIG_3LVL_PAGING;
    else
        config |= MMU_CONFIG_4LVL_PAGING;
    uint64_t psavl = csrrd(CSR_PRCFG2);
    if (psavl & (1ull << (PAGE_SIZE_LARGE_SHIFT))) config |= MMU_CONFIG_L2_LEAF;
    if (psavl & (1ull << (PAGE_SIZE_GIANT_SHIFT))) config |= MMU_CONFIG_L3_LEAF;
    return config;
}

static inline void arch_init_mmu() {
    uint64_t psavl = csrrd(CSR_PRCFG2);
    if (!(psavl & (1ull << PAGE_SIZE_SHIFT)))
        panic("Configured page size is not supported by the CPU");

    // execute the function via the DMW so nothing breaks when changing the paging CSRs
    uint64_t vbase = executable_address_request.response->virtual_base;
    uint64_t pbase = executable_address_request.response->physical_base;
    uint64_t stub_phys = (uint64_t)setup_mmu - vbase + pbase;
    uint64_t handler_phys = (uint64_t)tlb_refill_handler - vbase + pbase;
    void (*fn)(uint64_t, uint64_t) = TO_HHDM_PTR(stub_phys);
    fn(kernel_page_table, handler_phys);
}

static inline uint64_t prot_to_mmu_flags(uint64_t attr) {
    uint64_t flags = LOONGARCH_PTE_V | LOONGARCH_PTE_P;
    flags |= (attr & PAGE_W)  ? LOONGARCH_PTE_W | LOONGARCH_PTE_D : 0;
    flags |= (attr & PAGE_X)  ? 0 : LOONGARCH_PTE_NX;
    flags |= (attr & PAGE_U)  ? LOONGARCH_PTE_PLV3 : LOONGARCH_PTE_PLV0;
    flags |= (attr & PAGE_G)  ? LOONGARCH_PTE_G : 0;
    flags |= (attr & PAGE_UC) ? LOONGARCH_PTE_MAT_STRONG_UNCACHED : 0;
    flags |= (attr & PAGE_WC) ? LOONGARCH_PTE_MAT_WEAK_UNCACHED : 0;
    flags |= (!(attr & (PAGE_UC | PAGE_WC))) ? LOONGARCH_PTE_MAT_CACHED : 0;
    return flags;
}

static inline uint64_t arch_large_page_fixup(uint64_t flags) {
    if (flags & LOONGARCH_PTE_G) flags |= LOONGARCH_PTE_G_LARGE;
    return flags | LOONGARCH_PTE_H;
}

static inline uint64_t mmu_flags_to_prot(uint64_t pte, int level) {
    if (!(pte & LOONGARCH_PTE_V)) return 0;

    uint64_t perm = 0;
    perm |= PAGE_R;
    perm |= (pte & LOONGARCH_PTE_W) ? PAGE_W : 0;
    perm |= (pte & LOONGARCH_PTE_NX) ? 0 : PAGE_X;
    perm |= (((pte >> 2) & 0x3) != 0) ? PAGE_U : 0;
    if (level == 1) perm |= (pte & LOONGARCH_PTE_G) ? PAGE_G : 0;
    else perm |= (pte & LOONGARCH_PTE_G_LARGE) ? PAGE_G : 0;

    uint8_t mat = (pte >> 4) & 0x3;
    if (mat == 0)      perm |= PAGE_UC;
    else if (mat == 2) perm |= PAGE_WC;

    return perm;
}

static inline void arch_load_page_table(uint64_t page_table) {
    if (page_table == kernel_page_table)
        csrwr(CSR_PGDH, page_table);
    else
        csrwr(CSR_PGDL, page_table);
    asm volatile("invtlb 0, $r0, $r0" ::: "memory");
}

static inline uint64_t vaddr_split_bit() {
    uint64_t valen = ((cpucfg(1) >> 12) & 0xff) + 1;
    assert(valen >= 39);
    return MIN(valen - 1, LA64_VA_BITS - 1);
}

ALWAYS_INLINE static inline void arch_tlb_flush(uint64_t vaddr) {
    asm volatile("invtlb 6, $r0, %0" :: "r"(vaddr) : "memory");
}

ALWAYS_INLINE static inline void arch_tlb_flush_all() {
    asm volatile("invtlb 0, $r0, $r0" ::: "memory");
}

#endif
