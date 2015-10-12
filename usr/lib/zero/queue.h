#include <stddef.h>

#undef queueinit
#undef queueissingular
#if !defined(QUEUE_PREFIX)
#define QUEUE_PREFIX
#endif

#if defined(QUEUE_SINGLE_TYPE)

#define queueinit(item)                                                 \
    ((item)->next = (item), (item)->prev = (item))
#define queueissingular(item)                                           \
    ((item)->next == (item)->prev)

/* add item to beginning of queue */
static __inline__ void
queuepush(QUEUE_TYPE *item, QUEUE_TYPE **queue)
{
    QUEUE_TYPE *list = *queue;

    if (list) {
        list->prev->next = item;
        item->prev = list->prev;
        item->next = list;
        list->prev = item;
    } else {
        queueinit(item);
        *queue = item;
    }

    return;
}

/* get/remove item from beginning of queue */
static __inline__ QUEUE_TYPE *
queuepop(QUEUE_TYPE **queue)
{
    QUEUE_TYPE *item = *queue;

    if (item) {
        if (!queueissingular(item)) {
            item->prev->next = item->next;
            item->next->prev = item->prev;
            *queue = item->next;
        } else {
            *queue = NULL;
        }
    }

    return item;
}

/* add item to end of queue */
static __inline__ void
queueappend(QUEUE_TYPE *item, QUEUE_TYPE **queue)
{
    QUEUE_TYPE *list = *queue;

    if (list) {
        list->prev->next = item;
        item->prev = list->prev;
        item->next = list;
        list->prev = item;
    } else {
        queueinit(item);
        *queue = item;
    }

    return;
}

/* get/remove item from end of queue */
static __inline__ QUEUE_TYPE *
queuegetlast(QUEUE_TYPE **queue)
{
    QUEUE_TYPE *list = *queue;
    QUEUE_TYPE *item = NULL;

    if (list) {
        if (!queueissingular(list)) {
            item = list->prev;
            item->prev->next = item->next;
            item->next->prev = item->prev;
            list->prev = item->prev;
        } else {
            item = list;
            *queue = NULL;
        }
    }

    return item;
}

/* remove item from queue */
static __inline__ void
queuermitem(QUEUE_TYPE *item, QUEUE_TYPE **queue)
{
    QUEUE_TYPE *list = *queue;
    
    if (!queueissingular(item)) {
        item->prev->next = item->next;
        item->next->prev = item->prev;
        if (item == list) {
            *queue = item->next;
        }
    } else if (item == list) {
        *queue = NULL;
    }
}

#else /* !defined(QUEUE_SINGLE_TYPE) */

#define queueinit(item, queue)                                          \
    ((*queue)->next = (item), (*queue)->prev = (item))
#define queueissingular(list)  ((list)->next == (list)->prev)

/* add item to beginning of queue */
static __inline__ void
queuepush(QUEUE_ITEM_TYPE *item, QUEUE_TYPE **queue)
{
    QUEUE_ITEM_TYPE *list;

    if (*queue) {
        list = (*queue)->next;
        if (list) {
            list->prev = item;
            item->prev = (*queue)->prev;
            (*queue)->next = item;
        } else {
            queueinit(item, queue);
        }
    }

    return;
}

/* get/remove item from beginning of queue */
static __inline__ QUEUE_ITEM_TYPE *
queuepop(QUEUE_TYPE **queue)
{
    QUEUE_ITEM_TYPE *item = NULL;

    if (*queue) {
        item = (*queue)->next;
        if (item) {
            if (!queueissingular(item)) {
                (*queue)->prev->next = item->next;
                (*queue)->next = item->next;
            } else {
                *queue = NULL;
            }
        }
    }

    return item;
}

/* add item to end of queue */
static __inline__ void
queueappend(QUEUE_ITEM_TYPE *item, QUEUE_TYPE **queue)
{
    QUEUE_ITEM_TYPE *list;

    if (*queue) {
        list = (*queue)->prev;
        if (list) {
            item->prev = list;
            list->next = item;
            (*queue)->prev = item;
        } else {
            queueinit(item, queue);
        }
    } else {
        queueinit(item, queue);
    }

    return;
}

/* get/remove item from end of queue */
static __inline__ QUEUE_ITEM_TYPE *
queuegetlast(QUEUE_TYPE **queue)
{
    QUEUE_ITEM_TYPE *item = NULL;

    if (*queue) {
        item = (*queue)->prev;
        if (!queueissingular(item)) {
            item->prev->next = item->next;
            item->next->prev = item->prev;
        } else {
            *queue = NULL;
        }
    }

    return item;
}

/* remove item from queue */
static __inline__ void
queuermitem(QUEUE_ITEM_TYPE *item, QUEUE_TYPE **queue)
{
    QUEUE_ITEM_TYPE *list = (*queue)->next;
    
    if (!queueissingular(list)) {
        item->prev->next = item->next;
        item->next->prev = item->prev;
        if (item == list) {
            list = item->next;
        }
    } else {
        *queue = NULL;
    }

    return;
}

#endif /* QUEUE_SINGLE_TYPE */

