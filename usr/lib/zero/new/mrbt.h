#ifndef __ZERO_MRBT_H__
#define __ZERO_MRBT_H__

#include <stdint.h>

#define MRBT_DATA_COLOR   1
#define MRBT_UINTPTR_DATA 1
#define MRBT_BLACK        0
#define MRBT_RED          1
#define MRBT_PREORDER     0
#define MRBT_INORDER      1
#define MRBT_POSTORDER    2

#if defined(MRBT_UINTPTR_DATA)
#define mrbtcmpkeys(k1, k2) ((k1) - (k2))
#endif

#if defined(MRBT_DATA_COLOR)
#define MRBT_DATA_COLOR_BIT ((uintptr_t)1 << 0)
#define mrbtsetcolor(np, c) ((np)->data |= (c))
#define mrbtgetcolor(np)    ((np)->data & MRBT_DATA_COLOR_BIT)
#define mrbtclrcolor(np)    ((np)->data & ~MRBT_DATA_COLOR_BIT)
#else
#define mrbtsetcolor(np, c) ((np)->color = (c))
#define mrbtgetcolor(np, c) ((np)->color)
#endif

#define mrbtisempty(tp)                                                 \
    ((tp)->root.left == &(tp)->nil && (tp)->root.right == &(tp)->nil)

struct mrbtitem {
#if defined(MRBT_UINTPTR_DATA)
    volatile long     nref;     // # of allocation references
    uintptr_t         key;      // hash table key
    uintptr_t         data;     // data; value or pointer to custom data
    struct mrbtitem **tab;      // back-pointer to node->items
#else    
    void             *data;
    long              color;
#endif
};

#define MRBT_NITEM     (1L << MRBT_ITEM_NBIT)
#define MRBT_ITEM_NBIT 4
struct mrbtnode {
    uintptr_t        min;       // minimum key stored in items
    uintptr_t        max;       // maximum key stored in items
    size_t           nitembit;  // # of bits in item count
    struct mrbtitem *items;     // table of node entries
    struct mrbt     *tree;      // back pointer to tree structure
    struct mrbtnode *parent;    // parent node
    struct mrbtnode *left;      // left child with keys < min
    struct mrbtnode *right;     // right child with keys > max
};

struct mrbt {
#if !defined(MRBT_UINTPTR_DATA)
    int             (*cmp)(const void *, const void *);
#endif
    struct mrbtnode   root;
    struct mrbtnode   nil;
};

#endif /* __ZERO_MRBT_H__ */

