#pragma once

#include <utils/dstruct/bstree.h>

extern void rbtree_insert_fixup(bstree_t* tree, bstree_node_t* node, bstree_node_t* parent, bstree_direction_t dir);
extern void rbtree_remove_fixup(bstree_t* tree, bstree_node_t* node, bstree_node_t* parent, bstree_node_t* replacement, bstree_direction_t dir);
