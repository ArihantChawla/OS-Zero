#ifndef __MT_MTX_H__
#define __MT_MTX_H__

#define ZEROMTX     1
#if !defined(ZEROFMTX)
#define ZEROFMTX    1
#endif
#if !defined(ZERONEWFMTX)
#define ZERONEWFMTX 1
#endif

#include <zero/conf.h>
#include <stddef.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <mach/types.h>
#include <mach/asm.h>

typedef volatile m_atomic_t zerofmtx;

/*
 * Special thanks to Matthew 'kinetik'
 *Gregan for help with the mutex code.:)
 */

#if defined(PTHREAD) && defined(ZEROPTHREAD)
typedef pthread_mutex_t     zeromtx;
#endif

#define FMTXINITVAL MTXINITVAL
#define FMTXLKVAL   MTXLKVAL
#if defined(ZERONEWFMTX)
#define FMTXCONTVAL MTXCONTVAL
#endif

#define fmtxinit(lp) (*(lp) = FMTXINITVAL)
#define fmtxfree(lp) /* no-op */

#define MTXINITVAL            0
#define MTXLKVAL              1
#define MTXCONTVAL            2

/* initializer for non-dynamic attributes */
#define ZEROMTXATR_DEFVAL     { 0L }
/* flags for attribute flg-field */
#define ZEROMTX_RECURSIVE     (1L << 0)
#define ZEROMTX_DETACHED      (1L << 1)
/* private flg-bits */
#define __ZEROMTXATR_DYNAMIC  (1L << 30)
#define __ZEROMTXATR_INIT     (1L << 31)
/* error codes */
#define ZEROMTXATR_NOTDYNAMIC 1
typedef struct zeromtxatr {
    long flg;           // feature flag-bits
} zeromtxatr;

/* initializer for non-dynamic mutexes */
#define ZEROMTX_INITVAL { ZEROMTXFREE, 0, 0, ZEROMTXATRDEFVAL }
/* thr for unlocked mutexes */
#define ZEROMTX_FREE    0
typedef struct zeromtx {
    volatile m_atomic_t val; // owner for recursive mutexes, 0 if unlocked
    volatile m_atomic_t cnt;// access counter
    volatile m_atomic_t rec; // recursion depth
    zeromtxatr atr;
    uint8_t    _pad[CLSIZE - 3 * sizeof(long) - sizeof(zeromtxatr)];
} zeromtx;

#if (ZERONEWFMTX)

/*
 * try to acquire fast mutex lock
 * - return non-zero on success, zero if already locked
 */
static INLINE long
fmtxtrylk(m_atomic_t *lp)
{
    long res = m_cmpswap(lp, FMTXINITVAL, FMTXLKVAL);

    return res;
}

/*
 * - acquire fast mutex lock
 * - spin on volatile lock to avoid excess lock-operations
 */
static INLINE void
fmtxlk(m_atomic_t *lp)
{
    m_atomic_t res;

    do {
        do {
            res = *lp;
        } while (res);
        res = m_cmpswap(lp, FMTXINITVAL, FMTXLKVAL);
        if (!res) {
            /* do a "light-weight busy-wait" */
            m_waitspin();
        }
    } while (!res);

    return;
}

/*
 * unlock fast mutex
 * - must use full memory barrier to guarantee proper write-ordering
 */
static INLINE void
fmtxunlk(m_atomic_t *lp)
{
    m_membar();
    *lp = FMTXINITVAL;

    return;
}

#else

/*
 * try to acquire fast mutex lock
 * - return non-zero on success, zero if already locked
 */
static INLINE long
fmtxtrylk(m_atomic_t *lp)
{
    m_atomic_t res;

    res = m_cmpswap(lp, FMTXINITVAL, FMTXLKVAL);

    return res;
}

/*
 * acquire fast mutex lock
 * - allow other threads to run when blocking
 */
static INLINE void
fmtxlk(m_atomic_t *lp)
{
    m_atomic_t res;

    do {
        res = m_cmpswap(lp, FMTXINITVAL, FMTXLKVAL);
        if (!res) {
            m_waitspin();
        }
    } while (!res);

    return;
}

/*
 * unlock fast mutex
 * - must use full memory barrier to guarantee proper write-ordering
 */
static INLINE void
fmtxunlk(m_atomic_t *lp)
{
    m_membar();
    *lp = FMTXINITVAL;

    return;
}

#endif

#define zerotrylkfmtx(mp) fmtxtrylk(mp)
#define zerolkfmtx(mp)    fmtxlk(mp)
#define zerounlkfmtx(mp)  fmtxunlk(mp)

#if defined(ZERO_THREADS) || defined(ZERO_MUTEX)

#define zerotrylkmtx(mp) fmtxtrylk
#define zerolkmtx(mp)    fmtxtlk
#define zerounlkmtx(mp)  fmtxunlk

#elif defined(PTHREAD) && !defined(ZEROPTHREAD)

#define PTHREAD_MUTEX_INITIALIZER MTXINITVAL
#define PTHREAD_FMTX_INITIALIZER  FMTXINITVAL

#define zerotrylkmtx(mp) pthread_mutex_trylock(mp)
#define zerolkmtx(mp)    pthread_mutex_lock(mp)
#define zerounlkmtx(mp)  pthread_mutex_unlock(mp)

#elif defined(ZEROPTHREAD) && !defined(PTHREAD_MUTEX_INITIALIZER)

#define PTHREAD_MUTEX_INITIALIZER MTXINITVAL

#define pthread_mutex_init(mp, atr) mtxinit(mp, atr)
#define pthread_mutex_destroy(mp)   mtxfree(mp)
#define pthread_mutex_trylock(mp)   mtxtrylk(mp)
#define pthread_mutex_lock(mp)      mtxlk(mp)
#define pthread_mutex_unlock(mp)    mtxunlk(mp)

#endif

#endif /* __MT_MTX_H__ */

