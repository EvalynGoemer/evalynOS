#pragma once
#include <stdint.h>

// paging functions take in these defines and convert to internal flags for the architecture

#define PAGE_SIZE_NORM  1
#define PAGE_SIZE_LARGE 2
#define PAGE_SIZE_GIANT 3

#define PAGE_R   (1 << 1)
#define PAGE_W   (1 << 2)
#define PAGE_X   (1 << 3)
#define PAGE_U   (1 << 4)
#define PAGE_UC  (1 << 5)
#define PAGE_WC  (1 << 6)

#define PAGE_NONE   0

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
extern void paging_unmap_page (uint64_t page_table, uint64_t vaddr, int page_size);
extern uint64_t paging_get_paddr (uint64_t page_table, uint64_t vaddr);
extern uint64_t paging_get_pte(uint64_t page_table, uint64_t vaddr,int* size_out);
