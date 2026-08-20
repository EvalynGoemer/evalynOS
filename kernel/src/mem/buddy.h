#pragma once
#include <utils/dstruct/llist.h>

#define MIN_ORDER 0
#define MAX_ORDER 14

#define ALLOC_FLAG_NOFAIL (1 << 0)

extern void buddy_init();
extern uint64_t buddy_allocate(int order, size_t flags);
extern void buddy_free(uint64_t paddr, int order);
