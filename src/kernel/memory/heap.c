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

void setup_heap(void) {
    if (heap_start != NULL) {
        return;
    }

    heap_start = (void *)KERNEL_HEAP_START;
    heap_size = KERNEL_HEAP_INITIAL_SIZE;

    for (size_t i = 0; i < INITIAL_HEAP_PAGES; i++) {
        void *phys_page = allocate_page();
        if (phys_page == NULL) {
            panic("Heap: failed to allocate page", 0, 0, 0, 0);
        }
        uintptr_t virt_addr = KERNEL_HEAP_START + (i * PAGE_SIZE);
        if (!vmm_map_page(kernel_pagemap, virt_addr, (uintptr_t)phys_page, PTE_PRESENT | PTE_WRITABLE | PTE_NX)) {
            panic("Heap: failed to map page", 0, 0, 0, 0);
        }
    }

    free_list_head = (heap_free_block_t *)heap_start;
    free_list_head->size = heap_size;
    free_list_head->next = NULL;
}

void *kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    size_t total_size = ALIGN_UP_HEAP(size + sizeof(size_t));
    if (total_size < MIN_ALLOC_SIZE) {
        total_size = MIN_ALLOC_SIZE;
    }

    heap_free_block_t *prev = NULL;
    heap_free_block_t *curr = free_list_head;

    while (curr != NULL) {
        if (curr->size >= total_size) {
            if (curr->size - total_size >= MIN_ALLOC_SIZE) {
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

            void *user_ptr = (void *)((uintptr_t)curr + sizeof(size_t));

            return user_ptr;
        }
        prev = curr;
        curr = curr->next;
    }

    printf("Heap: out of heap memory\n");
    return NULL;
}

void kfree(void *ptr) {
    if (ptr == NULL) return;

    size_t *size_ptr = (size_t *)((uintptr_t)ptr - sizeof(size_t));
    void *block_start = (void *)size_ptr;
    size_t block_size = *size_ptr;

    if ((uintptr_t)block_start < (uintptr_t)heap_start || (uintptr_t)block_start >= (uintptr_t)heap_start + heap_size || block_size < MIN_ALLOC_SIZE || ((uintptr_t)block_start % HEAP_ALIGNMENT) != 0) {
        panic("Heap: nvalid pointer or heap corruption detected in kfree", 0, 0, 0, 0);
        return;
    }

    heap_free_block_t *prev = NULL;
    heap_free_block_t *curr = free_list_head;

    while (curr != NULL && (uintptr_t)curr < (uintptr_t)block_start) {
        prev = curr;
        curr = curr->next;
    }

    heap_free_block_t *freed_block = (heap_free_block_t *)block_start;
    freed_block->size = block_size;

    if (prev == NULL) {
        freed_block->next = free_list_head;
        free_list_head = freed_block;
    } else {
        freed_block->next = prev->next;
        prev->next = freed_block;
    }

    if (freed_block->next != NULL && (uintptr_t)freed_block + freed_block->size == (uintptr_t)freed_block->next) {
        freed_block->size += freed_block->next->size;
        freed_block->next = freed_block->next->next;
    }

    if (prev != NULL && (uintptr_t)prev + prev->size == (uintptr_t)freed_block) {
        prev->size += freed_block->size;
        prev->next = freed_block->next;
    }
}

void *kcalloc(size_t num, size_t size) {
    size_t total = num * size;
    if (size != 0 && total / size != num) {
        return NULL;
    }
    void *ptr = kmalloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void *krealloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return kmalloc(size);
    }
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }

    size_t old_size = *((size_t *)((uintptr_t)ptr - sizeof(size_t))) - sizeof(size_t);
    if (size <= old_size) {
        return ptr;
    }

    void *new_ptr = kmalloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, old_size);
        kfree(ptr);
    }
    return new_ptr;
}
