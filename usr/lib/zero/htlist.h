#if defined(HTLISTNOLOCK) || !defined(_REENTRANT)
#define htlistlk(lp)
#define htlistunlk(lp)
#else
#include <zero/mtx.h>
#define htlistlk(lp)   mtxlk(lp)
#define htlistunlk(lp) mtxunlk(lp)
#endif
#if !defined(HTLISTPREV)
#define HTLISTPREV     prev
#endif
#if !defined(HTLISTNEXT)
#define HTLISTNEXT     next
#endif
#if !defined(HTLIST_QUEUE)
#define HTLIST_QUEUE(dummy)
#endif
#if !defined(HTLIST_DEQUEUE)
#define HTLIST_DEQUEUE(dummy)
#endif
#if !defined(HTLIST_RM_COND)
#define HTLIST_RM_COND(dummy)
#endif

/*
 * Assumptions
 * -----------
 * - HTLIST_TYPE (struct/union) has members prev and next of HTLIST_TYPE
 * - HTLIST_QTYPE has members head and tail of HTLIST_TYPE
 * - if _REENTRANT is declared and non-zero, HTLIST_QTYPE has a volatile lk member
 */

/* #define HTLIST_TYPE  */
/* #define HTLIST_QTYPE */

#define htlistnotempty(queue) (queue->head)

/* get item from the head of queue */
#define htlistpop(queue, retpp)                                         \
    do {                                                                \
        HTLIST_TYPE *_item1;                                            \
        HTLIST_TYPE *_item2 = NULL;                                     \
                                                                        \
        htlistlk(&(queue)->lk);                                         \
        _item1 = (queue)->head;                                         \
        if (_item1) {                                                   \
            _item2 = _item1->HTLISTNEXT;                                \
        }                                                               \
        if (_item2) {                                                   \
            _item2->HTLISTPREV = NULL;                                  \
        } else {                                                        \
            (queue)->tail = NULL;                                       \
        }                                                               \
        (queue)->head = _item2;                                         \
        HTLIST_DEQUEUE(_item1);                                         \
        htlistunlk(&(queue)->lk);                                       \
        *(retpp) = _item1;                                              \
    } while (0)

/* queue item to the head of queue */
#define htlistpush(queue, item)                                         \
    do {                                                                \
        HTLIST_TYPE *_item;                                             \
                                                                        \
        (item)->HTLISTPREV = NULL;                                      \
        htlistlk(&(queue)->lk);                                         \
        _item = (queue)->head;                                          \
        if (_item) {                                                    \
            (_item)->HTLISTPREV = (item);                               \
        } else {                                                        \
            (queue)->head = (item);                                     \
            (queue)->tail = (item);                                     \
        }                                                               \
        (item)->HTLISTNEXT = _item;                                     \
        (queue)->head = item;                                           \
        HTLIST_QUEUE(item);                                             \
        htlistunlk(&(queue)->lk);                                       \
    } while (0)

/* get item from the tail of queue */
#define htlistdequeue(queue, retpp)                                     \
    do {                                                                \
        HTLIST_TYPE *_item1;                                            \
        HTLIST_TYPE *_item2;                                            \
                                                                        \
        htlistlk(&(queue)->lk);                                         \
        _item1 = (queue)->tail;                                         \
        if (_item1) {                                                   \
            _item2 = _item1->HTLISTPREV;                                \
            if (!_item2) {                                              \
                (queue)->head = NULL;                                   \
                (queue)->tail = NULL;                                   \
            } else {                                                    \
                _item2->HTLISTNEXT = NULL;                              \
                (queue)->tail = _item2;                                 \
            }                                                           \
        }                                                               \
        HTLIST_DEQUEUE(_item1);                                         \
        htlistunlk(&(queue)->lk);                                       \
        *(retpp) = _item1;                                              \
    } while (0)

/* add block to the tail of queue */
#define htlistqueue(queue, item)                                        \
    do {                                                                \
        htlist_TYPE *_tail;                                             \
                                                                        \
        (item)->HTLISTNEXT = NULL;                                      \
        mtxlk(&(queue)->lk);                                            \
        _tail  = (queue)->tail;                                         \
        (item)->HTLISTPREV = _tail;                                     \
        if (_tail) {                                                    \
            (queue)->tail = (item);                                     \
        } else {                                                        \
            (item)->HTLISTPREV = NULL;                                  \
            (queue)->head = (item);                                     \
        }                                                               \
        HTLIST_QUEUE(item);                                             \
        mtxunlk(&(queue)->lk);                                          \
    } while (0)

/* remove item from queue */
#define htlistrm(queue, item)                                           \
    do {                                                                \
        HTLIST_TYPE *_tmp;                                              \
                                                                        \
        htlistlk(&(queue)->lk);                                         \
        if (HTLIST_RM_COND(item)) {                                     \
            _tmp = (item)->HTLISTPREV;                                  \
            if (_tmp) {                                                 \
                _tmp->HTLISTNEXT = (item)->HTLISTNEXT;                  \
            } else {                                                    \
                _tmp = (item)->HTLISTNEXT;                              \
                if (_tmp) {                                             \
                    _tmp->HTLISTPREV = (item)->HTLISTPREV;              \
                } else {                                                \
                    (queue)->tail = NULL;                               \
                }                                                       \
                (queue)->head = _tmp;                                   \
            }                                                           \
            _tmp = (item)->HTLISTNEXT;                                  \
            if (_tmp) {                                                 \
                _tmp->prev = (item)->HTLISTPREV;                        \
            } else {                                                    \
                _tmp = (item)->HTLISTPREV;                              \
                if (_tmp) {                                             \
                    _tmp->HTLISTNEXT = NULL;                            \
                } else {                                                \
                    (queue)->head = NULL;                               \
                }                                                       \
                (queue)->tail = _tmp;                                   \
            }                                                           \
            HTLIST_DEQUEUE(item);                                       \
        }                                                               \
        htlistunlk(&(queue)->lk);                                       \
    } while (0)

