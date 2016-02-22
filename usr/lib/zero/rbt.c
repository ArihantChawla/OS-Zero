#include <stdlib.h>
#include <stdio.h>
#include <zero/rbt.h>

struct rbt *
rbtinit(int (*cmp)(const void *, const void *))
{
    struct rbt *rbt;
    struct rbt *nil;
    
    rbt = malloc(sizeof(struct rbt));
    if (!rbt) {
        fprintf(stderr, "rbt failed to allocated memory\n");
            
        exit(1);
    }
    rbt->cmp = cmp;
    nil = &rbt->nil;
    rbt->nil.data = NULL;
    rbt->nil.color = RBT_BLACK;
    rbt->root.parent = nil;
    rbt->root.left = nil;
    rbt->root.right = nil;
    rbt->root.color = RBT_BLACK;
    rbt->nil.parent = nil;
    rbt->nil.left = nil;
    rbt->nil.right = nil;

    return rbt;
}

void
rbtrotleft(struct rbt *tree, struct rbtnode *node)
{
    struct rbtnode *child;

    child = node->right;
    node->right = child->left;
    if (child->left != &tree->nil) {
        child->left->parent = node;
    }
    child->parent = node->parent;
    if (node == node->parent->left) {
        node->parent->left = child;
    } else {
        node->parent->right = child;
    }
    child->left = node;
    node->parent = child;

    return;
}

void
rbtrotright(struct rbt *tree, struct rbtnode *node)
{
    struct rbtnode *child;

    child = node->left;
    node->left = child->right;
    if (child->right != &tree->nil) {
        child->right->parent = node;
    }
    child->parent = node->parent;
    if (node == node->parent->left) {
        node->parent->left = child;
    } else {
        node->parent->right = child;
    }
    child->right = node;
    node->parent = child;

    return;
}

struct rbtnode *
rbtinsert(struct rbt *tree, void *data)
{
    struct rbtnode *node = tree->root.left;
    struct rbtnode *parent = &tree->root;
    struct rbtnode *nil = &tree->nil;
    int             res;
    
    while (node != &tree->nil) {
        parent = node;
        res = tree->cmp(data, node->data);
        if (!res) {
            
            return node;
        }
        if (res < 0) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    node = malloc(sizeof(struct rbtnode));
    if (!node) {
        fprintf(stderr, "rbt failed to allocated memory\n");
        
        exit(1);
    }
    node->data = data;
    node->color = RBT_RED;
    node->parent = parent;
    node->left = nil;
    node->right = nil;
    if (parent == &tree->nil || data < parent->data) {
        parent->left = node;
    } else {
        parent->right = node;
    }
    while (node->parent->color == RBT_RED) {
        struct rbtnode *uncle;
        
        if (node->parent == parent->parent->left) {
            uncle = parent->parent->right;
            if (uncle->color == RBT_RED) {
                parent->color = RBT_BLACK;
                uncle->color = RBT_BLACK;
                parent->parent->color = RBT_RED;
                node = parent->parent;
            } else {
                if (node == parent->right) {
                    node = parent;
                    rbtrotleft(tree, node);
                }
                parent->color = RBT_BLACK;
                parent->parent->color = RBT_RED;
                rbtrotright(tree, parent->parent);
            }
        } else {
            uncle = parent->parent->left;
            if (uncle->color == RBT_RED) {
                parent->color = RBT_BLACK;
                uncle->color = RBT_BLACK;
                parent->parent->color = RBT_RED;
                node = parent->parent;
            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    rbtrotright(tree, node);
                }
                node->parent->color = RBT_BLACK;
                node->parent->parent->color = RBT_RED;
                rbtrotleft(tree, node->parent->parent);
            }
        }
    }
    tree->root.left->color = RBT_BLACK;
    
    return NULL;
}

struct rbtnode *
rbtfind(struct rbt *tree, void *key)
{
    struct rbtnode *node = tree->root.left;
    int             res;

    while (node != &tree->nil) {
        res = tree->cmp(key, node->data);
        if (!res) {

            return node;
        }
        if (res < 0) {
            node = node->left;
        } else {
            node = node->right;
        }
    }

    return NULL;
}

int
rbtapply(struct rbt *tree, struct rbtnode *node,
         int (*func)(void *, void *),
         void *cookie,
         int order)
{
    int error;

    if (node != &tree->nil) {
        if (order == RBT_PREORDER) {
            error = func(node->data, cookie);
            if (error) {

                return error;
            }
        }
        error = rbtapply(tree, node->left, func, cookie, order);
        if (error) {

            return error;
        }
        if (order == RBT_INORDER) {
            error = func(node->data, cookie);
            if (error) {

                return error;
            }
        }
        error = rbtapply(tree, node->right, func, cookie, order);
        if (error) {

            return error;
        }
        if (order == RBT_POSTORDER) {
            error = func(node->data, cookie);
            if (error) {

                return error;
            }
        }
    }

    return 0;
}

struct rbtnode *
rbtsuccessor(struct rbt *tree, struct rbtnode *node)
{
    struct rbtnode *succ = node->right;
    struct rbtnode *nil = &tree->nil;

    if (succ != nil) {
        while (succ->left != nil) {
            succ = succ->left;
        }
    } else {
        for (succ = node->parent; node == succ->right ; succ = succ->parent) {
            node = succ;
        }
        if (succ == &tree->root) {
            succ = nil;
        }
    }

    return succ;
}

void
rbtrepair(struct rbt *tree, struct rbtnode *node)
{
    struct rbtnode *sibling;

    while (node->color == RBT_BLACK) {
        if (node == node->parent->left) {
            sibling = node->parent->right;
            if (sibling->color == RBT_RED) {
                sibling->color = RBT_BLACK;
                node->parent->color = RBT_RED;
                rbtrotleft(tree, node->parent);
                sibling = node->parent->right;
            }
            if (sibling->right->color == RBT_BLACK
                &&  sibling->left->color == RBT_BLACK) {
                sibling->color = RBT_RED;
                node = node->parent;
            } else {
                if (sibling->right->color == RBT_BLACK) {
                    sibling->left->color = RBT_BLACK;
                    sibling->color = RBT_RED;
                    rbtrotright(tree, sibling);
                    sibling = node->parent->right;
                }
                sibling->color = node->parent->color;
                node->parent->color = RBT_BLACK;
                sibling->right->color = RBT_BLACK;
                rbtrotleft(tree, node->parent);

                break;
            }
        } else {
            sibling = node->parent->left;
            if (sibling->color == RBT_RED) {
                sibling->color = RBT_BLACK;
                node->parent->color = RBT_RED;
                rbtrotright(tree, node->parent);
                sibling = node->parent->left;
            }
            if (sibling->right->color == RBT_BLACK
                && sibling->left->color == RBT_BLACK) {
                sibling->color = RBT_RED;
                node = node->parent;
            } else {
                if (sibling->left->color == RBT_BLACK) {
                    sibling->right->color = RBT_BLACK;
                    sibling->color = RBT_RED;
                    rbtrotleft(tree, sibling);
                    sibling = node->parent->left;
                }
                sibling->color = node->parent->color;
                node->parent->color = RBT_BLACK;
                sibling->left->color = RBT_BLACK;
                rbtrotright(tree, node->parent);
                
                break;
            }
        }
    }

    return;
}

void
_rbtdestroy(struct rbt *tree, struct rbtnode *node, void (*destroy)(void *))
{
    struct rbtnode *nil = &tree->nil;
    
    if (node != nil) {
        _rbdestroy(tree, node->left, destroy);
        _rbdestroy(tree, node->right, destroy);
        if (destroy) {
            destroy(node->data);
        }
        free(node);
    }

    return;
}

void
rbtdestroy(struct rbt *tree, void (*destroy)(void *))
{
    _rbtdestroy(tree, tree->root.left, destroy);
    free(tree);

    return;
}

void
*rbtdelete(struct rbt *tree, struct rbtnode *node)
{
    struct rbtnode *nil = &tree->nil;
    struct rbtnode *x;
    struct rbtnode *y;
    void           *data = node->data;

    if (node->left == nil || node->right == nil) {
        y = node;
    } else {
        y = rbtsuccessor(tree, node);
    }
    x = (y->left == nil) ? y->right : y->left;
    x->parent = y->parent;
    if (x->parent == &tree->root) {
        tree->root.left = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }
    if (y->color == RBT_BLACK) {
        rbtrepair(tree, x);
    }
    if (y != node) {
        y->left = node->left;
        y->right = node->right;
        y->parent = node->parent;
        y->color = node->color;
        node->left->parent = y;
        node->right->parent = y;
        if (node == node->parent->left) {
            node->parent->left = y;
        } else {
            node->parent->right = y;
        }
    }
    free(node);

    return data;
}

