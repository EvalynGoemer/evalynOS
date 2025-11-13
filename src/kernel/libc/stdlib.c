#include <stddef.h>
#include <memory/heap.h>

void* malloc(size_t size) {
    return kmalloc(size);
}

void free (void *ptr) {
    kfree(ptr);
}
