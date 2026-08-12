#pragma once
#include <utils/lib.h>
#include <arch/generic/panic.h>
#include <arch/generic/paging/paging.h>
#include <arch/riscv64/intrin/csr.h>
#include <utils/limine.h>
#include <stdint.h>

#define ARCH_PTE_MASK 0x003fffffffffc00

#define PAGE_SIZE       4096
#define PAGE_SIZE_LARGE (2ULL * 1024 * 1024)
#define PAGE_SIZE_GIANT (1ULL * 1024 * 1024 * 1024)

#define GET_PML5i(vaddr) (((vaddr) >> 48) & 0x1ff)
#define GET_PML4i(vaddr) (((vaddr) >> 39) & 0x1ff)
#define GET_PML3i(vaddr) (((vaddr) >> 30) & 0x1ff)
#define GET_PML2i(vaddr) (((vaddr) >> 21) & 0x1ff)
#define GET_PML1i(vaddr) (((vaddr) >> 12) & 0x1ff)

#define RISCV_PTE_V (1ull << 0)
#define RISCV_PTE_R (1ull << 1)
#define RISCV_PTE_W (1ull << 2)
#define RISCV_PTE_X (1ull << 3)
#define RISCV_PTE_U (1ull << 4)
#define RISCV_PTE_G (1ull << 5)
#define RISCV_PTE_A (1ull << 6)
#define RISCV_PTE_D (1ull << 7)

#define ARCH_INTERMEDIATE_MMU_FLAGS(attr) RISCV_PTE_V
#define ARCH_PTE_PRESENT(pte) ((pte) & RISCV_PTE_V)
#define ARCH_ENCODE_PTE(paddr, flags) (((paddr) >> 2) | (flags))
#define ARCH_DECODE_PTE(pte) (((pte) & ARCH_PTE_MASK) << 2)

static inline uint32_t arch_get_mmu_config() {
    uint32_t config = MMU_CONFIG_L2_LEAF | MMU_CONFIG_L3_LEAF | MMU_CONFIG_NX;
    switch (paging_mode_request.response->mode) {
        case LIMINE_PAGING_MODE_RISCV_SV57: config |= MMU_CONFIG_5LVL_PAGING; break;
        case LIMINE_PAGING_MODE_RISCV_SV48: config |= MMU_CONFIG_4LVL_PAGING; break;
        case LIMINE_PAGING_MODE_RISCV_SV39: config |= MMU_CONFIG_3LVL_PAGING; break;
        default: UNREACHABLE();
    }
    return config;
}

static inline uint64_t prot_to_mmu_flags(uint64_t attr) {
    uint64_t flags = RISCV_PTE_A | RISCV_PTE_D | RISCV_PTE_V | RISCV_PTE_R;
    flags |= (attr & PAGE_W) ? (RISCV_PTE_W | RISCV_PTE_R) : 0;
    flags |= (attr & PAGE_X) ? RISCV_PTE_X : 0;
    flags |= (attr & PAGE_U) ? RISCV_PTE_U : 0;
    return flags;
}

static inline uint64_t mmu_flags_to_prot(uint64_t pte, MAYBE_UNUSED int level) {
    if (!(pte & RISCV_PTE_V)) return 0;

    uint64_t perm = 0;
    perm |= (pte & RISCV_PTE_R) ? PAGE_R : 0;
    perm |= (pte & RISCV_PTE_W) ? PAGE_W : 0;
    perm |= (pte & RISCV_PTE_X) ? PAGE_X : 0;
    perm |= (pte & RISCV_PTE_U) ? PAGE_U : 0;
    return perm;
}

#define RISCV_SATP_SV39 8
#define RISCV_SATP_SV48 9
#define RISCV_SATP_SV57 10

static inline void arch_load_page_table(uint64_t page_table) {
    uint64_t mode;
    switch (MMU_CONFIG_TOP_LEVEL(mmu_config)) {
        case 3: mode = RISCV_SATP_SV39; break;
        case 4: mode = RISCV_SATP_SV48; break;
        case 5: mode = RISCV_SATP_SV57; break;
        default: UNREACHABLE();
    }

    csrw(CSR_SATP, (mode << 60) | (page_table >> 12));
    asm volatile("sfence.vma" ::: "memory");
}

#define arch_large_page_fixup(flags) (flags)

ALWAYS_INLINE static inline uint64_t vaddr_split_bit() {
    return MMU_CONFIG_TOP_LEVEL(mmu_config) * 9 + 12 - 1;
}

ALWAYS_INLINE static inline void arch_tlb_flush(uint64_t vaddr) {
    asm volatile("sfence.vma %0" :: "r"(vaddr) : "memory");
}

ALWAYS_INLINE static inline void arch_tlb_flush_all() {
    asm volatile("sfence.vma" ::: "memory");
}
