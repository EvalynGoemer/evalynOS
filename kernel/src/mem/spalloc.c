#include <mem/pmm.h>
#include <utils/dstruct/llist.h>
#include <utils/limine.h>
#include <utils/locks/ticketlock.h>
#include <utils/lib.h>
#include <mem/spalloc.h>
#include <stdint.h>
#include <utils/defer.h>
#include <assert.h>

#define COUNT_TRAILING_ZEROS(n) __builtin_ctzll((n))

#define SPALLOC_OBJ_STRIDE(size, align) ALIGN_UP((size), (align))

#define SPALLOC_PAGE_HDR_SIZE(align) ALIGN_UP(sizeof(spalloc_page_header_t), (align))

#define SPALLOC_PAGE_OF(ptr) ((spalloc_page_header_t *)ALIGN_DOWN((uintptr_t)(ptr), PAGE_SIZE))

[[gnu::always_inline]]
static inline uint32_t spalloc_obj_count(uint32_t size, uint32_t align) {
    uint32_t n = (PAGE_SIZE - SPALLOC_PAGE_HDR_SIZE(align)) / SPALLOC_OBJ_STRIDE(size, align);
    return MIN(128, n);
}

[[gnu::always_inline]]
static inline uint64_t spalloc_init_bmap_word(uint32_t size, uint32_t align, uint32_t word) {
    uint32_t c = spalloc_obj_count(size, align);
    uint32_t lo = word * 64u;
    if (c <= lo) {
        return ~0ull;
    } else if (c >= lo + 64u) {
        return 0ull;
    } else {
        return ~((1ull << (c - lo)) - 1ull);
    }
}

[[gnu::always_inline]]
static inline void *spalloc_obj_ptr(void* hdr, uint32_t bit, uint32_t size, uint32_t align) {
    uintptr_t base = (uintptr_t)hdr + SPALLOC_PAGE_HDR_SIZE(align);
    return (void*)(base + (uint64_t)bit * SPALLOC_OBJ_STRIDE(size, align));
}

[[gnu::always_inline]]
static inline uint32_t spalloc_obj_bit(void* hdr, void* ptr, uint32_t size, uint32_t align) {
    uintptr_t base = (uintptr_t)hdr + SPALLOC_PAGE_HDR_SIZE(align);
    return (uint32_t)(((uintptr_t)ptr - base) / SPALLOC_OBJ_STRIDE(size, align));
}

[[gnu::always_inline]]
static inline bool hdr_isempty(spalloc_allocator_t *alloc, spalloc_page_header_t* hdr) {
    if (hdr->bmap[0] == alloc->word0_initial_state && hdr->bmap[1] == alloc->word1_initial_state)
        return true;
    return false;
}

[[gnu::always_inline]]
static inline bool hdr_notfull(spalloc_page_header_t* hdr) {
    if (~hdr->bmap[0] != 0ull || ~hdr->bmap[1] != 0ull)
        return true;
    return false;
}

bool spalloc_init(spalloc_allocator_t *alloc, int obj_size, int obj_align) {
    if (alloc == nullptr)                                                 return false;
    if (obj_size <= 0)                                                    return false;
    if (obj_align <= 0)                                                   return false;
    if ((obj_align & (obj_align - 1)) != 0)                               return false;
    if ((size_t)obj_size >= PAGE_SIZE - sizeof(spalloc_page_header_t))    return false;
    if ((size_t)obj_align >= PAGE_SIZE - sizeof(spalloc_page_header_t))   return false;
    if (spalloc_obj_count(obj_size, obj_align) < 1u)                      return false;

    alloc->obj_size      = obj_size;
    alloc->obj_alignment = obj_align;
    alloc->partial_list   = LLIST_INIT;
    alloc->word0_initial_state = spalloc_init_bmap_word(alloc->obj_size, alloc->obj_alignment, 0);
    alloc->word1_initial_state = spalloc_init_bmap_word(alloc->obj_size, alloc->obj_alignment, 1);
    ticketlock_init(&alloc->lock);

    return true;
}

[[gnu::malloc]]
void *spalloc_malloc(spalloc_allocator_t *alloc) {
    int lock1r = ticketlock_lock(&alloc->lock);
    defer ticketlock_unlock(&alloc->lock, lock1r);

    llist_node_t *node = llist_pop(&alloc->partial_list);
    spalloc_page_header_t *hdr;
    if (node == NULL) {
        uintptr_t phys = pmm_alloc_page();
        hdr = TO_HHDM_PTR(phys);
        hdr->bmap[0] = alloc->word0_initial_state;
        hdr->bmap[1] = alloc->word1_initial_state;
    } else {
        hdr = CONTAINER_OF(node, spalloc_page_header_t, llnode);
    }

    // find the non full word
    uint32_t word = (~hdr->bmap[0] != 0ull) ? 0 : 1;

    // find a free place to allocate from
    assert(~hdr->bmap[word] != 0);
    uint32_t pos = COUNT_TRAILING_ZEROS(~hdr->bmap[word]);

    // mark as allocated
    assert((hdr->bmap[word] & (1ull << pos)) == 0);
    hdr->bmap[word] |= (1ull << (pos));

    // if its not full push it back
    if (hdr_notfull(hdr))
        llist_push(&alloc->partial_list, &hdr->llnode);

    // offset pos based on the word it came from
    pos += word * 64;
    return spalloc_obj_ptr(hdr, pos, alloc->obj_size, alloc->obj_alignment);
}

void spalloc_free(spalloc_allocator_t *alloc, void *obj) {
    int lock1r = ticketlock_lock(&alloc->lock);
    defer ticketlock_unlock(&alloc->lock, lock1r);

    // find the pos of the object
    spalloc_page_header_t *hdr = SPALLOC_PAGE_OF(obj);
    uint32_t pos = spalloc_obj_bit(hdr, obj, alloc->obj_size, alloc->obj_alignment);
    uint32_t word = pos / 64;
    pos -= word * 64;

    bool was_full = !hdr_notfull(hdr);

    // mark as free
    assert(hdr->bmap[word] & (1ull << pos));
    hdr->bmap[word] &= ~(1ull << pos);

    // if this page is empty free it and remove from freelist
    if (hdr_isempty(alloc, hdr)) {
        if (!was_full) llist_node_delete(&alloc->partial_list, &hdr->llnode);
        pmm_free_page(FROM_HHDM((uintptr_t)hdr));
        return;
    }

    // if the page was full add it back
    if (was_full) llist_push(&alloc->partial_list, &hdr->llnode);
}
