#pragma once

#include <stddef.h>

#define LLIST_INIT ((llist_t) { .count = 0, .head = nullptr, .tail = nullptr })

#define CONTAINER_OF(PTR, TYPE, MEMBER)                                                                                               \
    ({                                                                                                                                \
        static_assert(__builtin_types_compatible_p(typeof(((TYPE*) 0)->MEMBER), typeof(*PTR)), "member type does not match pointer"); \
        (TYPE*) (((uintptr_t) (PTR)) - __builtin_offsetof(TYPE, MEMBER));                                                             \
    })

typedef struct llist_node llist_node_t;
typedef struct llist llist_t;

struct llist {
    size_t count;
    llist_node_t* head;
    llist_node_t* tail;
};

struct llist_node {
    llist_node_t* next;
    llist_node_t* prev;
};

void llist_push(llist_t* list, llist_node_t* node);
llist_node_t* llist_pop(llist_t* list);

void llist_push_front(llist_t* list, llist_node_t* node);
void llist_push_back(llist_t* list, llist_node_t* node);

llist_node_t* llist_pop_front(llist_t* list);
llist_node_t* llist_pop_back(llist_t* list);

void llist_node_append(llist_t* list, llist_node_t* pos, llist_node_t* node);
void llist_node_prepend(llist_t* list, llist_node_t* pos, llist_node_t* node);
void llist_node_delete(llist_t* list, llist_node_t* node);
