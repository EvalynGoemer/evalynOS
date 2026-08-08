#pragma once
#include <stddef.h>
#include <stdint.h>
#include <utils/lib.h>

#include STRINGIFY(arch/TARGET_ARCH/intrin/paging.h)

static inline void arch_tlb_flush_range(uint64_t vaddr, size_t size) {
    uint64_t start = ALIGN_DOWN(vaddr, PAGE_SIZE);
    uint64_t end = ALIGN_UP(vaddr + size, PAGE_SIZE);
    for (uint64_t curr = start; curr < end; curr += PAGE_SIZE)
        arch_tlb_flush(curr);
}

static inline size_t paging_get_vaddr_lower_half_top() {
    return (1ull << vaddr_split_bit()) - 1;
}

static inline size_t paging_get_vaddr_higher_half_base() {
    return 0ull - (1ull << vaddr_split_bit());
}
