#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <mach/atomic.h>
#include <mach/asm.h>
#include <zero/trix.h>
#include <zero/hash.h>

#if !defined(TABHASH_VAL_NONE)
#define TABHASH_VAL_NONE 0
#endif

/*
 * TABHASH_FUNC may be #defined before including this header
 */
#if !defined(TABHASH_FUNC)
#define TABHASH_FUNC(val) tmhash32(val & 0xffffffff)
#endif

/* allocation hash-table queue structure; cache-friendly */
struct tabhashhdr {
    m_atomic_t  n;
    void       *chain;
};

/* allocation hash data entry */
#if !defined(TABHASH_ITEM_T)
#define TABHASH_ITEM_T struct tabhashitem
#endif
struct tabhashitem {
    uintptr_t key;
    uintptr_t val;
};

/*
 * allocation chain table buffer; should consume a few aligned cachelines and
 * is loaded in a few fetches only to avoid cache-trashing
 */
#define TABHASH_TAB_ITEMS      31
#define TABHASH_BUF_LK_BIT     (1L << TABHASH_BUF_LK_BIT_POS)
#define TABHASH_BUF_LK_BIT_POS 0
struct tabhashbuf {
    struct tabhashhdr hdr;
    TABHASH_ITEM_T    tab[TABHASH_TAB_ITEMS];
};

/*
 * acquire chain/buffer lock by locking the lowest bit of a pointer
 */
#define tabhashlkbit(buf)   (!m_cmpsetbit((m_atomic_t *)buf,            \
                                          TABHASH_BUF_LK_BIT_POS))
#define __tabhashlkbuf(buf) tabhashlkbit(buf)
static __inline__ void
tabhashlkbuf(struct tabhashbuf **buf)
{
    do {
        if (!((uintptr_t)*buf & TABHASH_BUF_LK_BIT)) {
            if (__tabhashlkbuf(buf)) {

                return;
            }
        }
        m_waitspin();
    } while (1);

    /* NOTREACHED */
    return;
}

/*
 * release lock/low bit of a pointer
 */
static __inline__ void
tabhashunlkbuf(struct tabhashbuf **buf)
{
    m_clrbit((m_atomic_t *)buf, TABHASH_BUF_LK_BIT_POS);
    m_endspin();

    return;
}

#define TABHASH_REMOVE -1
#define TABHASH_SEARCH  0

/* convenience macros */
/* allocate new hash chain table */
#define tabhashalloctab() calloc(1, sizeof(struct tabhashbuf))
/* find hash entry, return pointer but do not remove it */
#define tabhashfind(tab, key) tabhashsearch(tab, key, TAB_HASH_SEARCH)
/* find and remove a hash entry */
#define tabhashdel(tab, key)  tabhashsearch(tab, key, TAB_HASH_REMOVE)

