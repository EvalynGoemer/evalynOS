#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <utils/lib.h>
#include <arch/generic/paging/paging.h>
#include <mem/buddy.h>
#include <mem/spalloc.h>
#include <libc/stdlib.h>

#define MAGIC      0x9f37a967791335cb
#define MIN_SHIFT  3
#define MAX_SHIFT  (PAGE_SIZE_SHIFT - 2)
#define MAX_CLASS  (1 << MAX_SHIFT)

typedef struct malloc_header {
    uint64_t magic;
    uint64_t size;
} malloc_header_t;

static spalloc_allocator_t kmalloc_classes[MAX_SHIFT - MIN_SHIFT + 1];

void malloc_init() {
    for (int i = 0; i < MAX_SHIFT - MIN_SHIFT + 1; i++)
        spalloc_init(&kmalloc_classes[i], ((size_t)1 << (MIN_SHIFT + i)) + sizeof(malloc_header_t), 16);
}

[[gnu::malloc]]
void* malloc(size_t size) {
    if (size == 0) return nullptr;

    if (size <= MAX_CLASS) {
        int shift = log2ull(size);
        if ((size_t)1 << shift < size) shift++;
        if (shift < MIN_SHIFT) shift = MIN_SHIFT;

        malloc_header_t* hdr = spalloc_malloc(&kmalloc_classes[shift - MIN_SHIFT]);
        hdr->magic = MAGIC;
        hdr->size  = 1 << shift;
        return hdr + 1;
    }

    size_t block = ALIGN_UP(size + sizeof(malloc_header_t), PAGE_SIZE);
    int order = log2ullceil(block >> PAGE_SIZE_SHIFT);

    malloc_header_t* hdr = TO_HHDM_PTR(buddy_allocate(order, ALLOC_FLAG_NOFAIL));
    hdr->magic = MAGIC;
    hdr->size  = block;
    return hdr + 1;
}

void free(void* ptr) {
    if (ptr == nullptr) return;

    malloc_header_t* hdr = (malloc_header_t*)ptr - 1;
    assert(hdr->magic == MAGIC);

    if (hdr->size <= MAX_CLASS) {
        int shift = log2ull(hdr->size);
        spalloc_free(&kmalloc_classes[shift - MIN_SHIFT], hdr);
    } else {
        int order = log2ullceil(hdr->size >> PAGE_SIZE_SHIFT);
        buddy_free(FROM_HHDM((uintptr_t)hdr), order);
    }
}
