#pragma once
#include <stdint.h>

typedef struct freelist_pmm_node {
    struct freelist_pmm_node* next;
} freelist_pmm_node_t;

extern freelist_pmm_node_t* freelist_pmm_head;
extern uint64_t freelist_pmm_fill_entry;
extern uint64_t freelist_pmm_fill_offset;

extern void freelist_pmm_init();
