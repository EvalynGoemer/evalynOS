#pragma once

#include <stdint.h>
#include <utils/lib.h>

#define BSTREE_INIT ((bstree_t) {0})
#define BSTREE_NODE_INIT ((bstree_node_t) {0})

#define BSTREE_SUCCESSOR_OR_NULL(node)   ((node) ? bstree_successor(node) : nullptr)
#define BSTREE_PREDECESSOR_OR_NULL(node) ((node) ? bstree_predecessor(node) : nullptr)

#define BSTREE_FOR_EACH(tree, it)                            \
    for (bstree_node_t *(it) = bstree_minimum((tree).root);  \
         (it);                                               \
         (it) = bstree_successor((it)))

#define BSTREE_FOR_EACH_REVERSE(tree, it)                    \
    for (bstree_node_t *(it) = bstree_maximum((tree).root);  \
         (it);                                               \
         (it) = bstree_predecessor((it)))

#define BSTREE_FOR_EACH_SAFE(tree, it, nx)                      \
    for (bstree_node_t *(it) = bstree_minimum((tree).root),     \
                       *(nx) = BSTREE_SUCCESSOR_OR_NULL(it);    \
         (it);                                                  \
         (it) = (nx),                                           \
         (nx) = BSTREE_SUCCESSOR_OR_NULL(nx))

#define BSTREE_FOR_EACH_REVERSE_SAFE(tree, it, px)              \
    for (bstree_node_t *(it) = bstree_maximum((tree).root),     \
                       *(px) = BSTREE_PREDECESSOR_OR_NULL(it);  \
         (it);                                                  \
         (it) = (px),                                           \
         (px) = BSTREE_PREDECESSOR_OR_NULL(px))

typedef enum bstree_direction_t: uint_fast8_t {
    BST_LEFT,
    BST_RIGHT,
} bstree_direction_t;

typedef enum bstree_rbcolor_t: uint32_t {
    RB_BLACK,
    RB_RED,
} bstree_rbcolor_t;

typedef enum: uint16_t {
    BST_SEARCH_TYPE_EXACT,
    BST_SEARCH_TYPE_NEAREST,
    BST_SEARCH_TYPE_NEAREST_LT,
    BST_SEARCH_TYPE_NEAREST_LTE,
    BST_SEARCH_TYPE_NEAREST_GT,
    BST_SEARCH_TYPE_NEAREST_GTE,
} bstree_search_type_t;

typedef enum: uint16_t {
    BST_TYPE_NORM,
    BST_TYPE_RB,
    BST_TYPE_AVL,
} bstree_tree_type_t;

typedef struct bstree bstree_t;
typedef struct bstree_node bstree_node_t;

typedef struct bstree {
    uint64_t (*value_of_node)(bstree_node_t* node);
    bstree_tree_type_t type;
    bstree_node_t* root;
} bstree_t;

typedef struct bstree_node {
    bstree_node_t* parent;
    union {
        struct {
            bstree_node_t* left;
            bstree_node_t* right;
        };
        bstree_node_t* children[2];
    };
    union {
        bstree_rbcolor_t rbcolor;
        int32_t avlheight;
    } augment;
} bstree_node_t;

static inline bstree_direction_t bstree_direction(const bstree_node_t* node) {
    return node == node->parent->right ? BST_RIGHT : BST_LEFT;
}

extern bstree_node_t* bstree_search(bstree_t* tree, uint64_t query, bstree_search_type_t type);
extern bstree_node_t* bstree_insert(bstree_t* tree, bstree_node_t* node);
extern bstree_node_t* bstree_remove(bstree_t* tree, bstree_node_t* node);

extern bstree_node_t* bstree_rotate_subtree(bstree_t* tree, bstree_node_t* sub, bstree_direction_t dir);
extern bstree_node_t* bstree_minimum(bstree_node_t* node);
extern bstree_node_t* bstree_maximum(bstree_node_t* node);
extern bstree_node_t* bstree_successor(bstree_node_t* node);
extern bstree_node_t* bstree_predecessor(bstree_node_t* node);
