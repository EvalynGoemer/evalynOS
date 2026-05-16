#include <utils/dstruct/llist.h>

static void llist_initial_node(llist_t* llist, llist_node_t* node) {
    node->next = nullptr;
    node->prev = nullptr;
    llist->head = node;
    llist->tail = node;
    llist->count = 1;
}

[[gnu::alias("llist_push_front")]] void llist_push(llist_t* llist, llist_node_t* node);
[[gnu::alias("llist_pop_front")]] llist_node_t* llist_pop(llist_t* llist);

void llist_push_front(llist_t* llist, llist_node_t* node) {
    if(llist->head == nullptr) {
        llist_initial_node(llist, node);
        return;
    }

    llist_node_prepend(llist, llist->head, node);
}

void llist_push_back(llist_t* llist, llist_node_t* node) {
    if(llist->tail == nullptr) {
        llist_initial_node(llist, node);
        return;
    }

    llist_node_append(llist, llist->tail, node);
}


llist_node_t* llist_pop_front(llist_t* llist) {
    llist_node_t* node = llist->head;
    if(node != nullptr) llist_node_delete(llist, node);
    return node;
}

llist_node_t* llist_pop_back(llist_t* llist) {
    llist_node_t* node = llist->tail;
    if(node != nullptr) llist_node_delete(llist, node);
    return node;
}

void llist_node_append(llist_t* llist, llist_node_t* pos, llist_node_t* node) {
    node->next = pos->next;
    node->prev = pos;

    pos->next = node;
    if(node->next != nullptr) node->next->prev = node;

    if(llist->tail == pos) llist->tail = node;
    llist->count++;
}

void llist_node_prepend(llist_t* llist, llist_node_t* pos, llist_node_t* node) {
    node->next = pos;
    node->prev = pos->prev;

    pos->prev = node;
    if(node->prev != nullptr) node->prev->next = node;

    if(llist->head == pos) llist->head = node;
    llist->count++;
}

void llist_node_delete(llist_t* llist, llist_node_t* node) {
    llist->count--;
    if(llist->head == node) llist->head = node->next;
    if(llist->tail == node) llist->tail = node->prev;

    if(node->prev != nullptr) node->prev->next = node->next;
    if(node->next != nullptr) node->next->prev = node->prev;
}
