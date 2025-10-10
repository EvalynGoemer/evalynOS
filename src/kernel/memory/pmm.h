#pragma once
extern void* bump_alloc_base;
void kbump_alloc_init(unsigned long min_heap_size);
void *kbump_alloc(unsigned long alloc_size);
void *kbump_alloc_phys(unsigned long alloc_size);
