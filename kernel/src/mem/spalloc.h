#pragma once
#include <stdint.h>
#include <stddef.h>
#include <utils/dstruct/stack.h>
#include <utils/dstruct/llist.h>
#include <utils/locks/ticketlock.h>

typedef struct spalloc_page_header {
    uint32_t     magic;
    uint16_t     freecount;
    stack_t      free_objs;
    llist_node_t linkage;
} spalloc_page_header_t;

typedef struct spalloc_allocator {
    ticketlock_t lock;
    llist_t    partial_list;
    uint16_t   obj_stride;
    uint16_t   obj_offset;
    uint16_t   objs_per_page;
} spalloc_allocator_t;

void spalloc_init(spalloc_allocator_t* alloc, size_t obj_size, size_t obj_align);

[[gnu::malloc]] void* spalloc_malloc(spalloc_allocator_t* alloc);
void spalloc_free(spalloc_allocator_t* alloc, void* obj);
