#ifndef __ZERO_LKPOOL_H__
#define __ZERO_LKPOOL_H__

#include <stdint.h>
#include <mach/param.h>
#include <mach/atomic.h>
#include <mach/asm.h>
#include <mt/mtx.h>
#include <zero/trix.h>
#include <zero/hash.h>
#include <zero/qrand.h>

#define LKPOOLLOCKS 8

/* make mutex size equal cacheline */
struct lkpoolmtx {
    m_atomic_t lk;
    int8_t     _pad[CLSIZE - sizeof(m_atomic_t)];
};

/*
 * distribute load somewhat-randomly over LKPOOLLOCKS mutexes instead of a
 * single one
 */
struct lkpool {
    struct lkpoolmtx lktab[LKPOOLLOCKS];;
};

/* scan pool for an unacquired lock, start at random offset into lktab */
static INLINE long
lkpooltrylk(struct lkpool *pool)
{
    struct lkpoolmtx *lpmtx = &pool->lktab[0];
    long              n;
    long              ofs;

    n = LKPOOLLOCKS;
    ofs = qrand32();
    while (n--) {
        ofs &= LKPOOLLOCKS - 1;
        if (lpmtx[ofs].lk == FMTXINITVAL) {
            if (fmtxtrylk(&lpmtx[ofs].lk)) {

                return ofs;
            }
        }
        m_waitspin();
    }

    return -1;
}

/* acquire lock from pool */
static __inline__ long
lkpoollk(struct lkpool *pool)
{
    long res = -1;

    do {
        res = lkpooltrylk(pool);
    } while (res < 0);

    return res;
}

/* unlock lock in pool */
#define lkpoolunlk(pp, ofs) fmtxunlk(&(pp)->lktab[(ofs)].lk)

#endif /* __ZERO_LKPOOL_H__ */

