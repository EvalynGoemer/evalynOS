#include <utils/dstruct/pheap.h>

// basic implementation of a pairing heap
// https://www.cs.cmu.edu/~sleator/papers/pairing-heaps.pdf

static void pheap_attach(pheap_node_t* parent, pheap_node_t* child) {
    child->prev_sibling = parent;
    child->next_sibling = parent->left_child;
    if (parent->left_child)
        parent->left_child->prev_sibling = child;
    parent->left_child = child;
}

static void pheap_detach(pheap_node_t* node) {
    pheap_node_t* prev = node->prev_sibling;

    if (prev->left_child == node) {
        prev->left_child = node->next_sibling;
    } else {
        prev->next_sibling = node->next_sibling;
    }

    if (node->next_sibling)
        node->next_sibling->prev_sibling = prev;
}

static pheap_node_t* pheap_meld(pheap_t* heap, pheap_node_t* first, pheap_node_t* second) {
    if (first == nullptr) return second;
    if (second == nullptr) return first;

    if (heap->value_of_node(first) <= heap->value_of_node(second)) {
        pheap_attach(first, second);
        return first;
    }

    pheap_attach(second, first);
    return second;
}

static pheap_node_t* pheap_merge_children(pheap_t* heap, pheap_node_t* root) {
    pheap_node_t* cur = root->left_child;
    pheap_node_t* pairs = nullptr;

    while (cur != nullptr) {
        pheap_node_t* first = cur;
        pheap_node_t* second = first->next_sibling;
        cur = second ? second->next_sibling : nullptr;

        first = pheap_meld(heap, first, second);
        first->next_sibling = pairs;
        pairs = first;
    }

    pheap_node_t* head = nullptr;
    while (pairs != nullptr) {
        pheap_node_t* next = pairs->next_sibling;
        head = pheap_meld(heap, head, pairs);
        pairs = next;
    }
    return head;
}

void pheap_insert(pheap_t* heap, pheap_node_t* node) {
    node->left_child = nullptr;
    heap->head = pheap_meld(heap, heap->head, node);
}

pheap_node_t* pheap_pop(pheap_t* heap) {
    pheap_node_t* root = heap->head;
    if (root == nullptr) return nullptr;
    heap->head = pheap_merge_children(heap, root);

    // left child being a pointer to address 1 is a canary for it being removed
    // this is safe because on insertion the left child is always nulled
    root->left_child = (void*)1;
    return root;
}

pheap_node_t* pheap_remove(pheap_t* heap, pheap_node_t* node) {
    if (node == heap->head) return pheap_pop(heap);
    pheap_detach(node);
    pheap_node_t* children = pheap_merge_children(heap, node);
    heap->head = pheap_meld(heap, heap->head, children);

    // left child being a pointer to address 1 is a canary for it being removed
    // this is safe because on insertion the left child is always nulled
    node->left_child = (void*)1;
    return node;
}

pheap_node_t* pheap_try_remove(pheap_t* heap, pheap_node_t* node) {
    if (node->left_child != (void*)1)
        return pheap_remove(heap, node);
    return nullptr;
}
