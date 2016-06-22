#ifndef __ZERO_BTREE_H__
#define __ZERO_BTREE_H__

/* latch1: intent, delete */
/* latch2: read, write */
/* latch3: parentmod */
/* update/insert/delete: writelk */

#define BTREE_DELETED_BIT (1 << 0)
struct btreenode {
    volatile long     lk;
    long              hikey;
    long              level;
    long              flg;
    struct btreenode *child1;
    struct btreenode *child2;
    struct btreenode *child3;
    struct btreenode *parent;
    struct btreenode *right;
};

#endif /*  __ZERO_BTREE_H__ */

