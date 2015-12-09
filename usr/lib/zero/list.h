/*
 * assumptions
 * -----------
 * - LIST_TYPE has items prev and next of LIST_TYPE *
 */

#include <stddef.h>
#include <zero/cdefs.h>

/* #define LIST_TYPE */

/* initialise list */
#define listinit(item)                                                  \
    ((item)->next = (item), (item)->prev = (item))
/* add item between prev and next */
#define __listadd(item, item1, item2)                                   \
    ((item2)->prev = (item),                                            \
     (item)->next = (item2),                                            \
     (item)->prev = (item1),                                            \
     (item1)->next = (item))
/* add item1 after item2 */
#define listaddafter(item1, item2)                                      \
    __listadd(item1, item2, (item2)->next)
/* add item1 before item2 */
#define listaddbefore(item1, item2)                                     \
    __listadd(item1, (item2)->prev, item2)
/* remove item from between prev and next */
#define listdelbetween(item1, item2)                                    \
    ((item2)->prev = item1, (item1)->next = item2)
/* remove item */
#define listdel(item)                                                   \
    listdelbetween(item->prev, item->next)
/* replace item1 with item2 */
#define listreplace(item1, item2)                                       \
    ((item2)->next = item1->next,                                       \
     (item2)->next->prev = (item2),                                     \
     (item2)->prev = (item1)->prev,                                     \
     (item2)->prev->next = (item2))
/* replace item1 with item2, initialise new list with item1 */
#define listreplaceinit(item1, item2)                                   \
    (listreplace(item1, item2), listinit(item1))
/* remove item1 from list, initialise new list with it */
#define listdelinit(item1)                                              \
    (listdel(item1), listinit(item1))
/* remove item1 from list, add it after another list's item2 */
#define listmoveafter(item1, item2)                                     \
    (listdel(item1), listaddafter(item1, item2))
/* remove item1 from list, add it before another list's item2 */
#define listmovebefore(item1, item2)                                    \
    (listdel(item1), listaddbefore(item1, item2))
/* check if item is the last one on list */
#define listislast(item, head)                                          \
    ((item)->next == head)
/* check if list is empty */
#define listisempty(item)                                               \
    ((item)->next == item)
/* rotate list to the left */
#define listrotleft(item)                                               \
    do {                                                                \
        LIST_TYPE *_next;                                               \
                                                                        \
        if (!listisempty(item)) {                                       \
            _next = (item)->next;                                       \
            listmovebefore(_next, item);                                \
        }                                                               \
    } while (0)
/* check if list has just one item */
#define listissingular(item)                                            \
    (listisempty(item) && (item)->next == (item)->prev)
#define __listcutpos(list, item, entry)                                 \
    do {                                                                \
        LIST_TYPE *_next = (entry)->next;                               \
                                                                        \
        (list)->next = (item)->next;                                    \
        (list)->next->prev = list;                                      \
        (list)->prev = entry;                                           \
        (entry)->next = list;                                           \
        (item)->next = _next;                                           \
        _next->prev = item;                                             \
    } while (0)
/* cut list in two if entry != item */
#define listcutpos(list, item, entry)                                   \
    do {                                                                \
        if (!listisempty(item)                                          \
            && !listissingular(item)                                    \
            && (item)->next != (entry)                                  \
            && (item) != (entry)) {                                     \
            if ((item) == (entry)) {                                    \
                listinit(list);                                         \
            } else {                                                    \
                __listcutpos(list, item, entry);                        \
            }                                                           \
        } while (0)
#define __listsplice(list, item1, item2)                                \
    do {                                                                \
        LIST_TYPE *_first = (list)->next;                               \
        LIST_TYPE *_last = (list)->prev;                                \
                                                                        \
        _first->prev = prev;                                            \
        (item1)->next = _first;                                         \
        _last->next = next;                                             \
        (item2)->prev = _last;                                          \
    } while (0)
/* join two lists by adding list after item */
#define listsplice(list, item)                                          \
    do {                                                                \
        if (!listisempty(list)) {                                       \
            __listsplice(list, item, (item)->next);                     \
        }                                                               \
    } while (0)
/* join two lists by adding list before item */
#define listsplicetail(list, item)                                      \
    do {                                                                \
        if (!listisempty(list)) {                                       \
            __listsplice(list, (item)->prev, item);                     \
        }                                                               \
    } while (0)
/* join two lists and reinitialise the emptied one */
#define listspliceinit(list, item)                                      \
    do {                                                                \
        if (!listisempty(list)) {                                       \
            __listsplice(list, item, (item)->next);                     \
            listinit(list);                                             \
        }                                                               \
    } while (0)
#define listsplicetailinit(list, item)                                  \
    do {                                                                \
        if (!listisempty(list)) {                                       \
            __listsplice(list, (item)->prev, item);                     \
            listinit(list);                                             \
        }                                                               \
    } while (0)
/* get struct for list entry */
#define listentry(list, type, member)                                   \
    containerof(list, type, member)
/* get first element from list */
#define listfirstentry(list, type, member)                              \
    listentry((list)->next, type, member)
/* get last element from list */
#define listlastentry(list, type, member)                               \
    listentry((list)->prev, type, member)
/* get first element from list or NULL if list is empty */
#define listfirstentryornull(list, type, member)                        \
    (!listempty(list) ? listfirst(list, type, member) : NULL)
/* get next element in list */
#define listnextentry(pos, member)                                      \
    listentry((pos)->member.next, typeof(*(pos)), member)
/* get next previous in list */
#define listpreventry(pos, member)                                      \
    listentry((pos)->member.prev, typeof(*(pos)), member)
/* iterate over a list */
#define listiter(pos, item)                                             \
    for (pos = (item)->next ; (pos) != (item) ; pos = (pos)->next)
/* iterate over a list backwards */
#define listiterrev(pos, item)                                          \
    for (pos = (item)->prev ; (pos) != (item) ; pos = (pos)->prev)
/* iterate over a list; entries won't be removed */
#define listitersafe(pos, tmp, item)                                    \
    for (pos = (item)->next, tmp = (pos)->next ;                        \
         (pos) != (item) ;                                              \
         (pos) = (tmp), (tmp) = (pos)->next)
#define listiterrevsafe(pos, tmp, item)                                 \
    for (pos = (item)->prev, tmp = (pos)->prev ;                        \
         (pos) != (item) ;                                              \
         (pos) = (tmp), (tmp) = (pos)->prev)
#define listiterentry(pos, item, member)                                \
    for (pos = listfirst(item, typeof(*(pos)), member) ;                \
         &(pos)->member != (item) ;                                     \
         pos = listnextentry(pos, member))
#define listiterentryrev(pos, item, member)                             \
    for (pos = listlast(item, typeof(*(pos)), member) ;                 \
         &(pos)->member != (item) ;                                     \
         pos = listpreventry(pos, member))
/* prepare pos entry for listitercont */
#define listprepentry(pos, item, member)                                \
    ((pos) ? : listentry(item, typeof(*(pos)), member))
#define listiterentrycont(pos, item, member)                            \
    for (pos = listnextentry(pos, member) ;                             \
         &(pos)->member != (item) ;                                     \
         pos = listnextentry(pos, member))
#define listiterentrycontrev(pos, item, member)                         \
    for (pos = listpreventry(pos, member) ;                             \
         &(pos)->member != (item) ;                                     \
         pos = listpreventry(pos, member))
#define listiterentryfrom(pos, item, member)                            \
    for ( ;                                                             \
          &(pos)->member != (item) ;                                    \
          pos = listnextentry(item))
#define listiterentryfromrev(pos, item, member)                         \
    for ( ;                                                             \
          &(pos)->member != (item) ;                                    \
          pos = listpreventry(item))
/* entries won't be removed */
#define listiterentrysafe(pos, tmp, item, member)                       \
    for (pos = listfirstentry(item, typeof(*(pos)), member),            \
             tmp = listnextentry(pos, member) ;                         \
         &(pos)->member != (head) ;                                     \
         pos = tmp, tmp = listnextentry(tmp, member))
#define listiterentrysafecont(pos, tmp, item, member)                   \
    for (pos = listnextentry(item, member),                             \
             tmp = listnextentry(pos, member) ;                         \
         &(pos)->member != (head) ;                                     \
         pos = tmp, tmp = listnextentry(tmp, member))
#define listiterentrysafecontrev(pos, tmp, item, member)                \
    for (pos = listpreventry(item, member),                             \
             tmp = listpreventry(pos, member) ;                         \
         &(pos)->member != (head) ;                                     \
         pos = tmp, tmp = listpreventry(tmp, member))
#define listiterentrysafefrom(pos, tmp, head, member)                   \
    for (tmp = listnextentry(pos, member) ;                             \
         &(pos)->member != (head) ;                                     \
         pos = tmp, tmp = listnextentry(tmp, member))
#define listiterentrysafefromrev(pos, tmp, item, member)                \
    for (pos = listlasttentry(item, typeof(*(pos)), member),            \
             tmp = listpreventry(pos, member) ;                         \
         &(pos)->member != (head) ;                                     \
         pos = tmp, tmp = listpreventry(tmp, member))
/* reset stale list for safe loops */
#define listsaferesetnext(pos, tmp, member)                             \
    (tmp = listnextentry(pos, member))

