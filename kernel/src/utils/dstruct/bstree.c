#include <assert.h>

#include <utils/dstruct/bstree.h>
#include <utils/dstruct/rbtree.h>

bstree_node_t* bstree_minimum(bstree_node_t* node) {
    while (node->left != nullptr)
        node = node->left;
    return node;
}

bstree_node_t* bstree_maximum(bstree_node_t* node) {
    while (node->right != nullptr)
        node = node->right;
    return node;
}

bstree_node_t* bstree_successor(bstree_node_t* node) {
    if (node->right != nullptr)
        return bstree_minimum(node->right);
    bstree_node_t* parent = node->parent;
    while (parent != nullptr && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

bstree_node_t* bstree_predecessor(bstree_node_t* node) {
    if (node->left != nullptr)
        return bstree_maximum(node->left);
    bstree_node_t* parent = node->parent;
    while (parent != nullptr && node == parent->left) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

static inline bstree_node_t* bstree_search_exact(bstree_t* tree, uint64_t query) {
    bstree_node_t* node = tree->root;
    while (node != nullptr) {
        uint64_t val = tree->value_of_node(node);
        if (query < val)
            node = node->left;
        else if (query > val)
            node = node->right;
        else
            return node;
    }
    return nullptr;
}

static inline bstree_node_t* bstree_search_nearest(bstree_t* tree, uint64_t query) {
    bstree_node_t* node = tree->root;
    bstree_node_t* nearest = nullptr;
    uint64_t best = UINT64_MAX;
    while (node != nullptr) {
        uint64_t val = tree->value_of_node(node);
        uint64_t dist = val > query ? val - query : query - val;
        if (dist < best) {
            best = dist;
            nearest = node;
        }
        if (query < val)
            node = node->left;
        else if (query > val)
            node = node->right;
        else
            return node;
    }
    return nearest;
}

bstree_node_t* bstree_rotate_subtree(bstree_t* tree, bstree_node_t* sub, bstree_direction_t dir) {
    bstree_node_t* sub_parent = sub->parent;
    bstree_node_t* new_root = sub->children[1 - dir];
    bstree_node_t* new_child = new_root->children[dir];

    sub->children[1 - dir] = new_child;

    if (new_child)
        new_child->parent = sub;

    new_root->children[dir] = sub;
    new_root->parent = sub_parent;
    sub->parent = new_root;

    if (sub_parent)
        sub_parent->children[sub == sub_parent->right] = new_root;
    else
        tree->root = new_root;

    return new_root;
}

static inline bstree_node_t* bstree_search_lesser(bstree_t* tree, uint64_t query, bool find_equal) {
    bstree_node_t* node = tree->root;
    bstree_node_t* candidate = nullptr;
    while (node != nullptr) {
        uint64_t val = tree->value_of_node(node);
        if (val < query) {
            candidate = node;
            node = node->right;
        } else if (val > query) {
            node = node->left;
        } else {
            if (find_equal) return node;
            candidate = node->left ? bstree_maximum(node->left) : candidate;
            return candidate;
        }
    }
    return candidate;
}

static inline bstree_node_t* bstree_search_greater(bstree_t* tree, uint64_t query, bool find_equal) {
    bstree_node_t* node = tree->root;
    bstree_node_t* candidate = nullptr;
    while (node != nullptr) {
        uint64_t val = tree->value_of_node(node);
        if (val > query) {
            candidate = node;
            node = node->left;
        } else if (val < query) {
            node = node->right;
        } else {
            if (find_equal) return node;
            candidate = node->right ? bstree_minimum(node->right) : candidate;
            return candidate;
        }
    }
    return candidate;
}

bstree_node_t* bstree_search(bstree_t* tree, uint64_t query, bstree_search_type_t type) {
    switch (type) {
        case BST_SEARCH_TYPE_EXACT:       return bstree_search_exact(tree, query);
        case BST_SEARCH_TYPE_NEAREST:     return bstree_search_nearest(tree, query);
        case BST_SEARCH_TYPE_NEAREST_LT:  return bstree_search_lesser(tree, query, false);
        case BST_SEARCH_TYPE_NEAREST_LTE: return bstree_search_lesser(tree, query, true);
        case BST_SEARCH_TYPE_NEAREST_GT:  return bstree_search_greater(tree, query, false);
        case BST_SEARCH_TYPE_NEAREST_GTE: return bstree_search_greater(tree, query, true);
        default: __builtin_unreachable();
    }
}

bstree_node_t* bstree_insert(bstree_t* tree, bstree_node_t* node) {
    node->left = nullptr;
    node->right = nullptr;
    node->parent = nullptr;

    bstree_node_t* parent = nullptr;
    bstree_node_t* cur = tree->root;
    bstree_direction_t dir = BST_LEFT;

    uint64_t key = tree->value_of_node(node);

    while (cur) {
        parent = cur;
        uint64_t cur_val = tree->value_of_node(cur);

        if (key < cur_val) {
            dir = BST_LEFT;
            cur = cur->left;
        } else {
            dir = BST_RIGHT;
            cur = cur->right;
        }
    }

    if (!parent) {
        tree->root = node;
        node->parent = nullptr;
    } else {
        node->parent = parent;
        parent->children[dir] = node;
    }

    switch (tree->type) {
        case BST_TYPE_NORM: return node;
        case BST_TYPE_RB: rbtree_insert_fixup(tree, node, parent, dir); return node;
        case BST_TYPE_AVL: assert("unimplemented" && false); return nullptr;
        default: __builtin_unreachable();
    }
}

bstree_node_t* bstree_remove(bstree_t* tree, bstree_node_t* node) {
    bstree_node_t* parent = node->parent;
    bstree_node_t* replacement = nullptr;
    bstree_direction_t dir = BST_LEFT;

    if (node->left && node->right) {
        bstree_node_t* succ = bstree_minimum(node->right);
        bstree_node_t* succ_parent = succ->parent;
        bstree_node_t* succ_right  = succ->right;

        succ->left = node->left;
        succ->left->parent = succ;

        succ->parent = node->parent;
        if (!node->parent)
            tree->root = succ;
        else
            node->parent->children[node == node->parent->right] = succ;

        if (succ_parent == node) {
            succ->right = node;
            node->parent = succ;
        } else {
            succ->right = node->right;
            succ->right->parent = succ;
            succ_parent->left = node;
            node->parent = succ_parent;
        }

        node->left  = nullptr;
        node->right = succ_right;
        if (node->right) node->right->parent = node;

        typeof(node->augment) tmp = succ->augment;
        succ->augment = node->augment;
        node->augment = tmp;

        parent = node->parent;
    }

    replacement = node->left ? node->left : node->right;

    if (replacement)
        replacement->parent = parent;

    if (!parent) {
        tree->root = replacement;
    } else {
        dir = bstree_direction(node);
        parent->children[dir] = replacement;
    }

    switch (tree->type) {
        case BST_TYPE_NORM: break;
        case BST_TYPE_RB: rbtree_remove_fixup(tree, node, parent, replacement, dir); break;
        case BST_TYPE_AVL: assert("unimplemented" && false); return nullptr;
        default: __builtin_unreachable();
    }
    return replacement;
}
