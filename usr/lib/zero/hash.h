#include <stdlib.h>
#if defined(_REENTRANT)
#include <zero/mtx.h>
#define hashlk(lp)   mtxlk(lp)
#define hashunlk(lp) mtxunlk(lp)
#else
#define hashlk(lp)
#define hashunlk(lp)
#endif

/* #define HASH_FUNC    - takes one parameter of HASH_TYPE * */
/* #define HASH_CMP     - takes two parameters of HASH_TYPE *; 0 if equal */
/* #define HASH_TYPE    - hash table item type */
/* #define HASH_KEYTYPE - hash key type (such as long) */

/*
 * Example
 * -------
 *
 * struct hash {
 * #if (_REENTRANT)
 *     volatile long  lk;
 * #endif
 *     HASH_TYPE     *tab;
 * };
 *
 * typedef long HASH_KEYTYPE;
 *
 * typedef struct {
 *     HASH_KEYTYPE  key;
 *     HASH_TYPE    *prev;
 *     HASH_TYPE    *next;
 * } HASH_TYPE;
 *
 * #define HASH_TABSZ         65536
 * #define HASH_FUNC(ip)      ((ip)->key & 0xffff)
 * #define HASH_CMP(ip1, ip2) (!((ip1)->key == (ip2)->key))
 * #define HASH_ALLOC(n, sz)  calloc(n, sz)
 */

#if !defined(HASH_ALLOC)
#define HASH_ALLOC(n, sz)     calloc(n, sz)
#endif

#define hashinit()                                                      \
    do {                                                                \
        HASH_TYPE *_hash = HASH_ALLOC(1, sizeof(HASH_TYPE));            \
                                                                        \
        if (_hash) {                                                    \
            _hash->tab = HASH_ALLOC(HASH_TABSZ, sizeof(HASH_TYPE));     \
        }                                                               \
    } while (0)

#define hashadd(hash, item)                                             \
    do {                                                                \
        HASH_KEYTYPE  _key = HASH_FUNC(item);                           \
        HASH_TYPE    *_item;                                            \
                                                                        \
        (item)->HASHPREV = NULL;                                        \
        hashlk(&(hash)->lk);                                            \
        _item = (hash)->tab[_key];                                      \
        if (_item) {                                                    \
            _item->HASHPREV = item;                                     \
        }                                                               \
        (item)->HASHNEXT = _item;                                       \
        (hash)->tab[_key] = item;                                       \
        hashunlk(&(hash)->lk);                                          \
    } while (0)

#define hashfind(hash, item, rpp)                                       \
        _hashsrch(hash, item, rpp, 0)

#define hashrm(hash, item, rpp)                                         \
        _hashsrch(hash, item, rpp, 1)

#define _hashsrch(hash, item, rpp, rm)                                  \
    do {                                                                \
        HASH_KEYTYPE  _key = HASH_FUNC(item);                           \
        HASH_TYPE    *_item = NULL;                                     \
        HASH_TYPE    *_item1;                                           \
        HASH_TYPE    *_item2;                                           \
                                                                        \
        hashlk(&(hash)->lk);                                            \
        _item = (hash)->tab[key];                                       \
        while (_item) {                                                 \
            if (!HASH_CMP(_item, item)) {                               \
                if (rm) {                                               \
                    _item1 = _item->HASHPREV;                           \
                    _item2 = _item->HASHNEXT;                           \
                    if (_item1) {                                       \
                        _item1->HASHNEXT = _item2;                      \
                    }                                                   \
                    if (_item2) {                                       \
                        _item2->HASHPREV = _item1;                      \
                    }                                                   \
                }                                                       \
                                                                        \
                break;                                                  \
            }                                                           \
            _item = _item->HASHNEXT;                                    \
        }                                                               \
        hashunlk(&(hash)->lk);                                          \
        *(rpp) = _item;                                                 \
    } while (FALSE)

