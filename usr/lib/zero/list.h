#ifndef __ZERO_LIST_H__
#define __ZERO_LIST_H__

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
        if (_REENTRANT) {                                               \
            mtxlk(&(queue)->lk);                                        \
        }                                                               \
        _item1 = (queue)->head;                                         \
        if (_item1) {                                                   \
            _item2 = _item1->next;                                      \
        }                                                               \
        if (_item2) {                                                   \
            _item2->prev = NULL;                                        \
        } else {                                                        \
            (queue)->tail = NULL;                                       \
        }                                                               \
        (queue)->head = _item2;                                         \
        if (_REENTRANT) {                                               \
            mtxunlk(&(queue)->lk);                                      \
        }                                                               \
        *(retpp) = _item1;                                              \
    } while (0)

/* queue item to the head of queue */
#define listpush(queue, item)                                           \
    do {                                                                \
        LIST_TYPE *_item;                                               \
                                                                        \
        (item)->prev = NULL;                                            \
        if (_REENTRANT) {                                               \
            mtxlk(&(queue)->lk);                                        \
        }                                                               \
        _item = (queue)->head;                                          \
        if (_item) {                                                    \
            (_item)->prev = (item);                                     \
        } else {                                                        \
            (queue)->head = (item);                                     \
            (queue)->tail = (item);                                     \
        }                                                               \
        item->next = _item;                                             \
        (queue)->head = item;                                           \
        if (_REENTRANT) {                                               \
            mtxunlk(&(queue)->lk);                                      \
        }                                                               \
    } while (0)

/* get item from the tail of queue */
#define listdeq(queue, retpp)                                           \
    do {                                                                \
        LIST_TYPE *_item1;                                              \
        LIST_TYPE *_item2;                                              \
                                                                        \
        if (_REENTRANT) {                                               \
            mtxlk(&(queue)->lk);                                        \
        }                                                               \
        _item1 = (queue)->tail;                                         \
        if (_item1) {                                                   \
            _item2 = _item1->prev;                                      \
            if (!_item2) {                                              \
                (queue)->head = NULL;                                   \
                (queue)->tail = NULL;                                   \
            } else {                                                    \
                _item2->next = NULL;                                    \
                (queue)->tail = _item2;                                 \
            }                                                           \
        }                                                               \
        if (_REENTRANT) {                                               \
            mtxunlk(&(queue)->lk);                                      \
        }                                                               \
        *(retpp) = _item1;                                              \
    } while (0)

/* remove item from queue */
#define listrm(queue, item)                                             \
    do {                                                                \
        LIST_TYPE *_tmp;                                                \
                                                                        \
        if (_REENTRANT) {                                               \
            mtxlk(&queue->lk);                                          \
        }                                                               \
        _tmp = (item)->prev;                                            \
        if (_tmp) {                                                     \
            _tmp->next = (item)->next;                                  \
        } else {                                                        \
            _tmp = (item)->next;                                        \
            (queue)->head = _tmp;                                       \
            if (_tmp) {                                                 \
                _tmp->prev = (item)->prev;                              \
            } else {                                                    \
                (queue)->tail = _tmp;                                   \
            }                                                           \
            (queue)->head = _tmp;                                       \
        }                                                               \
        _tmp = (item)->next;                                            \
        if (_tmp) {                                                     \
            _tmp->prev = (item)->prev;                                  \
        } else {                                                        \
            _tmp = (item)->prev;                                        \
            (queue)->tail = _tmp;                                       \
            if (_tmp) {                                                 \
                _tmp->next = NULL;                                      \
            } else {                                                    \
                (queue)->head = _tmp;                                   \
                (queue)->tail = _tmp;                                   \
            }                                                           \
        }                                                               \
        if (_REENTRANT) {                                               \
            mtxunlk(&(queue)-lk);                                       \
        }                                                               \
    } while (0)

#endif /* __ZERO_LIST_H__ */

