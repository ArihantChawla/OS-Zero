#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <mach/atomic.h>
#include <mach/asm.h>
#include <zero/trix.h>

/* allocation hash-table queue structure; cache-friendly */
struct tabhashhdr {
    m_atomic_t     n;
    volatile void *chain;
};

#if !defined(TABHASH_ITEM_T)
#define TABHASH_ITEM_T struct tabhashitem
#endif
/* allocation hash entry */
struct tabhashitem {
    uintptr_t key;
    uintptr_t val;
};

//struct tabhashbuf;

#define TABHASH_TAB_ITEMS      31
#define TABHASH_BUF_LK_BIT     (1L << TABHASH_BUF_LK_BIT_POS)
#define TABHASH_BUF_LK_BIT_POS 0
struct tabhashbuf {
    struct tabhashhdr hdr;
    TABHASH_ITEM_T    tab[TABHASH_TAB_ITEMS];
};

static __inline__ void
tabhashlkbuf(struct tabhashbuf **buf)
{
    do {
        if (!((uintptr_t)*buf & TABHASH_BUF_LK_BIT)) {
            if (!m_cmpsetbit((m_atomic_t *)buf, TABHASH_BUF_LK_BIT_POS)) {

                return;
            }
        }
        m_waitspin();
    } while (1);

    /* NOTREACHED */
    return NULL;
}

static __inline__ void
tabhashunlkbuf(struct tabhashbuf **buf)
{
    m_clrbit((m_atomic_t *)buf, TABHASH_BUF_LK_BIT_POS);
    m_endspin();

    return;
}

#define tabhashalloctab() calloc(1, sizeof(struct tabhashbuf))

#define TABHASH_REMOVE -1
#define TABHASH_SEARCH  0
static __inline__ uintptr_t
tabhashop(struct tabhashbuf **hash, uintptr_t key, long op)
{
    unsigned long      hkey = TABHASH_FUNC(key);
    uintptr_t          val;
    struct tabhashbuf *buf;
    struct tabhashbuf *bptr;
    TABHASH_ITEM_T    *item;
    uintptr_t          mask;
    m_atomic_t         src;
    m_atomic_t         n;
    long               ni;
    long               ndx;

    tabhashlkbuf(&hash[hkey]);
    buf = (void *)((uintptr_t)hash[hkey] & ~TABHASH_BUF_LK_BIT);
    if (buf) {
        do {
            n = buf->hdr.n;
            bptr = buf->tab;
            do {
                ni = min(n, 8);
                if (ni) {
                    /*
                     * - if item found, the mask will be -1; all 1-bits), and val
                     *   will be the item address
                     * - if not found, the mask will be 0 and so will val
                     */
                    val = 0;
                    switch (ni) {
                        default:
                        case 8:
                            item = &bptr[7];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 7:
                            item = &bptr[6];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 6:
                            item = &bptr[5];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 5:
                            item = &bptr[4];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 4:
                            item = &bptr[3];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 3:
                            item = &bptr[2];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 2:
                            item = &bptr[1];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 1:
                            item = &bptr[0];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 0:

                            break;
                    }
                    if (val) {
                        item = (TABHASH_ITEM_T *)val;
                        if (op == TABHASH_REMOVE) {
                            src = buf->hdr.n;
                            n--;
                            ndx = item - &buf->tab[0];
                            if (!n) {
                                /* TODO: deallocate table */
                            } else if (ndx != src) {
                                buf->tab[ndx] = buf->tab[src];
                                buf->tab[src].key = key;
                                buf->tab[src].val = TABHASH_VAL_NONE;
                            } else {
                                buf->tab[ndx].key = 0;
                                buf->tab[ndx].val = TABHASH_VAL_NONE;
                            }
                            buf->hdr.n = n;
                        }
                        tabhashunlkbuf(&hash[hkey]);

                        return val;
                    }
                    bptr += ni;
                    n -= ni;
                } else {
                    ndx = 0;
                    buf = buf->hdr.chain;
                }
            } while (ni);
        } while (buf);
        tabhashunlkbuf(&hash[hkey]);
    }

    return val;
}

#define tabhashfind(tab, key) tabhashchk(tab, key, 0)
#define tabhashdel(tab, key)  tabhashchk(tab, key, 1)

