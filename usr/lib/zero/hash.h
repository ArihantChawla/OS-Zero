/* #define HASH_FUNC    - takes one parameter of HASH_TYPE * */
/* #define HASH_CMP     - takes two parameters of HASH_TYPE *; 0 if equal */
/* #define HASH_TYPE    - hash table item type */
/* #define HASH_KEYTYPE - hash key type (such as long) */

#define hashadd(htab, item)                                             \
    do {                                                                \
        HASH_KEYTYPE  _key = HASH_FUNC(item);                           \
        HASH_TYPE    *_item;                                            \
                                                                        \
        (item)->prev = NULL;                                            \
        if (_REENTRANT) {                                               \
            mtxlk(&(htab)->lk);                                         \
        }                                                               \
        _item = (htab)[_key];                                           \
        if (_item) {                                                    \
            _item->prev = item;                                         \
        }                                                               \
        (item)->next = _item;                                           \
        (htab)[_key] = item;                                            \
        if (_REENTRANT) {                                               \
            mtxunlk(&(htab)->lk);                                       \
        }                                                               \
    } while (0)

#define hashfind(htab, item, rpp)                                       \
        _hashsrch(htab, item, rpp, 0)

#define hashrm(htab, item, rpp)                                         \
        _hashsrch(htab, item, rpp, 1)

#define _hashsrch(htab, item, rpp, rm)                                  \
    do {                                                                \
        HASH_KEYTYPE  _key = HASH_FUNC(item);                           \
        HASH_TYPE    *_item = NULL;                                     \
        HASH_TYPE    *_item1;                                           \
        HASH_TYPE    *_item2;                                           \
                                                                        \
        if (_REENTRANT) {                                               \
            mtxlk(&(htab)->lk);                                         \
        }                                                               \
        _item = (htab)[key];                                            \
        while (_item) {                                                 \
            if (!HASH_CMP(_item, item)) {                               \
                if (rm) {                                               \
                    _item1 = _item->prev;                               \
                    _item2 = _item->next;                               \
                    if (_item1) {                                       \
                        _item1->next = _item2;                          \
                    }                                                   \
                    if (_item2) {                                       \
                        _item2->prev = _item1;                          \
                    }                                                   \
                }                                                       \
                                                                        \
                break;                                                  \
            }                                                           \
            _item = _item->next;                                        \
        }                                                               \
        if (_REENTRANT) {                                               \
            mtxunlk(&(htab)->lk);                                       \
        }                                                               \
        *(rpp) = _item;                                                 \
    } while (FALSE)

