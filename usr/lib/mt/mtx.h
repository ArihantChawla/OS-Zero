#ifndef __MT_MTX_H__
#define __MT_MTX_H__

#define ZEROMTX     1
#if !defined(ZEROFMTX)
#define ZEROFMTX    1
#endif
#if !defined(ZERONEWFMTX)
#define ZERONEWFMTX 1
#endif

//#include <mt/conf.h>
#include <stddef.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <mach/asm.h>

typedef m_atomic_t zerofmtx;

/*
 * Special thanks to Matthew 'kinetik'
 *Gregan for help with the mutex code.:)
 */

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
typedef struct __zeromtxatr {
    long flg; // feature flag-bits
} zeromtxatr;

/* initializer for non-dynamic mutexes */
#define ZEROMTX_INITVAL { ZEROMTXFREE, 0, 0, ZEROMTXATRDEFVAL }
/* thr for unlocked mutexes */
#define ZEROMTX_FREE    0
struct __zeromtx {
    m_atomic_t val; // owner for recursive mutexes, 0 if unlocked
    m_atomic_t cnt; // access counter
    m_atomic_t rec; // recursion depth
    zeromtxatr atr;
    uint8_t    _pad[CLSIZE - 3 * sizeof(m_atomic_t) - sizeof(zeromtxatr)];
};
typedef volatile struct __zeromtx zeromtx;

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
    long res;

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
    long res;

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

#if defined(ZERO_THREAD) || defined(ZERO_MUTEX)
#define zerotrylkmtx(mp)  fmtxtrylk
#define zerolkmtx(mp)     fmtxtlk
#define zerounlkmtx(mp)   fmtxunlk
#endif

#endif /* __MT_MTX_H__ */

