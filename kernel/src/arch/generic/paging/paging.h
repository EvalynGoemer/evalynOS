#pragma once
#include <stdint.h>

extern uint32_t mmu_config;
#define MMU_CONFIG_LVLS_MASK (0x7)
#define MMU_CONFIG_L3        (1 << 3)
#define MMU_CONFIG_L4        (1 << 4)
#define MMU_CONFIG_L5        (1 << 5)
#define MMU_CONFIG_L3_LEAF   (1 << 6)
#define MMU_CONFIG_L2_LEAF   (1 << 7)
#define MMU_CONFIG_NX        (1 << 8)

#define MMU_CONFIG_3LVL_PAGING ((0b001 << 3) | 3)
#define MMU_CONFIG_4LVL_PAGING ((0b011 << 3) | 4)
#define MMU_CONFIG_5LVL_PAGING ((0b111 << 3) | 5)

#define PAGE_NORM  1
#define PAGE_LARGE 2
#define PAGE_GIANT 3

#define PAGE_R   (1 << 0)
#define PAGE_W   (1 << 1)
#define PAGE_X   (1 << 2)
#define PAGE_U   (1 << 3)
#define PAGE_UC  (1 << 4)
#define PAGE_WC  (1 << 5)

#define PAGE_KRO    (PAGE_R)
#define PAGE_KRW    (PAGE_R | PAGE_W)
#define PAGE_KRX    (PAGE_R | PAGE_X)
#define PAGE_KRWX   (PAGE_R | PAGE_W | PAGE_X)
#define PAGE_KRW_UC (PAGE_R | PAGE_W | PAGE_UC)
#define PAGE_KRW_WC (PAGE_R | PAGE_W | PAGE_WC)

#define PAGE_URO    (PAGE_U | PAGE_R)
#define PAGE_URW    (PAGE_U | PAGE_R | PAGE_W)
#define PAGE_URX    (PAGE_U | PAGE_R | PAGE_X)
#define PAGE_URWX   (PAGE_U | PAGE_R | PAGE_W | PAGE_X)
#define PAGE_URW_UC (PAGE_U | PAGE_R | PAGE_W | PAGE_UC)
#define PAGE_URW_WC (PAGE_U | PAGE_R | PAGE_W | PAGE_WC)

extern uint64_t kernel_page_table;
extern void paging_init();
extern void paging_map_page (uint64_t page_table, uint64_t vaddr, uint64_t paddr, int perms,int page_size);

extern uint64_t VADDR_LOWER_HALF_TOP;
extern uint64_t VADDR_HIGHER_HALF_BASE;

#include <arch/intrin/paging.h>
