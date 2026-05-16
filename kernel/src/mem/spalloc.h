#include <mem/pmm.h>
#include <stdint.h>
#include <utils/dstruct/llist.h>
#include <utils/locks/ticketlock.h>

typedef struct spalloc_allocator {
    llist_t partial_list;
    int obj_size;
    int obj_alignment;
    uint64_t word0_initial_state;
    uint64_t word1_initial_state;
    ticketlock_t lock;
} spalloc_allocator_t;

typedef struct spalloc_page_header {
    llist_node_t llnode;
    uint64_t bmap[2];
} spalloc_page_header_t;

bool spalloc_init(spalloc_allocator_t *alloc, int obj_size, int obj_align);
void* spalloc_malloc(spalloc_allocator_t* alloc);
void spalloc_free(spalloc_allocator_t* alloc, void* obj);
