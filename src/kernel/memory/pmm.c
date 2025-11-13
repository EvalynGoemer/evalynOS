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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <utils/globals.h>
#include <utils/panic.h>
#include <memory/vmm.h>

#define ALIGN_UP(value, align) (((value) + (align) - 1) & ~((align) - 1))

extern uint8_t _text_start[], _text_end[];
extern uint8_t _rodata_start[], _rodata_end[];
extern uint8_t _data_start[], _data_end[];
extern uint8_t _bss_start[], _bss_end[];

#define PMM_BITMAP_SIZE (1024 * 1024)
static uint8_t memory_bitmap[PMM_BITMAP_SIZE];

static size_t highest_page = 0;
static size_t used_pages = 0;
static size_t free_pages = 0;
static size_t total_ram_pages = 0;

#define BITMAP_GET(index) (memory_bitmap[(index) / 8] & (1 << ((index) % 8)))
#define BITMAP_SET(index) (memory_bitmap[(index) / 8] |= (1 << ((index) % 8)))
#define BITMAP_CLEAR(index) (memory_bitmap[(index) / 8] &= ~(1 << ((index) % 8)))

size_t get_total_memory() { return total_ram_pages * PAGE_SIZE; }

size_t get_used_memory() {
    size_t used = used_pages;
    return used * PAGE_SIZE;
}

size_t get_free_memory() {
    size_t free_count = free_pages;
    return free_count * PAGE_SIZE;
}

void setup_pmm() {
    if (memmap_request.response == NULL) {
        panic("Could not acquire memory map response", 0, 0, 0, 0);
    }
    if (executable_address_request.response == NULL) {
        panic("Could not acquire kernel address response", 0, 0, 0, 0);
    }

    size_t entry_count = memmap_request.response->entry_count;
    struct limine_memmap_entry **entries = memmap_request.response->entries;
    uintptr_t highest_addr = 0;
    for (size_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];
        uintptr_t top = entry->base + entry->length;
        if (top > highest_addr) {
            highest_addr = top;
        }
    }

    highest_page = highest_addr / PAGE_SIZE;

    size_t max_bitmap_pages = PMM_BITMAP_SIZE * 8;
    if (highest_page >= max_bitmap_pages) {
        highest_page = max_bitmap_pages - 1;
    }

    memset(memory_bitmap, 0xFF, PMM_BITMAP_SIZE);

    total_ram_pages = 0;
    for (size_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            uintptr_t base = ALIGN_UP(entry->base, PAGE_SIZE);
            uintptr_t top = (entry->base + entry->length) & ~(PAGE_SIZE - 1);

            if (top <= base) continue;

            size_t first_page = base / PAGE_SIZE;
            size_t last_page = top / PAGE_SIZE;

            for (size_t page_idx = first_page; page_idx < last_page; ++page_idx) {
                if (page_idx <= highest_page) {
                    if (BITMAP_GET(page_idx)) {
                        BITMAP_CLEAR(page_idx);
                        total_ram_pages++;
                    }
                }
            }
        }
    }

    free_pages = total_ram_pages;

    struct limine_executable_address_response *kaddr = executable_address_request.response;
    uintptr_t k_phys_start = kaddr->physical_base;
    uintptr_t k_virt_start = (uintptr_t)_text_start;
    uintptr_t k_virt_end = (uintptr_t)_bss_end;
    uintptr_t k_size = k_virt_end - k_virt_start;
    uintptr_t k_phys_end = k_phys_start + ALIGN_UP(k_size, PAGE_SIZE);

    size_t k_first_page = k_phys_start / PAGE_SIZE;
    size_t k_last_page = k_phys_end / PAGE_SIZE;

    for (size_t page_idx = k_first_page; page_idx < k_last_page; ++page_idx) {
        if (page_idx <= highest_page) {
            if (!BITMAP_GET(page_idx)) {
                free_pages--;
            }
            BITMAP_SET(page_idx);
        }
    }

    uintptr_t bitmap_virt_start = (uintptr_t)memory_bitmap;
    uintptr_t bitmap_phys_start = bitmap_virt_start - kaddr->virtual_base + kaddr->physical_base;
    uintptr_t bitmap_phys_end = bitmap_phys_start + PMM_BITMAP_SIZE;
    size_t bitmap_first_page = bitmap_phys_start / PAGE_SIZE;
    size_t bitmap_last_page = (bitmap_phys_end + PAGE_SIZE - 1) / PAGE_SIZE;

    for (size_t page_idx = bitmap_first_page; page_idx < bitmap_last_page; ++page_idx) {
        if (page_idx <= highest_page) {
            if (!BITMAP_GET(page_idx)) {
                free_pages--;
            }
            BITMAP_SET(page_idx);
        }
    }

    size_t pages_below_1mb = 0x100000 / PAGE_SIZE;
    for (size_t page_idx = 0; page_idx < pages_below_1mb; ++page_idx) {
        if (page_idx <= highest_page) {
            if (!BITMAP_GET(page_idx)) {
                free_pages--;
            }
            BITMAP_SET(page_idx);
        }
    }

    used_pages = total_ram_pages - free_pages;
}

void *allocate_page() {
    for (size_t i = 0; i <= highest_page; ++i) {
        if (!BITMAP_GET(i)) {
            BITMAP_SET(i);
            used_pages++;
            free_pages--;

            void *addr = (void *)((uintptr_t)i * PAGE_SIZE);
            void *vaddr = (void *)((uintptr_t)addr + VMM_HIGHER_HALF);

            memset(vaddr, 0, PAGE_SIZE);

            return addr;
        }
    }

    printf("Kernel: Out of physical memory!\n");
    return NULL;
}

void free_page(void *page) {
    if (page == NULL) {
        printf("Kernel: PMM free_page called with NULL pointer\n");
        asm volatile("" ::: "memory");
        return;
    }

    uintptr_t addr = (uintptr_t)page;

    if (addr % PAGE_SIZE != 0) {
        printf("Kernel: PMM free_page called with non-page-aligned physical address %lx\n", (uint64_t)page);
        return;
    }

    size_t index = addr / PAGE_SIZE;

    if (index > highest_page) {
        printf("Kernel: PMM free_page called with physical address %lx outside managed range (max index %lu)\n", (uint64_t)page, highest_page);
        return;
    }

    if (!BITMAP_GET(index)) {
        printf("Kernel: PMM free_page called on already free page %lx (index %lu)\n", (uint64_t)page, index);
        return;
    }

    BITMAP_CLEAR(index);
    used_pages--;
    free_pages++;
}

bool is_page_free(uintptr_t paddr) {
    size_t index = paddr / PAGE_SIZE;
    if (index > highest_page) {
        return false;
    }

    return !BITMAP_GET(index);
}
