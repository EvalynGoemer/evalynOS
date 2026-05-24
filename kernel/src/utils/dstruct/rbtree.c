#include <utils/dstruct/bstree.h>

void rbtree_insert_fixup(bstree_t* tree, bstree_node_t* node, bstree_node_t* parent, bstree_direction_t dir) {
    node->augment.rbcolor = RB_RED;
    node->parent = parent;

    if (!parent) {
        tree->root = node;
        node->augment.rbcolor = RB_BLACK;
        return;
    }

    parent->children[dir] = node;

    do {
        // case 1
        if (parent->augment.rbcolor == RB_BLACK) {
            return;
        }

        bstree_node_t* grandparent = parent->parent;

        // case 4
        if (!grandparent) {
            parent->augment.rbcolor = RB_BLACK;
            return;
        }

        dir = bstree_direction(parent);
        bstree_node_t* uncle = grandparent->children[1 - dir];
        if (!uncle || uncle->augment.rbcolor == RB_BLACK) {
            // case 5
            if (node == parent->children[1 - dir]) {
                bstree_rotate_subtree(tree, parent, dir);
                node = parent;
                parent = grandparent->children[dir];
            }

            // case 6
            bstree_rotate_subtree(tree, grandparent, 1 - dir);
            parent->augment.rbcolor = RB_BLACK;
            grandparent->augment.rbcolor = RB_RED;
            return;
        }

        // case 2
        parent->augment.rbcolor = RB_BLACK;
        uncle->augment.rbcolor = RB_BLACK;
        grandparent->augment.rbcolor = RB_RED;
        node = grandparent;

    } while ((parent = node->parent));

    // case 3
    tree->root->augment.rbcolor = RB_BLACK;
    return;
}

static void __rbtree_remove_fixup(bstree_t* tree, bstree_node_t* node, bstree_direction_t dir) {
    bstree_node_t* parent = node->parent;

    if (!parent) {
        node->augment.rbcolor = RB_BLACK;
        return;
    }

    bstree_node_t* sibling;
    bstree_node_t* close_nephew;
    bstree_node_t* distant_nephew;

    do {
        sibling = parent->children[1 - dir];
        distant_nephew = sibling->children[1 - dir];
        close_nephew = sibling->children[dir];
        if (sibling->augment.rbcolor == RB_RED) {
            // case 3
            bstree_rotate_subtree(tree, parent, dir);
            parent->augment.rbcolor = RB_RED;
            sibling->augment.rbcolor = RB_BLACK;
            sibling = close_nephew;

            distant_nephew = sibling->children[1 - dir];
            if (distant_nephew && distant_nephew->augment.rbcolor == RB_RED) {
                goto case_6;
            }
            close_nephew = sibling->children[dir];
            if (close_nephew && close_nephew->augment.rbcolor == RB_RED) {
                goto case_5;
            }

            // case 4
            sibling->augment.rbcolor = RB_RED;
            parent->augment.rbcolor = RB_BLACK;
            return;
        }

        if (distant_nephew && distant_nephew->augment.rbcolor == RB_RED)
            goto case_6;

        if (close_nephew && close_nephew->augment.rbcolor == RB_RED)
            goto case_5;

        // case 4
        if (parent->augment.rbcolor == RB_RED) {
            sibling->augment.rbcolor = RB_RED;
            parent->augment.rbcolor = RB_BLACK;
            return;
        }

        // case 2
        sibling->augment.rbcolor = RB_RED;
        node = parent;
        if (!node->parent) break;
        dir = bstree_direction(node);

    } while ((parent = node->parent));

    // case 1
    return;

case_5:

    bstree_rotate_subtree(tree, sibling, 1 - dir);
    sibling->augment.rbcolor = RB_RED;
    close_nephew->augment.rbcolor = RB_BLACK;
    distant_nephew = sibling;
    sibling = close_nephew;

case_6:

    bstree_rotate_subtree(tree, parent, dir);
    sibling->augment.rbcolor = parent->augment.rbcolor;
    parent->augment.rbcolor = RB_BLACK;
    distant_nephew->augment.rbcolor = RB_BLACK;
    return;
}

void rbtree_remove_fixup(bstree_t* tree, bstree_node_t* node, bstree_node_t* parent, bstree_node_t* replacement, bstree_direction_t dir) {
    if (node->augment.rbcolor == RB_BLACK) {
        if (replacement && replacement->augment.rbcolor == RB_RED) {
            replacement->augment.rbcolor = RB_BLACK;
        } else if (replacement) {
            __rbtree_remove_fixup(tree, replacement, dir);
        } else if (parent) {
            __rbtree_remove_fixup(tree, node, dir);
        }
    }
}
