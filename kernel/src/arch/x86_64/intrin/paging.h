#pragma once
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/cpu/CRx.h>
#include <arch/generic/panic.h>
#include <arch/generic/paging/paging.h>
#include <utils/limine.h>
#include <stdint.h>

#define ARCH_PTE_MASK 0x000ffffffffff000

#define PAGE_SIZE       4096
#define PAGE_SIZE_LARGE (2ULL * 1024 * 1024)
#define PAGE_SIZE_GIANT (1ULL * 1024 * 1024 * 1024)

#define GET_PML5i(vaddr) (((vaddr) >> 48) & 0x1ff)
#define GET_PML4i(vaddr) (((vaddr) >> 39) & 0x1ff)
#define GET_PML3i(vaddr) (((vaddr) >> 30) & 0x1ff)
#define GET_PML2i(vaddr) (((vaddr) >> 21) & 0x1ff)
#define GET_PML1i(vaddr) (((vaddr) >> 12) & 0x1ff)

#define X86_64_PTE_PRESENT   (1ull << 0)
#define X86_64_PTE_WRITABLE  (1ull << 1)
#define X86_64_PTE_USER      (1ull << 2)
#define X86_64_PTE_PWT       (1ull << 3)
#define X86_64_PTE_PCD       (1ull << 4)
#define X86_64_PTE_ACCESSED  (1ull << 5)
#define X86_64_PTE_DIRTY     (1ull << 6)
#define X86_64_PTE_PS        (1ull << 7)  // only on PML2/3 entries
#define X86_64_PTE_PAT_HUGE  (1ull << 12) // only on PML2/3 entries
#define X86_64_PTE_PAT       (1ull << 7)  // only on PML1 entries
#define X86_64_PTE_GLOBAL    (1ull << 8)
#define X86_64_PTE_NX        (1ull << 63)

#define X86_64_PTE_UC (X86_64_PTE_PWT | X86_64_PTE_PCD)
#define X86_64_PTE_WC (X86_64_PTE_PWT | X86_64_PTE_PAT)

#define ARCH_INTERMEDIATE_MMU_FLAGS(perms) (X86_64_PTE_PRESENT | X86_64_PTE_WRITABLE | ((perms) & PAGE_U ? X86_64_PTE_USER : 0))
#define ARCH_PTE_PRESENT(pte) ((pte) & X86_64_PTE_PRESENT)
#define ARCH_ENCODE_PTE(paddr, flags) (((paddr) & ARCH_PTE_MASK) | (flags))
#define ARCH_DECODE_PTE(pte) ((pte) & ARCH_PTE_MASK)

static inline uint32_t arch_get_mmu_config() {
    uint32_t config = MMU_CONFIG_L2_LEAF;

    if (paging_mode_request.response->mode == LIMINE_PAGING_MODE_X86_64_5LVL)
        config |= MMU_CONFIG_5LVL_PAGING;
    else
        config |= MMU_CONFIG_4LVL_PAGING;

    if (cpuid_check(CPUID_HAS_1GB_PAGES))
        config |= MMU_CONFIG_L3_LEAF;
    if(cpuid_check(CPUID_HAS_NX))
        config |= MMU_CONFIG_NX;

    return config;
}

static inline uint64_t prot_to_mmu_flags(uint64_t perm) {
    uint64_t flags = X86_64_PTE_PRESENT;
    flags |= (perm & PAGE_W)  ? X86_64_PTE_WRITABLE : 0;
    flags |= (perm & PAGE_U)  ? X86_64_PTE_USER     : 0;
    flags |= (perm & PAGE_UC) ? X86_64_PTE_UC       : 0;
    flags |= (perm & PAGE_WC) ? X86_64_PTE_WC       : 0;
    if (mmu_config & MMU_CONFIG_NX) flags |= (perm & PAGE_X)  ? 0 : X86_64_PTE_NX;
    return flags;
}

static inline uint64_t arch_large_page_fixup(uint64_t flags) {
    if (flags & X86_64_PTE_PAT) flags |= X86_64_PTE_PAT_HUGE;
    return flags | X86_64_PTE_PS;
}

static inline uint64_t mmu_flags_to_prot(uint64_t pte, int level) {
    uint64_t perm = 0;
    perm |= (pte & X86_64_PTE_PRESENT)  ? PAGE_R : 0;
    perm |= (pte & X86_64_PTE_WRITABLE) ? PAGE_W : 0;
    perm |= (pte & X86_64_PTE_USER)     ? PAGE_U : 0;
    perm |= (pte & X86_64_PTE_NX)       ? 0      : PAGE_X;

    if (level != 1) {
        if (pte & X86_64_PTE_PWT && pte & X86_64_PTE_PCD)      perm |= PAGE_UC;
        if (pte & X86_64_PTE_PWT && pte & X86_64_PTE_PAT_HUGE) perm |= PAGE_WC;
        return perm;
    }

    if (pte & X86_64_PTE_PWT && pte & X86_64_PTE_PCD) perm |= PAGE_UC;
    if (pte & X86_64_PTE_PWT && pte & X86_64_PTE_PAT) perm |= PAGE_WC;
    return perm;
}

ALWAYS_INLINE static inline uint64_t vaddr_split_bit() {
    return (mmu_config & MMU_CONFIG_LVLS_MASK) * 9 + 12 - 1;
}

ALWAYS_INLINE static inline void arch_load_page_table(uint64_t page_table) {
    write_cr3(page_table);
}

ALWAYS_INLINE static inline void arch_tlb_flush(uint64_t vaddr) {
    asm volatile("invlpg (%0)" :: "r"(vaddr) : "memory");
}

ALWAYS_INLINE static inline void arch_tlb_flush_all() {
    write_cr3(read_cr3());
}
