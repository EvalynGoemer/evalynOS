// inspired by https://github.com/rdmsr/zag/blob/ff180bb385d3d706de73cad93f7d9c5baf791e59/src/mm/vmem.zig
// permission from rdmsr to use with MIT Licence here

#pragma once

#include <stdint.h>
#include <mem/spalloc.h>
#include <utils/dstruct/llist.h>
#include <utils/dstruct/bstree.h>
#include <utils/locks/ticketlock.h>

typedef struct vmem_segment {
    bool allocated;

    uint64_t base;
    uint64_t size;

    llist_node_t segment_list_node;
    union {
        llist_node_t freelist_node;
        bstree_node_t segment_tree_node;
    };
} vmem_segment_t;

typedef struct vmem_allocator {
    uint64_t base;
    uint64_t size;
    uint32_t quantum;
    llist_t freelists[64];
    llist_t segments_list;
    bstree_t segments_tree;

    spalloc_allocator_t segment_allocator;
    ticketlock_t lock;
} vmem_allocator_t;

extern void vmem_init();

extern vmem_allocator_t kernel_vmem_allocator;

extern void vmem_allocator_init(vmem_allocator_t* alloc, uint64_t base, uint64_t size, uint32_t quantum);
extern void vmem_add_segment(vmem_allocator_t* alloc, uint64_t base, uint64_t size);
extern uint64_t vmem_alloc(vmem_allocator_t* alloc, uint64_t size, uint64_t addr);
extern void vmem_free(vmem_allocator_t* alloc, uint64_t addr, uint64_t size);
extern vmem_segment_t* vmem_find_segment(vmem_allocator_t* alloc, uint64_t addr);
