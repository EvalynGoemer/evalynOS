#include "arch/generic/panic.h"
#include <stdint.h>
#include <stdio.h>

#include <arch/generic/paging/paging.h>
#include <arch/x86_64/paging/paging.h>
#include <arch/x86_64/cpu/cpuid.h>
#include <arch/x86_64/cpu/CRx.h>
#include <loader/elf_structs.h>
#include <mem/pmm.h>
#include <utils/limine.h>
#include <utils/align.h>

#define PAGE_SIZE_2MB  (2ULL * 1024 * 1024)
#define PAGE_SIZE_1GB  (1ULL * 1024 * 1024 * 1024)

uint64_t kernel_page_table;
bool _1gb_pages_supported;
bool is_5_level_paging = false;

/* Internal Helpers */
static inline uint64_t prot_to_mmu_flags(uint64_t perm) {
    if (perm == PAGE_NONE) return 0;
    uint64_t flags = X86_64_PTE_NX;
    flags |= (perm & PAGE_R)  ? X86_64_PTE_PRESENT  : 0;
    flags |= (perm & PAGE_W)  ? X86_64_PTE_WRITABLE : 0;
    flags |= (perm & PAGE_U)  ? X86_64_PTE_USER     : 0;
    flags |= (perm & PAGE_UC) ? X86_64_PTE_UC       : 0;
    flags |= (perm & PAGE_WC) ? X86_64_PTE_WC       : 0;
    flags ^= (perm & PAGE_X)  ? X86_64_PTE_NX       : 0;
    return flags;
}

static inline uint64_t get_next_level_and_allocate(uint64_t pte, int perms) {
    uint64_t* vpte = (uint64_t*)(pte + hhdm_request.response->offset);
    uint64_t entry = *vpte;

    if (!(entry & X86_64_PTE_PRESENT)) {
        uint64_t new_table = pmm_alloc_page();
        entry = new_table | prot_to_mmu_flags(perms);
        *vpte = entry;
    }

    return entry & X86_64_PTE_MASK;
}

static inline bool pte_walk(uint64_t table, uint16_t index, uint64_t **pte_out, uint64_t *next_table_out) {
    uint64_t phys = table + (index * sizeof(uint64_t));
    uint64_t *pte = (uint64_t*)(phys + hhdm_request.response->offset);
    *pte_out = pte;
    if (!(*pte & X86_64_PTE_PRESENT)) return false;
    *next_table_out = *pte & X86_64_PTE_MASK;
    return true;
}

static inline void handle_large_page(uint64_t current_pte_phys, uint64_t paddr, uint64_t vaddr, int perms) {
    uint64_t *pte_virt = (uint64_t *)(current_pte_phys + hhdm_request.response->offset);
    uint64_t flags = prot_to_mmu_flags(perms);
    // PAT is bit 12 on large pages
    if (flags & X86_64_PTE_PAT) flags |= (1 << 12);
    *pte_virt = (paddr & X86_64_PTE_MASK) | flags | X86_64_PTE_PS;
    asm volatile("invlpg (%0)" :: "r"(vaddr) : "memory");
}

/* Public Functions */

void paging_init() {
    if (paging_mode_request.response->mode == LIMINE_PAGING_MODE_X86_64_5LVL) {
        LOG_TAGGED("MEMORY", ANSI_BGREEN, "System is using 5 Level Paging")
        is_5_level_paging = true;
    } else {
        LOG_TAGGED("MEMORY", ANSI_BGREEN, "System is using 4 Level Paging")
        is_5_level_paging = false;
    }

    if (cpuid_check(CPUID_HAS_1GB_PAGES)) {
        LOG_TAGGED("MEMORY", ANSI_BGREEN, "System supports up to 1GB pages")
        LOG_TAGGED("MEMORY", ANSI_BGREEN, "1GB Pages will be used for HHDM")
        _1gb_pages_supported = true;
    } else {
        LOG_TAGGED("MEMORY", ANSI_BGREEN, "System supports up to 2MB pages")
        LOG_TAGGED("MEMORY", ANSI_BGREEN, "2MB Pages will be used for HHDM")
        _1gb_pages_supported = false;
    }

    kernel_page_table = pmm_alloc_page();

    // pre allocate page tables for upper half so cloning tables is simpler
    for (int i = 256; i < 512; i++) {
        uint64_t pte_phys = kernel_page_table + (i * sizeof(uint64_t));
        get_next_level_and_allocate(pte_phys, PAGE_KRWX);
    }


    LOG_TAGGED("MEMORY", ANSI_BGREEN, "Mapping HHDM")
    struct limine_memmap_response *memmap = memmap_request.response;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_RESERVED)   continue;
        if (entry->type == LIMINE_MEMMAP_BAD_MEMORY) continue;

        uint64_t base = ALIGN_DOWN(entry->base, PAGE_SIZE);
        uint64_t end = ALIGN_UP(entry->base + entry->length, PAGE_SIZE);
        uint64_t flags = PAGE_KRW;
        if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER) flags = PAGE_KRW_WC;

        uint64_t addr = base;
        while (addr < end) {
            if (_1gb_pages_supported && addr + PAGE_SIZE_1GB <= end && IS_ALIGNED(addr, PAGE_SIZE_1GB)) {
                paging_map_page(kernel_page_table, addr + hhdm_request.response->offset, addr, flags, PAGE_SIZE_GIANT);
                addr += PAGE_SIZE_1GB;
            } else if (addr + PAGE_SIZE_2MB <= end && IS_ALIGNED(addr, PAGE_SIZE_2MB)) {
                paging_map_page(kernel_page_table, addr + hhdm_request.response->offset, addr, flags, PAGE_SIZE_LARGE);
                addr += PAGE_SIZE_2MB;
            } else {
                paging_map_page(kernel_page_table, addr + hhdm_request.response->offset, addr, flags, PAGE_SIZE_NORM);
                addr += PAGE_SIZE;
            }
        }
    }

    LOG_TAGGED("MEMORY", ANSI_BGREEN, "Mapping Kernel ELF")
    struct limine_executable_address_response *kaddr = executable_address_request.response;
    struct limine_executable_file_response *kexec = executable_file_request.response;
    struct elf_header_64 *header = (struct elf_header_64 *)kexec->executable_file->address;
    struct elf_program_header_64 *prog_headers = (struct elf_program_header_64 *)((uint8_t *)kexec->executable_file->address + header->program_header_table);

    uint64_t elf_base = -1;
    for (uint16_t i = 0; i < header->program_header_entries; i++) {
        struct elf_program_header_64 *ph = &prog_headers[i];
        if (ph->type == ELF_PROG_PT_LOAD_TYPE && ph->virt_addr < elf_base)
            elf_base = ph->virt_addr;
    }
    if (elf_base == (uint64_t)-1)
        panic("Unable to get kernel base load address from elf");

    LOG_TAGGED("MEMORY", ANSI_BGREEN, "Kernel ELF Base Addr: 0x%016lx", elf_base)
    LOG_TAGGED("MEMORY", ANSI_BGREEN, "Kernel Loaded at Virt: 0x%016lx", kaddr->virtual_base)
    LOG_TAGGED("MEMORY", ANSI_BGREEN, "Kernel Loaded at Phys: 0x%016lx", kaddr->physical_base)

    for (uint16_t i = 0; i < header->program_header_entries; i++) {
        struct elf_program_header_64 *ph = &prog_headers[i];
        if (ph->type != ELF_PROG_PT_LOAD_TYPE) continue;
        uint64_t flags = PAGE_R;
        if (ph->flags & ELF_PROG_WRITE)     flags |= PAGE_W;
        if (ph->flags & ELF_PROG_EXEC_FLAG) flags |= PAGE_X;

        uint64_t pages = ALIGN_UP(ph->mem_size, PAGE_SIZE) / PAGE_SIZE;
        uint64_t vbase = ph->virt_addr + kaddr->virtual_base - elf_base;
        uint64_t pbase = kaddr->physical_base + ph->virt_addr - elf_base;
        for (uint64_t p = 0; p < pages; p++) {
            uint64_t vaddr = vbase + (p * PAGE_SIZE);
            uint64_t paddr = pbase + (p * PAGE_SIZE);
            paging_map_page(kernel_page_table, vaddr, paddr, flags, PAGE_SIZE_NORM);
        }
    }

    LOG_TAGGED("MEMORY", ANSI_BGREEN, "Swapping to new page tables")
    write_cr3(kernel_page_table);
    LOG_TAGGED_OK("MEMORY", ANSI_BGREEN, "Paging Init")
}

static inline void check_align(uint64_t vaddr, uint64_t paddr, uint64_t align, const char *msg) {
    if (!IS_ALIGNED(vaddr, align) || !IS_ALIGNED(paddr, align))
        panic(msg);
}

// callers must lock page tables if required
// callers must not attempt to map a large page ontop of a smaller page and vice versa
void paging_map_page (uint64_t page_table, uint64_t vaddr, uint64_t paddr, int perms, int page_size) {
    switch (page_size) {
        case PAGE_SIZE_NORM: check_align(vaddr, paddr, PAGE_SIZE, "tried to map a 4kb page with poor alignment"); break;
        case PAGE_SIZE_LARGE: check_align(vaddr, paddr, PAGE_SIZE_2MB, "tried to map a 2mb page with poor alignment"); break;
        case PAGE_SIZE_GIANT: check_align(vaddr, paddr, PAGE_SIZE_1GB, "tried to map a 1gb page with poor alignment"); break;
    }

    uint64_t current_table = page_table;
    uint64_t current_pte_phys;
    uint16_t pml1i = (vaddr >> 12) & 0x1ff;
    uint16_t pml2i = (vaddr >> 21) & 0x1ff;
    uint16_t pml3i = (vaddr >> 30) & 0x1ff;
    uint16_t pml4i = (vaddr >> 39) & 0x1ff;

    if (is_5_level_paging) {
        uint16_t pml5i = (vaddr >> 48) & 0x1ff;
        current_pte_phys = current_table + (pml5i * sizeof(uint64_t));
        current_table = get_next_level_and_allocate(current_pte_phys, PAGE_URWX);
    }

    current_pte_phys = current_table + (pml4i * sizeof(uint64_t));
    current_table = get_next_level_and_allocate(current_pte_phys, PAGE_URWX);

    current_pte_phys = current_table + (pml3i * sizeof(uint64_t));
    if (page_size == PAGE_SIZE_GIANT) {
        handle_large_page(current_pte_phys, paddr, vaddr, perms);
        return;
    }
    current_table = get_next_level_and_allocate(current_pte_phys, PAGE_URWX);

    current_pte_phys = current_table + (pml2i * sizeof(uint64_t));
    if (page_size == PAGE_SIZE_LARGE) {
        handle_large_page(current_pte_phys, paddr, vaddr, perms);
        return;
    }
    current_table = get_next_level_and_allocate(current_pte_phys, PAGE_URWX);

    uint64_t* pte_virt = (uint64_t*)((current_table + (pml1i * sizeof(uint64_t))) + hhdm_request.response->offset );
    *pte_virt = (paddr & X86_64_PTE_MASK) | prot_to_mmu_flags(perms);
    asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
}

void paging_unmap_page(uint64_t page_table, uint64_t vaddr, int page_size) {
    uint64_t current_table = page_table;
    uint64_t *pte;

    uint16_t pml1i = (vaddr >> 12) & 0x1ff;
    uint16_t pml2i = (vaddr >> 21) & 0x1ff;
    uint16_t pml3i = (vaddr >> 30) & 0x1ff;
    uint16_t pml4i = (vaddr >> 39) & 0x1ff;

    if (is_5_level_paging) {
        uint16_t pml5i = (vaddr >> 48) & 0x1ff;
        if (!pte_walk(current_table, pml5i, &pte, &current_table)) return;
    }

    if (!pte_walk(current_table, pml4i, &pte, &current_table)) return;

    if (!pte_walk(current_table, pml3i, &pte, &current_table)) return;
    if (*pte & X86_64_PTE_PS) {
        if (page_size != PAGE_SIZE_GIANT) panic("attempted to unmap with wrong page size");
        *pte = 0; asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
        return;
    }

    if (!pte_walk(current_table, pml2i, &pte, &current_table)) return;
    if (*pte & X86_64_PTE_PS) {
        if (page_size != PAGE_SIZE_LARGE) panic("attempted to unmap with  wrong page size");
        *pte = 0; asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
        return;
    }

    if (!pte_walk(current_table, pml1i, &pte, &current_table)) return;

    *pte = 0; asm volatile("invlpg (%0)" ::"r"(vaddr) : "memory");
}

uint64_t paging_get_pte(uint64_t page_table, uint64_t vaddr, int* size_out) {
    uint64_t current_table = page_table;
    uint64_t *current_pte = NULL;

    uint16_t pml1i = (vaddr >> 12) & 0x1ff;
    uint16_t pml2i = (vaddr >> 21) & 0x1ff;
    uint16_t pml3i = (vaddr >> 30) & 0x1ff;
    uint16_t pml4i = (vaddr >> 39) & 0x1ff;

    if (is_5_level_paging) {
        uint16_t pml5i = (vaddr >> 48) & 0x1ff;
        if (!pte_walk(current_table, pml5i, &current_pte, &current_table)) return 0;
    }

    if (!pte_walk(current_table, pml4i, &current_pte, &current_table)) return 0;
    if (!pte_walk(current_table, pml3i, &current_pte, &current_table)) return 0;

    if (*current_pte & X86_64_PTE_PS) { if (size_out) *size_out = PAGE_SIZE_GIANT; return *current_pte; }
    if (!pte_walk(current_table, pml2i, &current_pte, &current_table)) return 0;

    if (*current_pte & X86_64_PTE_PS) { if (size_out) *size_out = PAGE_SIZE_LARGE; return *current_pte; }
    if (!pte_walk(current_table, pml1i, &current_pte, &current_table)) return 0;

    if (size_out) *size_out = PAGE_SIZE_NORM;
    return *current_pte;
}

uint64_t paging_get_paddr(uint64_t page_table, uint64_t vaddr) {
    int size;
    uint64_t pte = paging_get_pte(page_table, vaddr, &size);
    if (!pte) return 0;
    switch (size) {
        case PAGE_SIZE_GIANT: return pte & 0x000FFFFFC0000000ULL;
        case PAGE_SIZE_LARGE: return pte & 0x000FFFFFFFE00000ULL;
        case PAGE_SIZE_NORM : return pte & 0x000FFFFFFFFFF000ULL;
    }
    panic("paging_get_paddr() got bad response from paging_get_pte()");
}
