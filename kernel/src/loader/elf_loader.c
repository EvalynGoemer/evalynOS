#include <loader/elf.h>
#include <loader/elf_structs.h>
#include <arch/generic/paging/paging.h>
#include <sched/scheduler.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mem/address_space.h"
#include "mem/pmm.h"
#include "mem/vmem.h"
#include "utils/lib.h"

uint64_t load_elf(void *file, address_space_t* addrspace) {
    if (!verify_elf(file))
        return 0;

    struct elf_header_64* header = (struct elf_header_64*)file;
    struct elf_program_header_64* prog_headers = (struct elf_program_header_64*)(file + header->program_header_table);

    for (size_t i = 0; i < header->program_header_entries; i++) {
        struct elf_program_header_64* ph = &prog_headers[i];

        if (ph->type != ELF_PROG_PT_LOAD_TYPE) continue;

        uint64_t start = ph->virt_addr;
        uint64_t end = start + ph->mem_size;
        uint64_t aligned_start = ALIGN_DOWN(start, PAGE_SIZE);
        uint64_t aligned_end = ALIGN_UP(end, PAGE_SIZE);

        uint64_t pte_flags = PAGE_URX;
        if (ph->flags & ELF_PROG_WRITE) pte_flags |= PAGE_W;
        if (!(ph->flags & ELF_PROG_EXEC_FLAG)) pte_flags &= ~PAGE_X;

        uint64_t vaddr_offset = ph->virt_addr - aligned_start;
        uint64_t pagesToMap = (aligned_end - aligned_start) / PAGE_SIZE;

        if (!vmem_alloc(&addrspace->valloc, aligned_end - aligned_start, aligned_start))
            return 0;

        uint64_t fileRemaining = ph->file_size;
        uint8_t* src = (uint8_t *)file + ph->offset;

        for (uint64_t p = 0; p < pagesToMap; p++) {
            uint64_t ppage = pmm_alloc_page();
            uint64_t vpage = ppage + hhdm_request.response->offset;
            paging_map_page(addrspace->pagetable, aligned_start, ppage, pte_flags);

            uint64_t offset = (p == 0) ? vaddr_offset : 0;
            uint64_t space = PAGE_SIZE - offset;
            uint64_t toCopy = fileRemaining < space ? fileRemaining : space;
            if (offset) memset((void*)vpage, 0, offset);
            if (toCopy) memcpy((void*)(vpage + offset), src, toCopy);
            if (space > toCopy) memset((void*)(vpage + offset + toCopy), 0, space - toCopy);

            src += toCopy;
            fileRemaining -= toCopy;
            aligned_start += PAGE_SIZE;
        }
    }

    return header->entry_point;
}
