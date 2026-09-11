#pragma once
#include <stddef.h>

extern void malloc_init();

[[gnu::malloc]]
extern void* malloc(size_t size);
extern void free(void* ptr);
