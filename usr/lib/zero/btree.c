/*
 * API
 * ---
 * - ptr btreeins(tree, val);
 * - ptr btreefind(tree, key);
 * - ptr btreedel(tree, key);
 *
 * INTERNAL
 * --------
 * - void _btreebal()
 */

#include <zero/btree.h>

/* insert node with val into tree, associate it with key */
static BTREE_ITEM_T *
btreeins(BTREE_T *tree, BTREE_ITEM_T *item)
{
    ;
}

static struct btreeitem *
btreefind(struct btree *tree, BTREE_KEY_T key)
{
    ;
}

/* delete node associated with key from tree */
static BTREE_ITEM_T
btreedel(struct btree *tree, BTREE_KEY_T key)
{
    ;
}

