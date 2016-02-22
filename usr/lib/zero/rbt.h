#ifndef __ZERO_RBT_H__
#define __ZERO_RBT_H__

#define RBT_BLACK     0
#define RBT_RED       1

#define RBT_PREORDER  0
#define RBT_INORDER   1
#define RBT_POSTORDER 2

#define rbtisempty(t) ((t)->root.left == &(t)->nil                      \
                       && (t)->root.right == &(t)->nil)

struct rbtnode {
    void           *data;
    long            color;
    struct rbtnode *parent;
    struct rbtnode *left;
    struct rbtnode *right;
};

struct rbt;

struct rbt {
    int            (*cmp)(const void *, const void *);
    struct rbtnode   root;
    struct rbtnode   nil;
};

#endif /* __ZERO_RBT_H__ */

