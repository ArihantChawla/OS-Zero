/* implementation of B-tree for libzero */

#ifndef __ZERO_BTREE_H__
#define __ZERO_BTREE_H__

/* TODO: implementation of binary trees as structure tables */

#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
/* FIXME: use lock-free approach later on */
#include <zero/mtx.h>

#define BTREE_NODE_LISTS 3
#if !defined(BTREE_LIST_ITEMS)
#define BTREE_LIST_ITEMS 7
#endif
#define BTREE_NODE_ITEMS (BTREE_NODE_LISTS * BTREE_LIST_ITEMS)

/* allocate uninitialised memory */
#if !defined(BTREE_MALLOC)
#define BTREE_MALLOC(sz)    malloc(sz)
#endif
/* allocate zeroed memory */
#if !defined(BTREE_CALLOC)
#define BTREE_CALLOC(n, sz) calloc(n, sz)
#endif
/* release memory */
#if !defined(BTREE_FREE)
#define BTREE_FREE(ptr)     free(ptr)
#endif

#if !defined(BTREE_KEY_T)
#define BTREE_KEY_T  intptr_t
#endif
#if !defined (BTREE_DATA_T)
#define BTREE_DATA_T void *
#endif
#if !defined(BTREE_ITEM_T)
#define BTREE_ITEM_T struct btreeitem
struct btreeitem {
    BTREE_KEY_T  key;
    BTREE_DATA_T data;
};
#endif
#if !defined(BTREE_RANGE_T)
#define BTREE_RANGET_T struct btreerange
struct btreerange {
    BTREE_KEY_T lokey;
    BTREE_KEY_T hikey;
};
#endif

#define BTREE_LEFT_LIST   0
#define BTREE_MIDDLE_LIST 1
#define BTREE_RIGHT_LIST  2
/* tree node structure */
#define BTREE_NODE struct btreenode
struct btreenode {
    unsigned long bits;
    BTREE_KEY_T   nleft;
    BTREE_KEY_T   nmid;
    BTREE_KEY_T   nright;
    BTREE_RANGE_T limtab[BTREE_NODE_LISTS];
    BTREE_ITEM_T *tab[BTREE_NODE_ITEMS];
};

#define BTREE_T struct btree
struct btree {
    BTREE_NODE_T root;
};

#endif /* __ZERO_BTREE_H__ */

