#include <stddef.h>
#include <zero/cbt.h>

struct cbtnode *
cbtfind(struct cbt *tree, uintptr_t key)
{
    struct cbtnode *node = &tree->root;

    if (key == node->key) {

        return node;
    } else if (key < node->key && (node->left)) {
        do {
            node = node->left;
            if (node) {
                if (key == node->key) {
                    
                    return node;
                } else if (key < node->key) {
                    node = node->left;
                } else {
                    node = node->right;
                }
            }
        } while ((node) && key != node->key);
    } else {
        do {
            node = node->right;
            if (node) {
                if (key == node->key) {
                    
                    return node;
                } else if (key < node->key) {
                    node = node->left;
                } else {
                    node = node->right;
                }
            }
        } while ((node) && key != node->key);
    }
    if (key == node->key) {

        return node;
    }

    return NULL;
}

