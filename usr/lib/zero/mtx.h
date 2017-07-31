#ifndef __ZERO_MTX_H__
#define __ZERO_MTX_H__

#if !defined(ZEROMTX)
#define ZEROMTX     1
#endif

#include <stddef.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/asm.h>
#include <zero/thr.h>

/*
 * Special thanks to Matthew 'kinetik' Gregan for help with the mutex code.
 * :)
 */

#if defined(__KERNEL__)
#undef PTHREAD
#endif

#include <zero/asm.h>
//#if defined(__KERNEL__) && (__MTKERNEL__)
#if !defined(__KERNEL__)
#if !defined(PTHREAD) && defined(__linux__)
#include <sched.h>
#endif
#endif /* !defined(__KERNEL__) */

#if (defined(__KERNEL__) || defined(ZEROFMTX))
typedef m_atomic_t      zerofmtx;
#elif defined(PTHREAD) && defined(ZEROPTHREAD)
typedef pthread_mutex_t zerofmtx;
#endif

#if (defined(__KERNEL__) || defined(ZEROFMTX))

//#include <zero/thr.h>

#define FMTXINITVAL 0
#define FMTXLKVAL   1
#if defined(ZERONEWFMTX)
#define FMTXCONTVAL 2
#endif

#define fmtxinit(lp) (*(lp) = FMTXINITVAL)
#define fmtxfree(lp) /* no-op */

#if (ZERONEWFMTX)

/*
 * try to acquire fast mutex lock
 * - return non-zero on success, zero if already locked
 */
static INLINE long
fmtxtrylk(m_atomic_t *lp)
{
    m_atomic_t res = *lp;

    if (res == FMTXINITVAL) {
        res = m_cmpswap(lp, FMTXINITVAL, FMTXLKVAL);
    }

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
        res = *lp;
        if (res == FMTXINITVAL) {
            res = m_cmpswap(lp, FMTXINITVAL, FMTXLKVAL);
        } else {
            res = 0;
        }
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
    m_endspin();
    
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
    m_endspin();
    
    return;
}

#endif

#define zerotrylkfmtx(mp) fmtxtrylk(mp)
#define zerolkfmtx(mp)    fmtxlk(mp)
#define zerounlkfmtx(mp)  fmtxunlk(mp)

#endif /* ZEROFMTX */

#if defined(PTHREAD) && !defined(ZEROPTHREAD)

#define MTXINITVAL PTHREAD_MUTEX_INITIALIZER

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

#elif defined(ZEROMTX)

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
    m_atomic_t val;     // owner for recursive mutexes, 0 if unlocked
    m_atomic_t cnt;     // access counter
    m_atomic_t rec;     // recursion depth
    zeromtxatr atr;
    uint8_t    _pad[CLSIZE - 3 * sizeof(long) - sizeof(zeromtxatr)];
} zeromtx;

#endif

#endif /* __ZERO_MTX_H__ */

