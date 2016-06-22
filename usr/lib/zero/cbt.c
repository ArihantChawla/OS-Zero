#include <zero/cbt.h>

struct cbtnode *
cbtfind(struct cbt *tree, uintptr_t key)
{
    struct cbtnode *node = &tree->root;

    if (key < node->key && (node->left)) {
        do {
            node = node->left;
            if ((node) && node->key == key) {
                
                return node;
            }
        } while (key < node->key && (node->left));
    } else {
        do {
            node = node->right;
            if ((node) && node->key == key) {
                
                return node;
            }
        } while (key < node->key && (node->right));
    }

    return node;
}

