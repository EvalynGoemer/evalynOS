#include "../memory/heap.h"
#include <stddef.h>

void* malloc(size_t size) {
    return kmalloc(size);
}

void free (void *ptr) {
    kfree(ptr);
}
