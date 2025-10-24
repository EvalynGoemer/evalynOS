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

#include <stddef.h>
#include <stdint.h>

#include "../kernel.h"
#include "../libc/stdio.h"
#include "../libc/string.h"
#include "../panic.h"
#include "pmm.h"
#include "vmm.h"

typedef struct heap_free_block {
    size_t size;
    struct heap_free_block *next;
} heap_free_block_t;

#define MIN_ALLOC_SIZE sizeof(heap_free_block_t)
#define HEAP_ALIGNMENT 16

#define ALIGN_UP_HEAP(size) (((size) + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1))

#define KERNEL_HEAP_START 0xFFFF810000000000
#define INITIAL_HEAP_PAGES 256
#define KERNEL_HEAP_INITIAL_SIZE (INITIAL_HEAP_PAGES * PAGE_SIZE)

static void *heap_start = NULL;
static size_t heap_size = 0;
static heap_free_block_t *free_list_head = NULL;

int heap_expand_pages(size_t pages) {
    if (pages == 0) return 0;

    uintptr_t base = (uintptr_t)heap_start + heap_size;
    uintptr_t new_region_size = pages * PAGE_SIZE;

    for (size_t i = 0; i < pages; ++i) {
        void *phys = allocate_page();
        if (!phys) {
            printf("heap_expand_pages: allocate_page failed at page %lx\n", i);
            return 0;
        }
        uintptr_t virt = base + (i * PAGE_SIZE);
        if (!vmm_map_page(kernel_pagemap, virt, (uintptr_t)phys, PTE_PRESENT | PTE_WRITABLE | PTE_NX)) {
            panic("heap_expand_pages: vmm_map_page failed", 0, 0, 0, 0);
            return 0;
        }
    }

    heap_free_block_t *new_block = (heap_free_block_t *)base;
    new_block->size = new_region_size;
    new_block->next = NULL;

    if (!free_list_head) {
        free_list_head = new_block;
    } else {
        heap_free_block_t *p = NULL;
        heap_free_block_t *c = free_list_head;
        while (c && (uintptr_t)c < base) {
            p = c;
            c = c->next;
        }
        if (p == NULL) {
            new_block->next = free_list_head;
            free_list_head = new_block;
        } else {
            new_block->next = p->next;
            p->next = new_block;
        }

        if (new_block->next && ((uintptr_t)new_block + new_block->size) == (uintptr_t)new_block->next) {
            new_block->size += new_block->next->size;
            new_block->next = new_block->next->next;
        }
        if (p && ((uintptr_t)p + p->size) == (uintptr_t)new_block) {
            p->size += new_block->size;
            p->next = new_block->next;
        }
    }

    heap_size += new_region_size;
    return 1;
}

void setup_heap(void) {
    if (heap_start != NULL) {
        return;
    }

    heap_start = (void *)KERNEL_HEAP_START;
    heap_size = 0;

    if (!heap_expand_pages(INITIAL_HEAP_PAGES)) {
        panic("init_heap: failed to allocate initial kernel heap", 0, 0, 0, 0);
    }

    free_list_head = (heap_free_block_t *)heap_start;
    free_list_head->size = heap_size;
    free_list_head->next = NULL;

    // printf("Heap installed\n");
}

void heap_dump(void) {
    printf("Heap dump: start=%lx size=%lx free-list:\n", (uint64_t)heap_start, heap_size);
    for (heap_free_block_t *b = free_list_head; b; b = b->next) {
        printf("block %lx size=%lx next=%p\n", (uint64_t)b, b->size, b->next);
    }
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;
    size_t payload = ALIGN_UP_HEAP(size);
    size_t header_sz = ALIGN_UP_HEAP(sizeof(size_t));
    size_t total_size = payload + header_sz;

    if (total_size < MIN_ALLOC_SIZE) total_size = MIN_ALLOC_SIZE;

    heap_free_block_t *prev = NULL;
    heap_free_block_t *curr = free_list_head;

    retry_search:
    while (curr) {
        if (curr->size >= total_size) {
            if (curr->size >= total_size + MIN_ALLOC_SIZE) {
                heap_free_block_t *new_block = (heap_free_block_t *)((uintptr_t)curr + total_size);
                new_block->size = curr->size - total_size;
                new_block->next = curr->next;

                curr->size = total_size;

                if (prev == NULL) {
                    free_list_head = new_block;
                } else {
                    prev->next = new_block;
                }
            } else {
                if (prev == NULL) {
                    free_list_head = curr->next;
                } else {
                    prev->next = curr->next;
                }
            }

            size_t *size_ptr = (size_t *)curr;
            *size_ptr = curr->size;

            void *user_ptr = (void *)((uintptr_t)curr + header_sz);
            return user_ptr;
        }
        prev = curr;
        curr = curr->next;
    }

    if (!heap_expand_pages(16)) {
        if (!heap_expand_pages(1)) {
            printf("kmalloc: Out of heap memory (requested %lx bytes)\n", size);
            heap_dump();
            return NULL;
        }
    }
    prev = NULL;
    curr = free_list_head;
    goto retry_search;
}

void kfree(void *ptr) {
    if (!ptr) return;

    size_t header_sz = ALIGN_UP_HEAP(sizeof(size_t));
    size_t *size_ptr = (size_t *)((uintptr_t)ptr - header_sz);
    void *block_start = (void *)size_ptr;
    size_t block_size = *size_ptr;

    if ((uintptr_t)block_start < (uintptr_t)heap_start ||
        (uintptr_t)block_start >= (uintptr_t)heap_start + heap_size) {
        panic("kfree: invalid pointer (out of heap range)", 0, 0, 0, 0);
    return;
        }
        if (block_size < MIN_ALLOC_SIZE) {
            panic("kfree: invalid block size", 0, 0, 0, 0);
            return;
        }
        if (((uintptr_t)block_start & (HEAP_ALIGNMENT - 1)) != 0) {
            panic("kfree: alignment error", 0, 0, 0, 0);
            return;
        }

        heap_free_block_t *prev = NULL;
        heap_free_block_t *curr = free_list_head;
        while (curr && (uintptr_t)curr < (uintptr_t)block_start) {
            prev = curr;
            curr = curr->next;
        }

        heap_free_block_t *freed = (heap_free_block_t *)block_start;
        freed->size = block_size;

        if (prev == NULL) {
            freed->next = free_list_head;
            free_list_head = freed;
        } else {
            freed->next = prev->next;
            prev->next = freed;
        }

        if (freed->next && ((uintptr_t)freed + freed->size) == (uintptr_t)freed->next) {
            freed->size += freed->next->size;
            freed->next = freed->next->next;
        }
        if (prev && ((uintptr_t)prev + prev->size) == (uintptr_t)freed) {
            prev->size += freed->size;
            prev->next = freed->next;
        }
}

void *kcalloc(size_t num, size_t size) {
    if (size != 0 && num > (SIZE_MAX / size)) return NULL;
    size_t total = num * size;
    void *p = kmalloc(total);
    if (p) memset(p, 0, total);
    return p;
}

void *krealloc(void *ptr, size_t new_size) {
    if (!ptr) return kmalloc(new_size);
    if (new_size == 0) { kfree(ptr); return NULL; }

    size_t header_sz = ALIGN_UP_HEAP(sizeof(size_t));
    size_t old_total = *((size_t *)((uintptr_t)ptr - header_sz));
    size_t old_payload = (old_total >= header_sz) ? (old_total - header_sz) : 0;
    if (new_size <= old_payload) return ptr;

    void *n = kmalloc(new_size);
    if (!n) return NULL;
    memcpy(n, ptr, old_payload);
    kfree(ptr);
    return n;
}
