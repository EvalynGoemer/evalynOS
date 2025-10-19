#pragma once
#include <stddef.h>

void setup_heap();
void* kmalloc (size_t size);
void kfree (void *ptr);
