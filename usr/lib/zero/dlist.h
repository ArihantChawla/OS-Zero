#ifndef __ZERO_DLIST_H__
#define __ZERO_DLIST_H__

#include <stddef.h>
#include <zero/cdecl.h>

/* list stuff based on what I found in Linux */

/* REFERENCE: http://lxr.free-electrons.com/source/include/linux/list.h */

struct dlist {
    struct dlist *prev;
    struct dlist *next;
};

/* initialise list */
#define DLIST(name) { &(name), &(name) }
#define dlistinit(item)                                                 \
    ((item)->next = (item), (item)->prev = (item))
/* add item between prev and next */
#define __dlistadd(item, prev, next)                                    \
    ((next)->prev = (item),                                             \
     (item)->next = next,                                               \
     (item)->prev = (prev),                                             \
     (prev)->next = (item))
/* add item1 after item2 */
#define dlistaddafter(item1, item2)                                     \
    __dlistadd(item1, item2, (item2)->next)
/* add item1 before item2 */
#define dlistaddbefore(item1, item2)                                    \
    __dlistadd(item1, (item2)->prev, item2)
/* remove item from between prev and next */
#define dlistdelbetween(prev, next)                                     \
    ((next)->prev = prev, (prev)->next = next)
/* remove item */
#define dlistdel(item)                                                  \
    listdelbetween((item)->prev, (item)->next)
/* replace item1 with item2 */
#define dlistreplace(item1, item2)                                      \
    ((item2)->next = (item1)->next,                                     \
     (item2)->next->prev = (item2),                                     \
     (item2)->prev = (item1)->prev,                                     \
     (item2)->prev->next = (item2))
/* replace item1 with item2, initialise new list with item1 */
#define dlistreplaceinit(item1, item2)                                  \
    (listreplace(item1, item2), listinit(item1))
/* remove item1 from list, initialise new list with it */
#define dlistdelinit(item1)                                             \
    (listdel(item1), listinit(item1))
/* remove item1 from list, add it after another list's item2 */
#define dlistmoveafter(item1, item2)                                    \
    (listdel(item1), listaddafter(item1, item2))
/* remove item1 from list, add it before another list's item2 */
#define dlistmovebefore(item1, item2)                                    \
    (listdel(item1), listaddbefore(item1, item2))
/* check if item is the last one on list */
#define dlistislast(item, head)                                         \
    ((item)->next == head)
/* check if list is empty */
#define dlistisempty(item)                                              \
    ((item)->next == item)
/* rotate list to the left */
#define dlistrotleft(item)                                              \
    do {                                                                \
        struct dlist *_next;                                            \
                                                                        \
        if (!listisempty(item)) {                                       \
            _next = (item)->next;                                       \
            listmovebefore(_next, item);                                \
        }                                                               \
    } while (0)
/* check if list has just one item */
#define dlistissingular(item)                                           \
    (listisempty(item) && (item)->next == (item)->prev)
#define __dlistcutpos(list, item, entry)                                \
    do {                                                                \
        struct dlist *_next = (entry)->next;                            \
                                                                        \
        (list)->next = (item)->next;                                    \
        (list)->next->prev = list;                                      \
        (list)->prev = entry;                                           \
        (entry)->next = list;                                           \
        (item)->next = _next;                                           \
        _next->prev = item;                                             \
    } while (0)
/* cut list in two if entry != item */
#define dlistcutpos(list, item, entry)                                  \
    do {                                                                \
        if (!listisempty(item)                                          \
            && !listissingular(item)                                    \
            && (item)->next != (entry)                                  \
            && (item) != (entry)) {                                     \
            if ((item) == (entry)) {                                    \
                listinit(list);                                         \
            } else {                                                    \
                __dlistcutpos(list, item, entry);                       \
            }                                                           \
        } while (0)
#define __dlistsplice(list, prev, next)                                 \
    do {                                                                \
        struct dlist *_first = (list)->next;                            \
        struct dlist *_last = (list)->prev;                             \
                                                                        \
        _first->prev = prev;                                            \
        (prev)->next = _first;                                          \
        _last->next = next;                                             \
        (next)->prev = _last;                                           \
    } while (0)
/* join two lists by adding list after item */
#define dlistsplice(list, item)                                         \
    do {                                                                \
        if (!listisempty(list)) {                                       \
            __dlistsplice(list, item, (item)->next);                    \
        }                                                               \
    } while (0)
/* join two lists by adding list before item */
#define dlistsplicetail(list, item)                                     \
    do {                                                                \
        if (!listisempty(list)) {                                       \
            __dlistsplice(list, (item)->prev, item);                    \
        }                                                               \
    } while (0)
/* join two lists and reinitialise the emptied one */
#define dlistspliceinit(list, item)                                     \
    do {                                                                \
        if (!listisempty(list)) {                                       \
            __dlistsplice(list, item, (item)->next);                    \
            listinit(list);                                             \
        }                                                               \
    } while (0)
#define dlistsplicetailinit(list, item)                                 \
    do {                                                                \
        if (!listisempty(list)) {                                       \
            __dlistsplice(list, (item)->prev, item);                    \
            listinit(list);                                             \
        }                                                               \
    } while (0)
/* get struct for list entry */
#define dlistentry(list, type, member)                                  \
    containerof(list, type, member)
/* get first element from list */
#define dlistfirstentry(list, type, member)                             \
    listentry((list)->next, type, member)
/* get last element from list */
#define dlistlastentry(list, type, member)                              \
    listentry((list)->prev, type, member)
/* get first element from list or NULL if list is empty */
#define dlistfirstentryornull(list, type, member)                       \
    (!listempty(list) ? listfirst(list, type, member) : NULL)
/* get next element in list */
#define dlistnextentry(pos, member)                                     \
    listentry((pos)->member.next, typeof(*(pos)), member)
/* get next previous in list */
#define dlistpreventry(pos, member)                                     \
    listentry((pos)->member.prev, typeof(*(pos)), member)
/* iterate over a list */
#define dlistiter(pos, item)                                            \
    for (pos = (item)->next ; (pos) != (item) ; pos = (pos)->next)
/* iterate over a list backwards */
#define dlistiterrev(pos, item)                                         \
    for (pos = (item)->prev ; (pos) != (item) ; pos = (pos)->prev)
/* iterate over a list; entries won't be removed */
#define dlistitersafe(pos, tmp, item)                                   \
    for (pos = (item)->next, tmp = (pos)->next ;                        \
         (pos) != (item) ;                                              \
         (pos) = (tmp), (tmp) = (pos)->next)
#define dlistiterrevsafe(pos, tmp, item)                                \
    for (pos = (item)->prev, tmp = (pos)->prev ;                        \
         (pos) != (item) ;                                              \
         (pos) = (tmp), (tmp) = (pos)->prev)
#define dlistiterentry(pos, item, member)                               \
    for (pos = listfirst(item, typeof(*(pos)), member) ;                \
         &(pos)->member != (item) ;                                     \
         pos = listnextentry(pos, member))
#define dlistiterentryrev(pos, item, member)                            \
    for (pos = listlast(item, typeof(*(pos)), member) ;                 \
         &(pos)->member != (item) ;                                     \
         pos = listpreventry(pos, member))
/* prepare pos entry for listitercont */
#define dlistprepentry(pos, item, member)                               \
    ((pos) ? : listentry(item, typeof(*(pos)), member))
#define dlistiterentrycont(pos, item, member)                           \
    for (pos = listnextentry(pos, member) ;                             \
         &(pos)->member != (item) ;                                     \
         pos = listnextentry(pos, member))
#define dlistiterentrycontrev(pos, item, member)                        \
    for (pos = listpreventry(pos, member) ;                             \
         &(pos)->member != (item) ;                                     \
         pos = listpreventry(pos, member))
#define dlistiterentryfrom(pos, item, member)                           \
    for ( ;                                                             \
          &(pos)->member != (item) ;                                    \
          pos = listnextentry(item))
#define dlistiterentryfromrev(pos, item, member)                        \
    for ( ;                                                             \
          &(pos)->member != (item) ;                                    \
          pos = listpreventry(item))
/* entries won't be removed */
#define dlistiterentrysafe(pos, tmp, item, member)                      \
    for (pos = listfirstentry(item, typeof(*(pos)), member),            \
             tmp = listnextentry(pos, member) ;                         \
         &(pos)->member != (head) ;                                     \
         pos = tmp, tmp = listnextentry(tmp, member))
#define dlistiterentrysafecont(pos, tmp, item, member)                  \
    for (pos = listnextentry(item, member),                             \
             tmp = listnextentry(pos, member) ;                         \
         &(pos)->member != (head) ;                                     \
         pos = tmp, tmp = listnextentry(tmp, member))
#define dlistiterentrysafecontrev(pos, tmp, item, member)               \
    for (pos = listpreventry(item, member),                             \
             tmp = listpreventry(pos, member) ;                         \
         &(pos)->member != (head) ;                                     \
         pos = tmp, tmp = listpreventry(tmp, member))
#define dlistiterentrysafefrom(pos, tmp, head, member)                  \
    for (tmp = listnextentry(pos, member) ;                             \
         &(pos)->member != (head) ;                                     \
         pos = tmp, tmp = listnextentry(tmp, member))
#define dlistiterentrysafefromrev(pos, tmp, item, member)               \
    for (pos = listlasttentry(item, typeof(*(pos)), member),            \
             tmp = listpreventry(pos, member) ;                         \
         &(pos)->member != (head) ;                                     \
         pos = tmp, tmp = listpreventry(tmp, member))
/* reset stale list for safe loops */
#define dlistsaferesetnext(pos, tmp, member)                            \
    (tmp = listnextentry(pos, member))

#endif /* __ZERO_DLIST_H__ */

