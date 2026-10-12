#pragma once
#include <stddef.h>

extern void malloc_init();

extern void* kmalloc(size_t size);
extern void kfree(void* ptr, size_t size);

[[gnu::malloc]] extern void* malloc(size_t size);
extern void* malloc_aligned(size_t size, size_t align);
extern void free(void* ptr);
