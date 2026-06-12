#pragma once

#include <stddef.h>
#include <utils/lib.h>

#define LLIST_INIT ((llist_t) { .count = 0, .head = nullptr, .tail = nullptr })

#define LLIST_NEXT_OR_NULL(node)  ((node) ? (node)->next : nullptr)
#define LLIST_PREV_OR_NULL(node)  ((node) ? (node)->prev : nullptr)

#define LLIST_FOR_EACH(list, it) \
    for (llist_node_t *(it) = (list).head; (it); (it) = (it)->next)

#define LLIST_FOR_EACH_REVERSE(list, it) \
    for (llist_node_t *(it) = (list).tail; (it); (it) = (it)->prev)

#define LLIST_FOR_EACH_SAFE(list, it, nx)              \
    for (llist_node_t *(it) = (list).head,             \
                       *(nx) = LLIST_NEXT_OR_NULL(it); \
         (it);                                         \
         (it) = (nx), (nx) = LLIST_NEXT_OR_NULL(nx))

#define LLIST_FOR_EACH_REVERSE_SAFE(list, it, px)      \
    for (llist_node_t *(it) = (list).tail,             \
                       *(px) = LLIST_PREV_OR_NULL(it); \
         (it);                                         \
         (it) = (px), (px) = LLIST_PREV_OR_NULL(px))

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
