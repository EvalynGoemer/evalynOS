#pragma once

#include <mem/vmem.h>
#include <stdint.h>

typedef struct address_space {
    uint64_t pagetable;
    vmem_allocator_t valloc;
} address_space_t;

extern address_space_t* new_address_space();
