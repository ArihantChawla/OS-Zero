#include <stdint.h>
#include <mach/atomic.h>
#include <zero/tabhash.h>

/* add item { key, val } into hash table */
static __inline__ void
tabhashadd(struct tabhashbuf **hash, uintptr_t key, uintptr_t val)
{
    unsigned long      hkey = TABHASH_FUNC(key);
    struct tabhashbuf *buf;
    TABHASH_ITEM_T    *item;
    m_atomic_t         n;
    long               ni;

    tabhashlkbuf(&hash[key]);
    buf = (void *)((uintptr_t)&hash[key] & ~TABHASH_BUF_LK_BIT);
    if (buf) {
        /* scan existing chain tables for an empty slot */
        while (buf) {
            n = buf->hdr.n;
            item = &buf->tab[n];
            if (n < TABHASH_TAB_ITEMS) {
                buf->hdr.n++;
                item->key = key;
                item->val = val;

                break;
            } else {
                buf = buf->hdr.chain;
            }
        }
        /* if empty slot not found, allocate new table and push it to chain */
        if (!buf) {
            buf = tabhashalloctab();
            if (!buf) {
                fprintf(stderr, "TABHASH: failed to allocate teble\n");

                exit(1);
            }
            item = &buf->tab[0];
            item->key = key;
            item->val = val;
            buf->hdr.chain = buf;
            m_atomwrite(&hash[hkey], buf);
        }
    }
}

/*
 * array hash, chains tables of items instead of single ones to avoid excess
 * pointer data dependencies/chase; if item is found and op is TABHASH_REMOVE,
 * the object is removed from the hash table
 */
static __inline__ uintptr_t
tabhashsearch(struct tabhashbuf **hash, uintptr_t key, long op)
{
    unsigned long      hkey = TABHASH_FUNC(key);
    uintptr_t          val;
    struct tabhashbuf *buf;
    TABHASH_ITEM_T    *item;
    TABHASH_ITEM_T    *iptr;
    uintptr_t          mask;
    m_atomic_t         src;
    m_atomic_t         n;
    long               ni;
    long               ndx;

    tabhashlkbuf(&hash[hkey]);
    buf = (void *)((uintptr_t)&hash[hkey] & ~TABHASH_BUF_LK_BIT);
    if (buf) {
        do {
            n = buf->hdr.n;
            iptr = buf->tab;
            do {
                ni = min(n, 8);
                if (ni) {
                    /*
                     * - if item found, the mask will be -1; all 1-bits),
                     *   and val will be the item address
                     * - if not found, the mask will be 0 and so will val
                     */
                    val = 0;
                    switch (ni) {
                        default:
                        case 8:
                            item = &iptr[7];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 7:
                            item = &iptr[6];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 6:
                            item = &iptr[5];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 5:
                            item = &iptr[4];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 4:
                            item = &iptr[3];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 3:
                            item = &iptr[2];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 2:
                            item = &iptr[1];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 1:
                            item = &iptr[0];
                            mask = -(item->key == key);
                            mask &= (uintptr_t)item;
                            val |= mask;
                        case 0:

                            break;
                    }
                    if (val) {
                        item = (TABHASH_ITEM_T *)val;
                        if (op == TABHASH_REMOVE) {
                            n--;
                            src = n;
                            ndx = item - &buf->tab[0];
                            if (!n) {
                                /* TODO: deallocate table or keep it */
                            } else if (ndx != src) {
                                buf->tab[ndx] = buf->tab[src];
                                buf->tab[src].key = key;
                                buf->tab[src].val = TABHASH_VAL_NONE;
                            } else {
                                buf->tab[ndx].key = 0;
                                buf->tab[ndx].val = TABHASH_VAL_NONE;
                            }
                            buf->hdr.n = n;
                            /*
                             * TODO: implement add to front caching of items
                             */
                        }
                        tabhashunlkbuf(&hash[hkey]);

                        return val;
                    }
                    iptr += ni;
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

