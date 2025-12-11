/*
 * Arikoto
 * Copyright (c) 2025
 * Licensed under the NCSA/University of Illinois Open Source License; see the
 * following licence text
 *
 * NCSA/University of Illinois Open Source License
 *
 * Copyright (c) 2025 NerdNextDoor
 * All rights reserved.
 *
 * Developed by: Arikoto Operating System Development Project
 * https://arikoto.nerdnextdoor.net, https://codeberg.org/NerdNextDoor/arikoto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimers. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimers in the documentation and/or other
 * materials provided with the distribution. Neither the names of the Arikoto
 * Operating System Development Project, NerdNextDoor, nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * Software without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH
 * THE SOFTWARE.
 */

// Modified by Evalyn Goemer to work with EvalynOS

#define ALIGN_UP(value, align) (((value) + (align) - 1) & ~((align) - 1))

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <drivers/x86_64/cpuid.h>
#include <utils/globals.h>
#include <utils/panic.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <stdlib.h>

extern uint8_t _text_start[], _text_end[];
extern uint8_t _rodata_start[], _rodata_end[];
extern uint8_t _data_start[], _data_end[];
extern uint8_t _bss_start[], _bss_end[];

pagemap_t *kernel_pagemap = NULL;

static bool vmm_is_table_empty(uint64_t *table_virt) {
    for (int i = 0; i < 512; i++) {
        if (table_virt[i] != 0) {
            return false;
        }
    }
    return true;
}

static uint64_t *vmm_get_next_level(uint64_t *current_level_virt, size_t index, bool allocate, uint64_t alloc_entry_flags) {
    uint64_t entry = current_level_virt[index];

    if (entry & PTE_PRESENT) {
        return (uint64_t *)(PTE_GET_ADDR(entry) + VMM_HIGHER_HALF);
    }

    if (!allocate) {
        return NULL;
    }

    void *next_level_phys = allocate_page();
    if (next_level_phys == NULL) {
        printf("Kernel: Failed to allocate page for new page table level (index %u)\n", (unsigned)index);
        return NULL;
    }

    uint64_t *next_level_virt = (uint64_t *)((uintptr_t)next_level_phys + VMM_HIGHER_HALF);
    memset(next_level_virt, 0, PAGE_SIZE);

    uint64_t new_entry_flags = alloc_entry_flags ? alloc_entry_flags : (PTE_PRESENT | PTE_WRITABLE);

    current_level_virt[index] = (uint64_t)(uintptr_t)next_level_phys | new_entry_flags;

    return next_level_virt;
}

bool vmm_map_page(pagemap_t *pagemap, uintptr_t virt_addr, uintptr_t phys_addr, uint64_t flags) {
    asm volatile("cli");

    virt_addr &= ~(PAGE_SIZE - 1);
    phys_addr &= ~(PAGE_SIZE - 1);

    size_t pml4_index = (virt_addr >> 39) & 0x1FF;
    size_t pdpt_index = (virt_addr >> 30) & 0x1FF;
    size_t pd_index = (virt_addr >> 21) & 0x1FF;
    size_t pt_index = (virt_addr >> 12) & 0x1FF;

    uint64_t alloc_flags = PTE_PRESENT | PTE_WRITABLE;
    if (flags & PTE_USER) alloc_flags |= PTE_USER;

    uint64_t *pml4 = pagemap->top_level;
    uint64_t *pdpt = vmm_get_next_level(pml4, pml4_index, true, alloc_flags);
    if (!pdpt) goto fail;
    uint64_t *pd = vmm_get_next_level(pdpt, pdpt_index, true, alloc_flags);
    if (!pd) goto fail;
    uint64_t *pt = vmm_get_next_level(pd, pd_index, true, alloc_flags);
    if (!pt) goto fail;

    pt[pt_index] = phys_addr | flags | PTE_PRESENT;

    asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");

    asm volatile("sti");
    return true;

    fail:
    printf("Kernel: Failed to map page for virt %p\n", (void *)virt_addr);
    asm volatile("sti");
    return false;
}

bool vmm_unmap_page(pagemap_t *pagemap, uintptr_t virt_addr) {
    if (virt_addr % PAGE_SIZE != 0) {
        printf("Kernel: vmm_unmap_page called with non-aligned virt %p\n", (void *)virt_addr);
        return false;
    }

    size_t pml4_index = (virt_addr >> 39) & 0x1FF;
    size_t pdpt_index = (virt_addr >> 30) & 0x1FF;
    size_t pd_index = (virt_addr >> 21) & 0x1FF;
    size_t pt_index = (virt_addr >> 12) & 0x1FF;

    uint64_t *pml4 = pagemap->top_level;
    uint64_t *pdpt = vmm_get_next_level(pml4, pml4_index, false, 0);
    if (!pdpt) {
        return true;
    }
    uint64_t pdpt_entry = pml4[pml4_index];

    uint64_t *pd = vmm_get_next_level(pdpt, pdpt_index, false, 0);
    if (!pd) {
        return true;
    }
    uint64_t pd_entry = pdpt[pdpt_index];

    uint64_t *pt = vmm_get_next_level(pd, pd_index, false, 0);
    if (!pt) {
        return true;
    }
    uint64_t pt_entry = pd[pd_index];

    if (!(pt[pt_index] & PTE_PRESENT)) {
        return true;
    }

    pt[pt_index] = 0;

    asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");

    if (vmm_is_table_empty(pt)) {
        pd[pd_index] = 0;

        uintptr_t pt_phys = PTE_GET_ADDR(pt_entry);
        free_page((void *)pt_phys);

        if (vmm_is_table_empty(pd)) {
            pdpt[pdpt_index] = 0;

            uintptr_t pd_phys = PTE_GET_ADDR(pd_entry);
            free_page((void *)pd_phys);

            if (vmm_is_table_empty(pdpt)) {
                if (pml4_index >= 256) {
                    return true;
                }
                pml4[pml4_index] = 0;
                uintptr_t pdpt_phys = PTE_GET_ADDR(pdpt_entry);
                free_page((void *)pdpt_phys);
            }
        }
    }

    return true;
}

uintptr_t vmm_virt_to_phys(pagemap_t *pagemap, uintptr_t virt_addr) {
    size_t pml4_index = (virt_addr >> 39) & 0x1FF;
    size_t pdpt_index = (virt_addr >> 30) & 0x1FF;
    size_t pd_index = (virt_addr >> 21) & 0x1FF;
    size_t pt_index = (virt_addr >> 12) & 0x1FF;

    uint64_t *pml4 = pagemap->top_level;
    uint64_t *pdpt = vmm_get_next_level(pml4, pml4_index, false, 0);
    if (pdpt == NULL) {
        return (uintptr_t)-1;
    }

    uint64_t *pd = vmm_get_next_level(pdpt, pdpt_index, false, 0);
    if (pd == NULL) {
        return (uintptr_t)-1;
    }

    uint64_t *pt = vmm_get_next_level(pd, pd_index, false, 0);
    if (pt == NULL) {
        return (uintptr_t)-1;
    }

    uint64_t entry = pt[pt_index];

    if (!(entry & PTE_PRESENT)) {
        return (uintptr_t)-1;
    }

    uintptr_t phys_addr_base = PTE_GET_ADDR(entry);
    uintptr_t offset = virt_addr % PAGE_SIZE;

    return phys_addr_base + offset;
}

void vmm_switch_to(pagemap_t *pagemap) {
    if (!pagemap || !pagemap->top_level) {
        panic("Attempted to switch to an invalid pagemap\n", NULL);
        return;
    }

    uintptr_t pml4_phys = (uintptr_t)pagemap->top_level - VMM_HIGHER_HALF;

    asm volatile("mov %0, %%cr3" ::"r"(pml4_phys) : "memory");
}

void setup_vmm() {
    if (hhdm_request.response == NULL) {
        panic("HHDM request response missing\n", NULL);
    }
    if (executable_address_request.response == NULL) {
        panic("Kernel Address request response missing\n", NULL);
    }
    if (memmap_request.response == NULL) {
        panic("Memory Map request response missing\n", NULL);
    }

    void *pml4_phys = allocate_page();
    if (pml4_phys == NULL) {
        panic("Failed to allocate kernel PML4 table page\n", NULL);
    }
    uint64_t *pml4_virt = (uint64_t *)((uintptr_t)pml4_phys + VMM_HIGHER_HALF);
    memset(pml4_virt, 0, PAGE_SIZE);

    static pagemap_t k_pagemap;
    kernel_pagemap = &k_pagemap;
    kernel_pagemap->top_level = pml4_virt;

    for (int i = 256; i <= 511; i++) {
        vmm_get_next_level(pml4_virt, i, true, PTE_DUMMY);
    }

    struct limine_executable_address_response *kaddr = executable_address_request.response;
    uintptr_t kernel_phys_base = kaddr->physical_base;
    uintptr_t kernel_virt_base = kaddr->virtual_base;

    uintptr_t text_start_addr = (uintptr_t)_text_start;
    uintptr_t text_end_addr = (uintptr_t)_text_end;
    uintptr_t rodata_start_addr = (uintptr_t)_rodata_start;
    uintptr_t rodata_end_addr = (uintptr_t)_rodata_end;
    uintptr_t data_start_addr = (uintptr_t)_data_start;
    uintptr_t data_end_addr = ALIGN_UP((uintptr_t)_bss_end, PAGE_SIZE);

    uintptr_t kernel_virt_end = data_end_addr;

    for (uintptr_t p_virt = kernel_virt_base; p_virt < kernel_virt_end; p_virt += PAGE_SIZE) {
        uintptr_t p_phys = (p_virt - kernel_virt_base) + kernel_phys_base;
        uint64_t flags = PTE_PRESENT;

        if (p_virt >= text_start_addr && p_virt < text_end_addr) {
        } else if (p_virt >= rodata_start_addr && p_virt < rodata_end_addr) {
            flags |= PTE_NX;
        } else if (p_virt >= data_start_addr && p_virt < data_end_addr) {
            flags |= PTE_WRITABLE | PTE_NX;
        } else {
            flags |= PTE_WRITABLE | PTE_NX;
        }

        if (!vmm_map_page(kernel_pagemap, p_virt, p_phys, flags)) {
            panic("Failed to map kernel page\n", NULL);
        }
    }

    struct limine_memmap_response *memmap = memmap_request.response;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        uintptr_t base = entry->base;
        uintptr_t top = base + entry->length;
        uintptr_t map_base = ALIGN_UP(base, PAGE_SIZE);
        uintptr_t map_top = top & ~(PAGE_SIZE - 1);

        if (map_top <= map_base) continue;

        for (uintptr_t p = map_base; p < map_top; p += PAGE_SIZE) {
            if (!vmm_map_page(kernel_pagemap, p + VMM_HIGHER_HALF, p, PTE_PRESENT | PTE_WRITABLE | PTE_NX)) {
                panic("Failed to map HHDM page", NULL);
            }
        }
    }

    // remap frame buffer as write combining if on phsyical hardware
    if (!cpu_feature_bit(1, 0, 'c', CPUID_HYPERVISOR)) {
        printf("Kernel: Running on physical machine; Enabling WC on Framebuffer\n");
        uint32_t total_bytes = framebuffer->pitch * framebuffer->height;
        uint16_t pages = (total_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
        uint64_t virt_addr = (uint64_t)framebuffer->address;
        uint64_t phys_addr = ((uint64_t)framebuffer->address - hhdm_request.response->offset);
        for (uint16_t i = 0; i < pages; i++) {
            vmm_map_page(kernel_pagemap, virt_addr, phys_addr, PTE_PRESENT | PTE_WRITABLE | PTE_PCD | PTE_PAT);
            asm volatile("invlpg (%0)" :: "r"(virt_addr) : "memory");
            virt_addr += 0x1000;
            phys_addr += 0x1000;
        }
    } else {
        printf("Kernel: Running on virtual machine; Not Enabling WC on Framebuffer\n");
    }


    vmm_switch_to(kernel_pagemap);
}

pagemap_t *new_pagemap() {
    void *pml4_phys = allocate_page();
    if (pml4_phys == NULL) {
        panic("Failed to allocate new PML4 table page\n", NULL);
    }
    uint64_t *pml4_virt = (uint64_t *)((uintptr_t)pml4_phys + VMM_HIGHER_HALF);
    memcpy(pml4_virt, kernel_pagemap->top_level, PAGE_SIZE);

    memset(pml4_virt, 0, PAGE_SIZE / 2);

    pagemap_t *new_pagemap = malloc(sizeof(pagemap_t));
    new_pagemap->top_level = pml4_virt;
    return new_pagemap;
}
