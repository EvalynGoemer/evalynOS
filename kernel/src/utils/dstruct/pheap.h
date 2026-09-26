#pragma once
#include <stdint.h>

typedef struct pheap_node pheap_node_t;
typedef struct pheap pheap_t;

struct pheap {
    pheap_node_t* head;
    uint64_t (*value_of_node)(pheap_node_t* node);
};

struct pheap_node {
    pheap_node_t* left_child;
    pheap_node_t* next_sibling;
    pheap_node_t* prev_sibling;
};

extern void pheap_insert(pheap_t* heap, pheap_node_t* node);
extern pheap_node_t* pheap_remove(pheap_t* heap, pheap_node_t* node);
extern pheap_node_t* pheap_try_remove(pheap_t* heap, pheap_node_t* node);
extern pheap_node_t* pheap_pop(pheap_t* heap);

static inline pheap_node_t* pheap_peek(pheap_t* heap) {
    return heap->head;
}
