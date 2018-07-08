#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <mach/asm.h>
#include <zero/trix.h>

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
            m_waitspin();
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
thashadd(struct thashtab **tab, uintptr_t key, uintptr_t val)
{
    unsigned long    hkey = THASH_FUNC(key);
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
            item->key = key;
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
thashchk(struct thashtab **tab, uintptr_t key, long del)
{
    unsigned long    hkey = THASH_FUNC(key);
    uintptr_t        val = THASH_VAL_NONE;
    struct thashtab *slot;
    THASH_ITEM_T    *item;
    uintptr_t        mask;
    long             n;
    long             ni;
    long             ndx;
    long             src;

    slot = thashlklist(&tab[hkey]);
    if (slot) {
        n = slot->hdr.n;
        do {
            ni = min(n, 8);
            if (ni) {
                /*
                 * - if item found, the mask will be -1; all 1-bits), and val
                 *   will be the item address
                 * - if not found, the mask will be 0 and so will val/slot
                 */
                val = 0;
                switch (ni) {
                    case 8:
                        item = &slot->list[7];
                        mask = -(item->key == key);
                        mask &= (uintptr_t)item;
                        val |= mask;
                    case 7:
                        item = &slot->list[6];
                        mask = -(item->key == key);
                        mask &= (uintptr_t)item;
                        val |= mask;
                    case 6:
                        item = &slot->list[5];
                        mask = -(item->key == key);
                        mask &= (uintptr_t)item;
                        val |= mask;
                    case 5:
                        item = &slot->list[4];
                        mask = -(item->key == key);
                        mask &= (uintptr_t)item;
                        val |= mask;
                    case 4:
                        item = &slot->list[3];
                        mask = -(item->key == key);
                        mask &= (uintptr_t)item;
                        val |= mask;
                    case 3:
                        item = &slot->list[2];
                        mask = -(item->key == key);
                        mask &= (uintptr_t)item;
                        val |= mask;
                    case 2:
                        item = &slot->list[1];
                        mask = -(item->key == key);
                        mask &= (uintptr_t)item;
                        val |= mask;
                    case 1:
                        item = &slot->list[0];
                        mask = -(item->key == key);
                        mask &= (uintptr_t)item;
                        val |= mask;
                    default:

                        break;
                }
                if (val) {
                    item = (THASH_ITEM_T *)val;
                    if (del) {
                        n--;
                        ndx = item - &slot->list[0];
                        if (!n) {
                            /* TODO: deallocate table */
                        } else {
                            slot->list[ndx] = slot->list[src];
                            slot->list[src].key = key;
                            slot->list[src].val = THASH_VAL_NONE;
                            slot->hdr.n = n;
                        }
                    }

                    return val;
                } else {
                    slot += ni;
                    n -= ni;
                }
            } else {
                ndx = 0;
                slot = slot->hdr.next;
            }
        } while (slot);
        thashunlklist(&tab[hkey]);
    }

    return val;
}

#define thashfind(tab, key) thashchk(tab, key, 0)
#define thashdel(tab, key)  thashchk(tab, key, 1)

