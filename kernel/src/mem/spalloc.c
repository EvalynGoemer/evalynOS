#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <utils/defer.h>
#include <utils/lib.h>
#include <utils/locks/ticketlock.h>
#include <utils/dstruct/llist.h>
#include <mem/buddy.h>
#include <arch/generic/paging/paging.h>
#include <mem/spalloc.h>

#define SPALLOC_MAGIC 0x2d59fe75
#define SPALLOC_PAGE_OF(ptr) ((spalloc_page_header_t*)ALIGN_DOWN((ptr), PAGE_SIZE))

void spalloc_init(spalloc_allocator_t* alloc, size_t obj_size, size_t obj_align) {
    assert(alloc != nullptr);
    assert(obj_size >= 8);
    assert(obj_size  <= (PAGE_SIZE / 4));
    assert(obj_align <= (PAGE_SIZE / 4));
    assert(isPow2ull(obj_align));

    int stride = ALIGN_UP(obj_size, obj_align);
    assert(stride % 8 == 0);

    alloc->obj_stride = stride;
    alloc->obj_offset = ALIGN_UP(sizeof(spalloc_page_header_t), obj_align);
    alloc->objs_per_page = (PAGE_SIZE - alloc->obj_offset) / alloc->obj_stride;
    alloc->partial_list = LLIST_INIT;
    ticketlock_init(&alloc->lock);
}

[[gnu::malloc]]
void* spalloc_malloc(spalloc_allocator_t* alloc) {
    ticketlock_lock(&alloc->lock);
    defer ticketlock_unlock(&alloc->lock);

    llist_node_t* node = llist_pop(&alloc->partial_list);

    spalloc_page_header_t* hdr;
    if (node != nullptr) {
        hdr = SPALLOC_PAGE_OF(node);
        assert(hdr->magic == SPALLOC_MAGIC);
    } else {
        uint64_t paddr = buddy_allocate(MIN_ORDER, ALLOC_FLAG_NOFAIL);
        hdr = TO_HHDM_PTR(paddr);
        hdr->magic = SPALLOC_MAGIC;
        hdr->freecount = alloc->objs_per_page;
        hdr->free_objs.head = nullptr;
        uintptr_t vbase = (uintptr_t)hdr + alloc->obj_offset;
        for (uint32_t i = 0; i < alloc->objs_per_page; i++)
            stack_push(&hdr->free_objs, (stack_node_t*)(vbase + i * alloc->obj_stride));
    }

    stack_node_t* obj = stack_pop(&hdr->free_objs);
    hdr->freecount--;

    // page is not full; return to partial list
    if (hdr->freecount > 0)
        llist_push(&alloc->partial_list, &hdr->linkage);

    return obj;
}

void spalloc_free(spalloc_allocator_t* alloc, void* obj) {
    ticketlock_lock(&alloc->lock);
    defer ticketlock_unlock(&alloc->lock);

    spalloc_page_header_t* hdr = SPALLOC_PAGE_OF(obj);
    assert(hdr->magic == SPALLOC_MAGIC);

    stack_push(&hdr->free_objs, (stack_node_t*)obj);
    hdr->freecount++;

    // page was full; return to partial list
    if (hdr->freecount == 1) {
        llist_push(&alloc->partial_list, &hdr->linkage);
        return;
    }

    // page is now empty; return to pmm
    if (hdr->freecount == alloc->objs_per_page) {
        llist_node_delete(&alloc->partial_list, &hdr->linkage);
        buddy_free(FROM_HHDM((uintptr_t)hdr), MIN_ORDER);
        return;
    }
}
