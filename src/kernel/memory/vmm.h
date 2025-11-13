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

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <utils/globals.h>

#define PAGE_SIZE 4096

#define PTE_PRESENT (1ull << 0)
#define PTE_WRITABLE (1ull << 1)
#define PTE_USER (1ull << 2)
#define PTE_PWT (1ull << 3)
#define PTE_PCD (1ull << 4)
#define PTE_ACCESSED (1ull << 5)
#define PTE_DIRTY (1ull << 6)
#define PTE_PAT (1ull << 7)
#define PTE_GLOBAL (1ull << 8)
#define PTE_NX (1ull << 63)

#define PTE_ADDR_MASK 0x000ffffffffff000
#define PTE_GET_ADDR(VALUE) ((VALUE) & PTE_ADDR_MASK)
#define PTE_GET_FLAGS(VALUE) ((VALUE) & ~PTE_ADDR_MASK)

typedef struct
{
    uint64_t *top_level;
} pagemap_t;

extern pagemap_t *kernel_pagemap;

#define VMM_HIGHER_HALF (hhdm_request.response->offset)

void setup_vmm();

void vmm_switch_to (pagemap_t *pagemap);

bool vmm_map_page (pagemap_t *pagemap, uintptr_t virt_addr, uintptr_t phys_addr, uint64_t flags);

bool vmm_unmap_page (pagemap_t *pagemap, uintptr_t virt_addr);

uintptr_t vmm_virt_to_phys (pagemap_t *pagemap, uintptr_t virt_addr);
