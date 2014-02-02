#if defined(_REENTRANT)
#include <zero/mtx.h>
#define listlk(lp)   mtxlk(lp)
#define listunlk(lp) mtxunlk(lp)
#else
#define listlk(lp)
#define listunlk(lp)
#endif
#if !defined(LISTPREV)
#define LISTPREV     prev
#endif
#if !defined(LISTNEXT)
#define LISTNEXT     next
#endif

/*
 * Assumptions
 * -----------
 * - LIST_TYPE (struct/union) has members prev and next of LIST_TYPE
 * - LIST_QTYPE has members head and tail of LIST_TYPE
 * - if _REENTRANT is declared and non-zero, LIST_QTYPE has a volatile lk member
 */

/* #define LIST_TYPE  */
/* #define LIST_QTYPE */

/* get item from the head of queue */
#define listpop(queue, retpp)                                           \
    do {                                                                \
        LIST_TYPE *_item1;                                              \
        LIST_TYPE *_item2 = NULL;                                       \
                                                                        \
        listlk(&(queue)->lk);                                           \
        _item1 = (queue)->head;                                         \
        if (_item1) {                                                   \
            _item2 = _item1->LISTNEXT;                                  \
        }                                                               \
        if (_item2) {                                                   \
            _item2->LISTPREV = NULL;                                    \
        } else {                                                        \
            (queue)->tail = NULL;                                       \
        }                                                               \
        (queue)->head = _item2;                                         \
        listunlk(&(queue)->lk);                                         \
        *(retpp) = _item1;                                              \
    } while (0)

/* queue item to the head of queue */
#define listpush(queue, item)                                           \
    do {                                                                \
        LIST_TYPE *_item;                                               \
                                                                        \
        (item)->LISTPREV = NULL;                                        \
        listlk(&(queue)->lk);                                           \
        _item = (queue)->head;                                          \
        if (_item) {                                                    \
            (_item)->LISTPREV = (item);                                 \
        } else {                                                        \
            (queue)->head = (item);                                     \
            (queue)->tail = (item);                                     \
        }                                                               \
        (item)->LISTNEXT = _item;                                       \
        (queue)->head = item;                                           \
        listunlk(&(queue)->lk);                                         \
    } while (0)

/* get item from the tail of queue */
#define listdeq(queue, retpp)                                           \
    do {                                                                \
        LIST_TYPE *_item1;                                              \
        LIST_TYPE *_item2;                                              \
                                                                        \
        listlk(&(queue)->lk);                                           \
        _item1 = (queue)->tail;                                         \
        if (_item1) {                                                   \
            _item2 = _item1->LISTPREV;                                  \
            if (!_item2) {                                              \
                (queue)->head = NULL;                                   \
                (queue)->tail = NULL;                                   \
            } else {                                                    \
                _item2->LISTNEXT = NULL;                                \
                (queue)->tail = _item2;                                 \
            }                                                           \
        }                                                               \
        listunlk(&(queue)->lk);                                         \
        *(retpp) = _item1;                                              \
    } while (0)

/* add block to the tail of queue */
#define listqueue(queue, item)                                          \
    do {                                                                \
        LIST_TYPE *_tail;                                               \
                                                                        \
        (item)->LISTNEXT = NULL;                                        \
        mtxlk(&(queue)->lk);                                            \
        _tail  = (queue)->tail;                                         \
        (item)->LISTPREV = _tail;                                       \
        if (_tail) {                                                    \
            (queue)->tail = (item);                                     \
        } else {                                                        \
            (item)->LISTPREV = NULL;                                    \
            (queue)->head = (item);                                     \
        }                                                               \
        mtxunlk(&(queue)->lk);                                          \
    } while (0)

/* remove item from queue */
#define listrm(queue, item)                                             \
    do {                                                                \
        LIST_TYPE *_tmp;                                                \
                                                                        \
        listlk(&(queue)->lk);                                           \
        _tmp = (item)->LISTPREV;                                        \
        if (_tmp) {                                                     \
            _tmp->LISTNEXT = (item)->LISTNEXT;                                  \
        } else {                                                        \
            _tmp = (item)->LISTNEXT;                                    \
            if (_tmp) {                                                 \
                _tmp->LISTPREV = (item)->LISTPREV;                      \
            } else {                                                    \
                (queue)->tail = NULL;                                   \
            }                                                           \
            (queue)->head = _tmp;                                       \
        }                                                               \
        _tmp = (item)->LISTNEXT;                                        \
        if (_tmp) {                                                     \
            _tmp->prev = (item)->LISTPREV;                              \
        } else {                                                        \
            _tmp = (item)->LISTPREV;                                        \
            if (_tmp) {                                                 \
                _tmp->LISTNEXT = NULL;                                  \
            } else {                                                    \
                (queue)->head = NULL;                                   \
            }                                                           \
            (queue)->tail = _tmp;                                       \
        }                                                               \
        listunlk(&(queue)->lk);                                         \
    } while (0)

