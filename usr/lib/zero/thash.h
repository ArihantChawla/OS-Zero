#ifndef __ZERO_THASH_H__
#define __ZERO_THASH_H__

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <zero/asm.h>

#if !defined(THASH_ITEM_T)
#define THASH_ITEM_T struct thashitem
#endif
/* allocation hash entry */
struct thashitem {
    uintptr_t key;
    uintptr_t val;
};

struct thashtab;

/* allocation hash-table queue structure; cache-friendly */
struct thashtabhdr {
    m_atomic_t       n;
    struct thashtab *next;
};

#define THASH_TAB_ITEMS       31
#define THASH_LIST_LK_BIT     ((m_atomic_t)1 << THASH_LIST_LK_BIT_POS)
#define THASH_LIST_LK_BIT_POS 0
struct thashtab {
    struct thashtabhdr hdr;
    THASH_ITEM_T       list[THASH_TAB_ITEMS];
};

#define thashtrylklist(cptr) \
    (!m_cmpsetbit((m_atomic_t *)cptr, THASH_LIST_LK_BIT_POS))

static __inline__ struct thashtab *
thashlklist(struct thashtab **cptr)
{
    struct thashtab *tab = NULL;
    uintptr_t        uptr;

    do {
        while ((uintptr_t)*cptr & THASH_LIST_LK_BIT) {
            m_spinwait();
        }
        if (thashtrylklist(cptr)) {
            uptr = (uintptr_t)*cptr;
            tab = (void *)(uptr & ~THASH_LIST_LK_BIT);

            return tab;
        }
    } while (1);

    /* NOTREACHED */
    return NULL;
}

#define thashunlklist(cptr)                                            \
    (m_clrbit((m_atomic_t *)cptr, THASH_LIST_LK_BIT_POS))

#define thashalloctab() calloc(1, sizeof(struct thashtab))

static __inline__ void
thashadd(struct thashtab **tab, void *ptr, uintptr_t val)
{
    unsigned long    hkey = THASH_FUNC(ptr);
    long             ndx;
    struct thashtab *slot;
    THASH_ITEM_T    *item;

    slot = thashlklist(&tab[hkey]);
    if (!slot) {
        slot = thashalloctab();
        if (!slot) {
            fprintf(stderr, "THASH: failed to allocate table\n");

            exit(1);
        }
        tab[hkey] = (void *)((uintptr_t)slot | THASH_LIST_LK_BIT);
    }
    while (slot) {
        ndx = m_fetchadd((m_atomic_t *)&slot->hdr.n, 1);
        if (ndx < THASH_TAB_ITEMS) {
            item = &slot->list[ndx];
            item->key = ptr;
            item->val = val;

            break;
        } else {
            m_fetchadd((m_atomic_t *)&slot->hdr.n, -1);
            slot = slot->hdr.next;

            continue;
        }
    }
    thashunlklist(&tab[hkey]);

    return;
}

static __inline__ uintptr_t
thashchk(struct thashtab **tab, uintptr_t ptr, long del)
{
    unsigned long    hkey = THASH_FUNC(ptr);
    uintptr_t        val = THASH_VAL_NONE;
    struct thashtab *slot;
    THASH_ITEM_T    *item;
    long             n;
    long             ndx;
    long             src;

    slot = thashlklist(&tab[hkey]);
    if (slot) {
        do {
            ndx = 0;
            n = slot->hdr.n;
            while (ndx < n) {
                item = &slot->list[ndx];
                if (item->key == ptr) {
                    val = item->val;
                    if (del) {
                        src = m_fetchadd(&slot->hdr.n, -1);
                        if (!src) {
                            /* TODO: deallocate table */
                        } else {
                            slot->list[ndx] = slot->list[src];
                            slot->list[src].key = ptr;
                            slot->list[src].val = THASH_VAL_NONE;
                        }
                    }

                    return val;
                }
            }
            slot = slot->hdr.next;
        } while (slot);
    }
    thashunlklist(&tab[hkey]);

    return val;
}

#define thashfind(tab, ptr) thashchk(tab, ptr, 0)
#define thashdel(tab, ptr)  thashchk(tab, ptr, 1)

#endif /* __ZERO_THASH_H__ */

