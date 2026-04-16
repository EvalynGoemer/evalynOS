#include <stdint.h>
#include <arch/generic/panic.h>

uint64_t kernel_page_table;

void paging_init() {
    panic("paging_init() is unimplemented for loongarch64");
}

void paging_map_page(uint64_t page_table, uint64_t vaddr, uint64_t paddr, int perms, int page_size) {
    (void)page_table;
    (void)vaddr;
    (void)paddr;
    (void)perms;
    (void)page_size;
    panic("paging_map_page() is unimplemented for loongarch64");
}

void paging_unmap_page(uint64_t page_table, uint64_t vaddr, int page_size) {
    (void)page_table;
    (void)vaddr;
    (void)page_size;
    panic("paging_unmap_page() is unimplemented for loongarch64");
}

uint64_t paging_get_paddr(uint64_t page_table, uint64_t vaddr) {
    (void)page_table;
    (void)vaddr;
    panic("paging_get_paddr() is unimplemented for loongarch64");
    return 0;
}

uint64_t paging_get_pte(uint64_t page_table, uint64_t vaddr, int* size_out) {
    (void)page_table;
    (void)vaddr;
    if (size_out) {
        *size_out = 0;
    }
    panic("paging_get_pte() is unimplemented for loongarch64");
    return 0;
}
