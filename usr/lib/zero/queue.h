#define queueinit(item)                                                 \
    ((item)->next = (item), (item)->prev = (item))
#define queueisempty(item)                                              \
    ((item)->next == item)
#define queueissingular(item)                                           \
    (queueisempty(item) && (item)->next == (item)->prev)

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

