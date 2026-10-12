#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <utils/lib.h>
#include <arch/generic/paging/paging.h>
#include <mem/buddy.h>
#include <mem/spalloc.h>
#include <libc/stdlib.h>

#define MAGIC      0x9f37a967
#define MIN_SHIFT  3
#define MAX_SHIFT  (PAGE_SIZE_SHIFT - 2)
#define MAX_CLASS  (1 << MAX_SHIFT)

typedef struct malloc_header {
    uint32_t magic;
    uint32_t offset;
    uint64_t size;
} malloc_header_t;

static spalloc_allocator_t kmalloc_classes[MAX_SHIFT - MIN_SHIFT + 1];

static inline int size_class(size_t size) {
    int shift = log2ull(size);
    if (((size_t)1 << shift) < size) shift++;
    return shift < MIN_SHIFT ? MIN_SHIFT : shift;
}

static inline int size_order(size_t size) {
    size_t pages = ALIGN_UP(size, PAGE_SIZE) >> PAGE_SIZE_SHIFT;
    return log2ullceil(pages);
}

void malloc_init() {
    for (int i = 0; i < MAX_SHIFT - MIN_SHIFT + 1; i++)
        spalloc_init(&kmalloc_classes[i], (size_t)1 << (MIN_SHIFT + i), 16);
}

void* kmalloc(size_t size) {
    if (size == 0) return nullptr;
    if (size <= MAX_CLASS)
        return spalloc_malloc(&kmalloc_classes[size_class(size) - MIN_SHIFT]);
    else
        return TO_HHDM_PTR(buddy_allocate(size_order(size), ALLOC_FLAG_NOFAIL));
}

void kfree(void* ptr, size_t size) {
    if (ptr == nullptr) return;
    if (size <= MAX_CLASS)
        spalloc_free(&kmalloc_classes[size_class(size) - MIN_SHIFT], ptr);
    else
        buddy_free(FROM_HHDM(ptr), size_order(size));
}

[[gnu::malloc]]
void* malloc(size_t size) {
    if (size == 0) return nullptr;
    malloc_header_t* hdr = kmalloc(size + sizeof(malloc_header_t));
    if (hdr == nullptr) return nullptr;
    hdr->size = size + sizeof(malloc_header_t);
    hdr->magic = MAGIC;
    hdr->offset = 0;
    return hdr + 1;
}

void* malloc_aligned(size_t size, size_t align) {
    if (size == 0) return nullptr;
    if (align < 16) align = 16;
    assert(isPow2ull(align));

    size_t total = size + sizeof(malloc_header_t) + align - 1;

    uint8_t* raw = kmalloc(total);
    if (raw == nullptr) return nullptr;

    uint8_t* ptr = (uint8_t*)ALIGN_UP(raw + sizeof(malloc_header_t), align);
    malloc_header_t* hdr = (malloc_header_t*)ptr - 1;

    hdr->size = total;
    hdr->magic = MAGIC;
    hdr->offset = (uint8_t*)hdr - raw;

    return ptr;
}

void free(void* ptr) {
    if (ptr == nullptr) return;
    malloc_header_t* hdr = (malloc_header_t*)ptr - 1;
    assert(hdr->magic == MAGIC);
    void* rptr = (uint8_t*)hdr - hdr->offset;
    kfree(rptr, hdr->size);
}
