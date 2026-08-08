#include <arch/generic/panic.h>
#include <stdint.h>
#include <stdio.h>

#include <arch/intrin/paging.h>
#include <arch/generic/paging/paging.h>
#include <loader/elf_structs.h>
#include <loader/elf_introspection.h>
#include <mem/pmm.h>
#include <utils/limine.h>
#include <utils/lib.h>

uint64_t kernel_page_table;

uint64_t VADDR_LOWER_HALF_TOP = 0;
uint64_t VADDR_HIGHER_HALF_BASE = 0;

uint32_t mmu_config = 0;

/* Internal Helpers */
static inline uint64_t get_next_level_and_allocate(uint64_t pte_phys, MAYBE_UNUSED int perms) {
    uint64_t* vpte = TO_HHDM_PTR(pte_phys);
    uint64_t entry = *vpte;

    if (!ARCH_PTE_PRESENT(entry)) {
        uint64_t new_table = pmm_alloc_page();
        entry = ARCH_ENCODE_PTE(new_table, ARCH_INTERMEDIATE_MMU_FLAGS(perms));
        *vpte = entry;
    }

    return ARCH_DECODE_PTE(entry);
}

static inline void write_leaf(uint64_t pte_phys, uint64_t paddr, uint64_t vaddr, int perms, int page_size) {
    uint64_t *pte_virt = TO_HHDM_PTR(pte_phys);
    uint64_t flags = prot_to_mmu_flags(perms);
    if (page_size != PAGE_NORM) flags = arch_large_page_fixup(flags);
    *pte_virt = ARCH_ENCODE_PTE(paddr, flags);
    arch_tlb_flush(vaddr);
}

/* Public Functions */

void paging_init() {
    mmu_config = arch_get_mmu_config();
    VADDR_LOWER_HALF_TOP = paging_get_vaddr_lower_half_top();
    VADDR_HIGHER_HALF_BASE = paging_get_vaddr_higher_half_base();

    switch (mmu_config & MMU_CONFIG_LVLS_MASK) {
        case 5: LOG_TAGGED("MEMORY", ANSI_BGREEN, "System is using 5 Level Paging") break;
        case 4: LOG_TAGGED("MEMORY", ANSI_BGREEN, "System is using 4 Level Paging") break;
        case 3: LOG_TAGGED("MEMORY", ANSI_BGREEN, "System is using 3 Level Paging") break;
        default: UNREACHABLE();
    }

    if (mmu_config & MMU_CONFIG_NX)
        LOG_TAGGED("MEMORY", ANSI_BGREEN, "System supports non executable pages")
    else
        LOG_TAGGED("MEMORY", ANSI_BGREEN, "System lacks support for non executable pages")

    if (mmu_config & MMU_CONFIG_L2_LEAF)
        LOG_TAGGED("MEMORY", ANSI_BGREEN, "System supports large pages")
    if (mmu_config & MMU_CONFIG_L3_LEAF)
        LOG_TAGGED("MEMORY", ANSI_BGREEN, "System supports giant pages")

    kernel_page_table = pmm_alloc_page();

#ifndef ARCH_SEPARATE_PAGING_ROOTS
    // pre allocate page tables for upper half so cloning tables is simpler
    for (int i = 256; i < 512; i++) {
        uint64_t pte_phys = kernel_page_table + (i * sizeof(uint64_t));
        get_next_level_and_allocate(pte_phys, PAGE_KRWX);
    }
#endif

#ifndef ARCH_UNPAGED_HHDM
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
            if ((mmu_config & MMU_CONFIG_L3_LEAF) && addr + PAGE_SIZE_GIANT <= end && IS_ALIGNED(addr, PAGE_SIZE_GIANT)) {
                paging_map_page(kernel_page_table, TO_HHDM(addr), addr, flags, PAGE_GIANT);
                addr += PAGE_SIZE_GIANT;
                continue;
            }

            if ((mmu_config & MMU_CONFIG_L2_LEAF) && addr + PAGE_SIZE_LARGE <= end && IS_ALIGNED(addr, PAGE_SIZE_LARGE)) {
                paging_map_page(kernel_page_table, TO_HHDM(addr), addr, flags, PAGE_LARGE);
                addr += PAGE_SIZE_LARGE;
                continue;
            }

            paging_map_page(kernel_page_table, TO_HHDM(addr), addr, flags, PAGE_NORM);
            addr += PAGE_SIZE;
        }
    }
#endif

    LOG_TAGGED("MEMORY", ANSI_BGREEN, "Mapping Kernel ELF")
    struct limine_executable_address_response *kaddr = executable_address_request.response;
    struct limine_executable_file_response *kexec = executable_file_request.response;
    struct elf_header_64 *header = (struct elf_header_64 *)kexec->executable_file->address;
    struct elf_program_header_64 *prog_headers = (struct elf_program_header_64 *)((uint8_t *)kexec->executable_file->address + header->program_header_table);

    uint64_t elf_base = elf_introspect_base();

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
            paging_map_page(kernel_page_table, vaddr, paddr, flags, PAGE_NORM);
        }
    }

    LOG_TAGGED("MEMORY", ANSI_BGREEN, "Swapping to new page tables")
    arch_load_page_table(kernel_page_table);
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
        case PAGE_NORM: check_align(vaddr, paddr, PAGE_SIZE, "tried to map a normal page with poor alignment"); break;
        case PAGE_LARGE: check_align(vaddr, paddr, PAGE_SIZE_LARGE, "tried to map a large page with poor alignment"); break;
        case PAGE_GIANT: check_align(vaddr, paddr, PAGE_SIZE_GIANT, "tried to map a giant page with poor alignment"); break;
    }

    uint64_t current_table = page_table;
    uint64_t current_pte_phys;

    if (mmu_config & MMU_CONFIG_L5) {
        uint16_t pml5i = GET_PML5i(vaddr);
        current_pte_phys = current_table + (pml5i * sizeof(uint64_t));
        current_table = get_next_level_and_allocate(current_pte_phys, PAGE_URWX);
    }

    if (mmu_config & MMU_CONFIG_L4) {
        uint16_t pml4i = GET_PML4i(vaddr);
        current_pte_phys = current_table + (pml4i * sizeof(uint64_t));
        current_table = get_next_level_and_allocate(current_pte_phys, PAGE_URWX);
    }

    uint16_t pml3i = GET_PML3i(vaddr);
    current_pte_phys = current_table + (pml3i * sizeof(uint64_t));
    if (page_size == PAGE_GIANT) {
        write_leaf(current_pte_phys, paddr, vaddr, perms, PAGE_GIANT);
        return;
    }
    current_table = get_next_level_and_allocate(current_pte_phys, PAGE_URWX);

    uint16_t pml2i = GET_PML2i(vaddr);
    current_pte_phys = current_table + (pml2i * sizeof(uint64_t));
    if (page_size == PAGE_LARGE) {
        write_leaf(current_pte_phys, paddr, vaddr, perms, PAGE_LARGE);
        return;
    }
    current_table = get_next_level_and_allocate(current_pte_phys, PAGE_URWX);

    uint16_t pml1i = GET_PML1i(vaddr);
    current_pte_phys = current_table + (pml1i * sizeof(uint64_t));
    write_leaf(current_pte_phys, paddr, vaddr, perms, PAGE_NORM);
}
