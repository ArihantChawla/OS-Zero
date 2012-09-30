/*
 * Copyright (C) 2008-2012 Tuomo Petteri Venäläinen. All rights reserved.
 *
 * See the file LICENSE for more information about using this software.
 */

/*
 *        malloc buffer layers
 *        --------------------
 *
 *                --------
 *                | mag  |
 *                --------
 *                    |
 *                --------
 *                | slab |
 *                --------
 *        --------  |  |   -------
 *        | heap |--|  |---| map |
 *        --------         -------
 *
 *        mag
 *        ---
 *        - magazine cache with allocation stack of pointers into the slab
 *          - LIFO to reuse freed blocks of virtual memory
 *
 *        slab
 *        ----
 *        - slab allocator bottom layer
 *        - power-of-two size slab allocations
 *          - supports both heap and mapped regions
 *
 *        heap
 *        ----
 *        - process heap segment
 *          - sbrk() interface; needs global lock
 *
 *        map
 *        ---
 *        - process map segment
 *          - mmap() interface; thread-safe
 */

#define NEWHACK 1
#define FIXES   1
#define HACKS   0

#define INTSTAT 0
#define ZEROMTX 1
#define STAT    0

#define SPINLK  0
/* NOT sure if FreeBSD still needs spinlocks */
#if defined(__FreeBSD__)
#undef SPINLK
#define SPINLK  1
#endif

#ifdef _REENTRANT
#ifndef MTSAFE
#define MTSAFE  1
#endif

/*
 * TODO
 * ----
 * - tune nmbuf() and other behavior
 * - implement mallopt()
 * - improve fault handling
 */

/*
 * THANKS
 * ------
 * - Matthew 'kinetik' Gregan for pointing out bugs, giving me cool routines to
 *   find more of them, and all the constructive criticism etc.
 * - Thomas 'Freaky' Hurst for patience with early crashes, 64-bit hints, and
 *   helping me find some bottlenecks.
 * - Henry 'froggey' Harrington for helping me fix issues on AMD64.
 * - Dale 'swishy' Anderson for the enthusiasism, encouragement, and everything
 *   else.
 * - Martin 'bluet' Stensgård for an account on an AMD64 system for testing
 *   earlier versions.
 */

#include <features.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define SBRK_FAILED ((void *)-1L)

static void   initmall(void);
static void   relarn(void *arg);
static void * getmem(size_t size, size_t align, long zero);
static void   putmem(void *ptr);
static void * _realloc(void *ptr, size_t size, long rel);

/* red-zones haven't been implemented completely yet... some bugs. */
#define RZSZ     0
#define markred(p) (*(uint64_t *)(p) = UINT64_C(0xb4b4b4b4b4b4b4b4))
#define chkred(p)                                                       \
    ((*(uint64_t *)(p) == UINT64_C(0xb4b4b4b4b4b4b4b4))                 \
     ? 0                                                                \
     : 1)

#define LKDBG    0
#define SYSDBG   0
#define VALGRIND 0

#include <string.h>
#if (MTSAFE)
#define PTHREAD  1
#include <pthread.h>
#endif
#endif
#if (ZEROMTX)
#include <zero/mtx.h>
typedef volatile long   LK_T;
#elif (SPINLK)
#include <zero/spin.h>
typedef volatile long   LK_T;
#elif (PTHREAD)
typedef pthread_mutex_t LK_T;
#endif
#if (VALGRIND)
#include <valgrind/valgrind.h>
#endif
#include <zero/param.h>
#include <zero/cdecl.h>
//#include <mach/mach.h>
#include <zero/trix.h>
#include <zero/unix.h>
//#include <mach/param.h>

/* experimental */
#if (PTRBITS > 32)
#define TUNEBUF 1
#else
#define TUNEBUF 0
#endif

/* basic allocator parameters */
#if (NEWHACK)
#define BLKMINLOG2    5  /* minimum-size allocation */
#define SLABTEENYLOG2 14 /* little block */
#define SLABTINYLOG2  17 /* small-size block */
#define SLABLOG2      22 /* base size for heap allocations */
#define MAPMIDLOG2    24
#else
#define BLKMINLOG2    5  /* minimum-size allocation */
#define SLABTEENYLOG2 12 /* little block */
#define SLABTINYLOG2  16 /* small-size block */
#define SLABLOG2      21 /* base size for heap allocations */
#define MAPMIDLOG2    23
#endif
#define MINSZ         (1UL << BLKMINLOG2)
#define HQMAX         SLABLOG2
#define NBKT          (8 * PTRSIZE)
#if (MTSAFE)
#define NARN          4
#else
#define NARN          1
#endif

/* lookup tree of tables */

#if (PTRBITS > 32)

#define NL1KEY     (1UL << NL1BIT)
#define NL2KEY     (1UL << NL2BIT)
#define NL3KEY     (1UL << NL3BIT)
#define L1NDX      (L2NDX + NL2BIT)
#define L2NDX      (L3NDX + NL3BIT)
#define L3NDX      SLABLOG2
#define NL1BIT     16

#if (PTRBITS > 48)

#define NL2BIT     16
#define NL3BIT     (PTRBITS - SLABLOG2 - NL1BIT - NL2BIT)
#else

#define NL2BIT     (PTRBITS - SLABLOG2 - NL1BIT)
#define NL3BIT     0

#endif /* PTRBITS > 48 */

#endif /* PTRBITS <= 32 */

/* macros */

#define isbufbkt(bid)     ((bid) <= MAPMIDLOG2)
#if (TUNEBUF)
#define nmagslablog2(bid) (_nslabtab[(bid)])
#else
#define nmagslablog2(bid) (ismapbkt(bid) ? nmaplog2(bid) : nslablog2(bid))
#define nslablog2(bid)    0
#define nmaplog2(bid)     0
#define nslablog2(bid)    0
#define nmaplog2(bid)     0
#endif

#if (TUNEBUF)
/* adjust how much is buffered based on current use */
#define nmagslablog2up(m, v, t)                                         \
    do {                                                                \
        if (t >= (v)) {                                                 \
            for (t = 0 ; t < NBKT ; t++) {                              \
                _nslabtab[(t)] = m(t);                                  \
            }                                                           \
        }                                                               \
    } while (0)
#if (NEWHACK)
#define nmagslablog2init(bid)                                           \
    ((ismapbkt(bid))                                                    \
     ? 0                                                                \
     : (((bid) <= SLABTEENYLOG2)                                        \
        ? 0                                                             \
        : (((bid) <= SLABTINYLOG2)                                      \
           ? 1                                                          \
           : 2)))
#define nmagslablog2m64(bid)                                            \
    ((ismapbkt(bid))                                                    \
     ? (((bid) <= MAPMIDLOG2)                                           \
        ? 1                                                             \
        : 0)                                                            \
     : (((bid) <= SLABTEENYLOG2)                                        \
        ? 0                                                             \
        : (((bid) <= SLABTINYLOG2)                                      \
           ? 1                                                          \
           : 2)))
#define nmagslablog2m128(bid)                                           \
    ((ismapbkt(bid))                                                    \
     ? (((bid) <= MAPMIDLOG2)                                           \
        ? 3                                                             \
        : 0)                                                            \
     : (((bid) <= SLABTEENYLOG2)                                        \
        ? 1                                                             \
        : (((bid) <= SLABTINYLOG2)                                      \
           ? 1                                                          \
           : 2)))
#define nmagslablog2m512(bid)                                           \
    ((ismapbkt(bid))                                                    \
     ? (((bid) <= MAPMIDLOG2)                                           \
        ? 1                                                             \
        : 0)                                                            \
     : (((bid) <= SLABTEENYLOG2)                                        \
        ? 0                                                             \
        : (((bid) <= SLABTINYLOG2)                                      \
           ? 1                                                          \
           : 2)))
#else
#define nmagslablog2init(bid)                                           \
    ((ismapbkt(bid))                                                    \
     ? (((bid) <= 23)                                                   \
        ? 2                                                             \
        : 1)                                                            \
     : (((bid) <= SLABTEENYLOG2)                                        \
        ? 1                                                             \
        : (((bid) <= SLABTINYLOG2)                                      \
           ? 1                                                          \
           : 2)))
#define nmagslablog2m64(bid)                                            \
    ((ismapbkt(bid))                                                    \
     ? 0                                                                \
     : (((bid) <= SLABTEENYLOG2)                                        \
        ? 0                                                             \
        : (((bid) <= SLABTINYLOG2)                                      \
           ? 1                                                          \
           : 2)))
#define nmagslablog2m128(bid)                                           \
    ((ismapbkt(bid))                                                    \
     ? (((bid) <= 23)                                                   \
        ? 1                                                             \
        : 0)                                                            \
     : (((bid) <= SLABTEENYLOG2)                                        \
        ? 1                                                             \
        : (((bid) <= SLABTINYLOG2)                                      \
           ? 1                                                          \
           : 2)))
#define nmagslablog2m256(bid)                                           \
    ((ismapbkt(bid))                                                    \
     ? (((bid) <= 24)                                                   \
        ? 1                                                             \
        : 0)                                                            \
     : (((bid) <= SLABTEENYLOG2)                                        \
        ? 1                                                             \
        : (((bid) <= SLABTINYLOG2)                                      \
           ? 1                                                          \
           : 2)))
#define nmagslablog2m512(bid)                                           \
    ((ismapbkt(bid))                                                    \
     ? (((bid) <= 24)                                                   \
        ? 1                                                             \
        : 0)                                                            \
     : (((bid) <= SLABTEENYLOG2)                                        \
        ? 0                                                             \
        : (((bid) <= SLABTINYLOG2)                                      \
           ? 1                                                          \
           : 2)))
#endif /* NEWHACK */
#endif /* TUNEBUF */
#define nblklog2(bid)                                                   \
    ((!(ismapbkt(bid))                                                  \
      ? (SLABLOG2 - (bid))                                              \
      : nmagslablog2(bid)))
#define nblk(bid)         (1UL << nblklog2(bid))
#define NBSLAB            (1UL << SLABLOG2)
#define nbmap(bid)        (1UL << (nmagslablog2(bid) + (bid)))
#define nbmag(bid)        (1UL << (nmagslablog2(bid) + SLABLOG2))

#if (PTRBITS <= 32)
#define NSLAB             (1UL << (PTRBITS - SLABLOG2))
#define slabid(ptr)       ((uintptr_t)(ptr) >> SLABLOG2)
#endif
#define nbhdr()           PAGESIZE
#define NBUFHDR           16

#define thrid()           ((_aid >= 0) ? _aid : (_aid = getaid()))
#define blksz(bid)        (1UL << (bid))
#define usrsz(bid)        (blksz(bid) - RZSZ)
#define ismapbkt(bid)     (bid > HQMAX)
#define magfull(mag)      (!(mag)->cur)
#define magempty(mag)     ((mag)->cur == (mag)->max)
#if (ALNSTK)
#define nbstk(bid)        max(nblk(bid) * sizeof(void *), PAGESIZE)
#define nbalnstk(bid)     nbstk(bid)
#else
#define nbstk(bid)        max((nblk(bid) << 1) * sizeof(void *), PAGESIZE)
#endif
#define mapstk(n)         mapanon(_mapfd, ((n) << 1) * sizeof(void *))
#define unmapstk(mag)     unmapanon((mag)->bptr, mag->max * sizeof(void *))
#define putblk(mag, ptr)                                                \
    ((gt2(mag->max, 1)                                                  \
      ? (((void **)(mag)->bptr)[--(mag)->cur] = (ptr))                  \
      : ((mag)->cur = 0, (mag)->adr = (ptr))))
#define getblk(mag)                                                     \
    ((gt2(mag->max, 1)                                                  \
      ? (((void **)(mag)->bptr)[(mag)->cur++])                          \
      : ((mag)->cur = 1, ((mag)->adr))))
#define NPFBIT BLKMINLOG2
#define BPMASK (~((1UL << NPFBIT) - 1))
#define BDIRTY 0x01UL
#define BALIGN 0x02UL
#define clrptr(ptr)       ((void *)((uintptr_t)(ptr) & BPMASK))
#define setflg(ptr, flg)  ((void *)((uintptr_t)(ptr) | (flg)))
#define chkflg(ptr, flg)  ((uintptr_t)(ptr) & (flg))
#define blkid(mag, ptr)                                                 \
    ((mag)->max + (((uintptr_t)(ptr) - (uintptr_t)(mag)->adr) >> (mag)->bid))
#define putptr(mag, ptr1, ptr2)                                         \
    ((gt2((mag)->max, 1))                                               \
     ? (((void **)(mag)->bptr)[blkid(mag, ptr1)] = (ptr2))              \
     : ((mag)->bptr = (ptr2)))
#define getptr(mag, ptr)                                                \
    ((gt2((mag)->max, 1))                                               \
     ? (((void **)(mag)->bptr)[blkid(mag, ptr)])                        \
     : ((mag)->bptr))

#if (STAT)
#include <stdio.h>
#endif

/* synchonisation */

#if (ZEROMTX)
#define mlk(mp)           mtxlk(mp, _aid + 1)
#define munlk(mp)         mtxunlk(mp, _aid + 1)
#define mtrylk(mp)        mtxtrylk(mp, _aid + 1)
#elif (SPINLK)
#define mlk(sp)           spinlk(sp)
#define munlk(sp)         spinunlk(sp)
#define mtrylk(sp)        spintrylk(sp)
#elif (MTSAFE)
#if (PTHREAD)
#define mlk(sp)           pthread_mutex_lock(sp)
#define munlk(sp)         pthread_mutex_unlock(sp)
#define mtrylk(sp)        pthread_mutex_trylock(sp)
#else
#define mlk(sp)           spinlk(sp)
#define munlk(sp)         spinunlk(sp)
#define mtrylk(sp)        spintrylk(sp)
#endif
#else
#define mlk(sp)
#define munlk(sp)
#define mtrylk(sp)
#endif
#define mlkspin(sp)       spinlk(sp)
#define munlkspin(sp)     spinunlk(sp)
#define mtrylkspin(sp)    spintry(sp)

/* configuration */

#define CONF_INIT 0x00000001
#define VIS_INIT  0x00000002
struct mconf {
    long        flags;
#if (MTSAFE)
    LK_T        initlk;
    LK_T        arnlk;
    LK_T        heaplk;
#endif
    long        scur;
    long        acur;
    long        narn;
};

#define istk(bid)                                                       \
    ((nblk(bid) << 1) * sizeof(void *) <= PAGESIZE)
struct mag {
    long        cur;
    long        max;
    long        aid;
    long        bid;
    void       *adr;
    void       *bptr;
    struct mag *prev;
    struct mag *next;
    struct mag *stk[EMPTY];
};

#define nbarn() (blksz(bktid(sizeof(struct arn))))
struct arn {
    struct mag  *btab[NBKT];
    struct mag  *ftab[NBKT];
    long         nref;
    long         hcur;
    long         nhdr;
    struct mag **htab;
    long         scur;
    LK_T         lktab[NBKT];
};

struct mtree {
#if (MTSAFE)
    LK_T         lk;
#endif
    struct mag **tab;
    long         nblk;
};

/* globals */

#if (INTSTAT)
static uint64_t        nalloc[NARN][NBKT];
static long            nhdrbytes[NARN];
static long            nstkbytes[NARN];
static long            nmapbytes[NARN];
static long            nheapbytes[NARN];
#endif
#if (STAT)
static unsigned long   _nheapreq[NBKT] ALIGNED(PAGESIZE);
static unsigned long   _nmapreq[NBKT];
#endif
#if (TUNEBUF)
static long            _nslabtab[NBKT];
#endif
#if (MTSAFE)
static LK_T            _flktab[NBKT];
#endif
static struct mag     *_ftab[NBKT];
#if (HACKS)
static long            _fcnt[NBKT];
#endif
static void          **_mdir;
static struct arn    **_atab;
static struct mconf    _conf;
#if (MTSAFE) && (PTHREAD)
static pthread_key_t   _akey;
static __thread long   _aid = -1;
#else
static long            _aid = 0;
#endif
#if (TUNEBUF)
static int64_t         _nbheap;
static int64_t         _nbmap;
#endif
static int             _mapfd = -1;

/* utility functions */

static __inline__ long
szceil2(size_t size)
{
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
#if (LONGSIZE == 8)
    size |= size >> 32;
#endif
    size++;

    return size;
}

static __inline__ long
bktid(size_t size)
{
    long tmp = szceil2(size);
    long bid;

#if (LONGSIZE == 4)
    tzero32(tmp, bid);
#elif (LONGSIZE == 8)
    tzero64(tmp, bid);
#endif

    return bid;
}

#if (MTSAFE)
static long
getaid(void)
{
    long        aid;

    mlk(&_conf.arnlk);
    aid = _conf.acur++;
    _conf.acur &= NARN - 1;
    pthread_setspecific(_akey, _atab[aid]);
    munlk(&_conf.arnlk);

    return aid;
}
#endif

static __inline__ void
zeroblk(void *ptr,
        size_t size)
{
    unsigned long *ulptr = ptr;
    unsigned long  zero = 0UL;
    long           small = (size < (LONGSIZE << 3));
    long           n = ((small)
                        ? (size >> LONGSIZELOG2)
                        : (size >> (LONGSIZELOG2 + 3)));
    long           nl = 8;

    if (small) {
        while (n--) {
            *ulptr++ = zero;
        }
    } else {
        while (n--) {
            ulptr[0] = zero;
            ulptr[1] = zero;
            ulptr[2] = zero;
            ulptr[3] = zero;
            ulptr[4] = zero;
            ulptr[5] = zero;
            ulptr[6] = zero;
            ulptr[7] = zero;
            ulptr += nl;
        }
    }

    return;
}

/* fork() management */

#if (MTSAFE)

static void
prefork(void)
{
    long        aid;
    long        bid;
    struct arn *arn;

    mlk(&_conf.initlk);
    mlk(&_conf.arnlk);
    mlk(&_conf.heaplk);
    aid = _conf.narn;
    while (aid--) {
        arn = _atab[aid];
        for (bid = 0 ; bid < NBKT ; bid++) {
            mlk(&arn->lktab[bid]);
        }
    }

    return;
}

static void
postfork(void)
{
    long        aid;
    long        bid;
    struct arn *arn;

    aid = _conf.narn;
    while (aid--) {
        arn = _atab[aid];
        for (bid = 0 ; bid < NBKT ; bid++) {
            munlk(&arn->lktab[bid]);
        }
    }
    munlk(&_conf.heaplk);
    munlk(&_conf.arnlk);
    munlk(&_conf.initlk);

    return;
}

static void
relarn(void *arg)
{
    struct arn *arn = arg;
#if (HACKS)
    long        n = 0;
#endif
    long        nref;
    long        bid;
    struct mag *mag;
    struct mag *head;

    nref = --arn->nref;
    if (!nref) {
        bid = NBKT;
        while (bid--) {
            mlk(&arn->lktab[bid]);
            head = arn->ftab[bid];
            if (head) {
#if (HACKS)
                n++;
#endif
                mag = head;
                while (mag->next) {
#if (HACKS)
                    n++;
#endif
                    mag = mag->next;
                }
                mlk(&_flktab[bid]);
                mag->next = _ftab[bid];
                _ftab[bid] = head;
#if (HACKS)
                _fcnt[bid] += n;
#endif
                munlk(&_flktab[bid]);
                arn->ftab[bid] = NULL;
            }
            munlk(&arn->lktab[bid]);
        }
    }

    return;
}

#endif /* MTSAFE */

/* statistics */

#if (STAT)
void
printstat(void)
{
    long l;

    for (l = 0 ; l < NBKT ; l++) {
        fprintf(stderr, "%ld\t%lu\t%lu\n", l, _nheapreq[l], _nmapreq[l]);
    }

    exit(0);
}
#elif (INTSTAT)
void
printintstat(void)
{
    long aid;
    long bkt;
    long nbhdr = 0;
    long nbstk = 0;
    long nbheap = 0;
    long nbmap = 0;

    for (aid = 0 ; aid < NARN ; aid++) {
        nbhdr += nhdrbytes[aid];
        nbstk += nstkbytes[aid];
        nbheap += nheapbytes[aid];
        nbmap += nmapbytes[aid];
        fprintf(stderr, "%lx: hdr: %ld\n", aid, nhdrbytes[aid] >> 10);
        fprintf(stderr, "%lx: stk: %ld\n", aid, nstkbytes[aid] >> 10);
        fprintf(stderr, "%lx: heap: %ld\n", aid, nheapbytes[aid] >> 10);
        fprintf(stderr, "%lx: map: %ld\n", aid, nmapbytes[aid] >> 10);
        for (bkt = 0 ; bkt < NBKT ; bkt++) {
            fprintf(stderr, "NALLOC[%lx][%lx]: %lld\n",
                    aid, bkt, nalloc[aid][bkt]);
        }
    }
    fprintf(stderr, "TOTAL: hdr: %ld, stk: %ld, heap: %ld, map: %ld\n",
            nbhdr, nbstk, nbheap, nbmap);
}
#endif

#if (X11VIS)
#include <X11/Xlibint.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xmd.h>
#include <X11/Xlocale.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

static LK_T  x11visinitlk;
#if 0
static LK_T  x11vislk;
#endif
long         x11visinit = 0;
Display     *x11visdisp = NULL;
Window       x11viswin = None;
Pixmap       x11vispmap = None;
GC           x11visinitgc = None;
GC           x11visfreedgc = None;
GC           x11visusedgc = None;
GC           x11visresgc = None;

#define x11vismarkfreed(ptr)                                            \
    do {                                                                \
        if (x11visinit) {                                               \
            int y = ((uintptr_t)(ptr) >> (BLKMINLOG2 + 10)) & 0x3ff;    \
            int x = ((uintptr_t)(ptr) >> BLKMINLOG2) & 0x3ff;           \
            XDrawPoint(x11visdisp, x11vispmap, x11visfreedgc, x, y);    \
        }                                                               \
} while (0)
#define x11vismarkres(ptr)                                              \
    do {                                                                \
        if (x11visinit) {                                               \
            int y = ((uintptr_t)(ptr) >> (BLKMINLOG2 + 10)) & 0x3ff;    \
            int x = ((uintptr_t)(ptr) >> BLKMINLOG2) & 0x3ff;           \
            XDrawPoint(x11visdisp, x11vispmap, x11visresgc, x, y);      \
        }                                                               \
    } while (0)
#define x11vismarkused(ptr)                                             \
    do {                                                                \
        if (x11visinit) {                                               \
            int y = ((uintptr_t)(ptr) >> (BLKMINLOG2 + 10)) & 0x3ff;    \
            int x = ((uintptr_t)(ptr) >> BLKMINLOG2) & 0x3ff;           \
            XDrawPoint(x11visdisp, x11vispmap, x11visusedgc, x, y);     \
        }                                                               \
    } while (0)

void
initx11vis(void)
{
    XColor col;
    XGCValues gcval;

//    mlk(&x11vislk);
    mlk(&x11visinitlk);
    if (x11visinit) {
        munlk(&x11visinitlk);

        return;
    }
    XInitThreads();
    x11visdisp = XOpenDisplay(NULL);
    if (x11visdisp) {
        x11viswin = XCreateSimpleWindow(x11visdisp,
                                        DefaultRootWindow(x11visdisp),
                                        0, 0,
                                        1024, 1024, 0,
                                        BlackPixel(x11visdisp,
                                                   DefaultScreen(x11visdisp)),
                                        WhitePixel(x11visdisp,
                                                   DefaultScreen(x11visdisp)));
        if (x11viswin) {
            XEvent ev;

            x11vispmap = XCreatePixmap(x11visdisp,
                                       x11viswin,
                                       1024, 1024,
                                       DefaultDepth(x11visdisp,
                                                    DefaultScreen(x11visdisp)));

            gcval.foreground = WhitePixel(x11visdisp,
                                          DefaultScreen(x11visdisp));
            x11visinitgc = XCreateGC(x11visdisp,
                                     x11viswin,
                                     GCForeground,
                                     &gcval);

            XFillRectangle(x11visdisp,
                           x11vispmap,
                           x11visinitgc,
                           0, 0,
                           1024, 1024);

            col.red = 0x0000;
            col.green = 0x0000;
            col.blue = 0xffff;
            if (!XAllocColor(x11visdisp,
                             DefaultColormap(x11visdisp,
                                             DefaultScreen(x11visdisp)),
                             &col)) {

                return;
            }
            gcval.foreground = col.pixel;
            x11visfreedgc = XCreateGC(x11visdisp,
                                      x11viswin,
                                      GCForeground,
                                      &gcval);

            col.red = 0xffff;
            col.green = 0x0000;
            col.blue = 0x0000;
            if (!XAllocColor(x11visdisp,
                             DefaultColormap(x11visdisp,
                                             DefaultScreen(x11visdisp)),
                             &col)) {

                return;
            }
            gcval.foreground = col.pixel;
            x11visusedgc = XCreateGC(x11visdisp,
                                     x11viswin,
                                     GCForeground,
                                     &gcval);

            col.red = 0x0000;
            col.green = 0xffff;
            col.blue = 0x0000;
            if (!XAllocColor(x11visdisp,
                             DefaultColormap(x11visdisp,
                                             DefaultScreen(x11visdisp)),
                             &col)) {

                return;
            }
            gcval.foreground = col.pixel;
            x11visresgc = XCreateGC(x11visdisp,
                                    x11viswin,
                                    GCForeground,
                                    &gcval);
            
            XSelectInput(x11visdisp, x11viswin, ExposureMask);
            XMapRaised(x11visdisp, x11viswin);
            do {
                XNextEvent(x11visdisp, &ev);
            } while (ev.type != Expose);
            XSelectInput(x11visdisp, x11viswin, NoEventMask);
        }
    }
    x11visinit = 1;
    munlk(&x11visinitlk);
//    munlk(&x11vislk);
}
#endif

static void
initmall(void)
{
    long        bid = NBKT;
    long        aid = NARN;
    long        ofs;
    uint8_t    *ptr;

    mlk(&_conf.initlk);
    if (_conf.flags & CONF_INIT) {
        munlk(&_conf.initlk);
        
        return;
    }
#if (STAT)
    atexit(printstat);
#elif (INTSTAT)
    atexit(printintstat);
#endif
#if (_MMAP_DEV_ZERO)
    _mapfd = open("/dev/zero", O_RDWR);
#endif
#if (MTSAFE)
    mlk(&_conf.arnlk);
    _atab = mapanon(_mapfd, NARN * sizeof(struct arn **));
    ptr = mapanon(_mapfd, NARN * nbarn());
    aid = NARN;
    while (aid--) {
        _atab[aid] = (struct arn *)ptr;
        ptr += nbarn();
    }
    aid = NARN;
    while (aid--) {
        for (bid = 0 ; bid < NBKT ; bid++) {
#if (ZEROMTX)
            mtxinit(&_atab[aid]->lktab[bid]);
#elif (PTHREAD) && !SPINLK
            pthread_mutex_init(&_atab[aid]->lktab[bid], NULL);
#endif
        }
        _atab[aid]->hcur = NBUFHDR;
    }
    _conf.narn = NARN;
    pthread_key_create(&_akey, relarn);
    munlk(&_conf.arnlk);
#endif
#if (PTHREAD)
    pthread_atfork(prefork, postfork, postfork);
#endif
#if (PTHREAD)
    while (bid--) {
#if (ZEROMTX)
        mtxinit(&_flktab[bid]);
#elif (PTHREAD) && !SPINLK
        pthread_mutex_init(&_flktab[bid], NULL);
#endif
    }
#endif
    mlk(&_conf.heaplk);
    ofs = NBSLAB - ((long)growheap(0) & (NBSLAB - 1));
    if (ofs != NBSLAB) {
        growheap(ofs);
    }
    munlk(&_conf.heaplk);
#if (PTRBITS <= 32)
    _mdir = mapanon(_mapfd, NSLAB * sizeof(void *));
#else
    _mdir = mapanon(_mapfd, NL1KEY * sizeof(void *));
#endif
#if (TUNEBUF)
    for (bid = 0 ; bid < NBKT ; bid++) {
        _nslabtab[bid] = nmagslablog2init(bid);
    }
#endif
    _conf.flags |= CONF_INIT;
    munlk(&_conf.initlk);
#if (X11VIS)
    initx11vis();
#endif

    return;
}

#if (MTSAFE)
#if (PTRBITS > 32)
#define l1ndx(ptr) getbits((uintptr_t)ptr, L1NDX, NL1BIT)
#define l2ndx(ptr) getbits((uintptr_t)ptr, L2NDX, NL2BIT)
#define l3ndx(ptr) getbits((uintptr_t)ptr, L3NDX, NL3BIT)
#if (PTRBITS > 48)
static struct mag *
findmag(void *ptr)
{
    uintptr_t  l1 = l1ndx(ptr);
    uintptr_t  l2 = l2ndx(ptr);
    uintptr_t  l3 = l3ndx(ptr);
    void       *ptr1;
    void       *ptr2;
    struct mag *mag = NULL;

    ptr1 = _mdir[l1];
    if (ptr1) {
        ptr2 = ((void **)ptr1)[l2];
        if (ptr2) {
            mag = ((struct mag **)ptr2)[l3];
        }
    }

    return mag;
}

static void
addblk(void *ptr,
       struct mag *mag)
{
    uintptr_t    l1 = l1ndx(ptr);
    uintptr_t    l2 = l2ndx(ptr);
    uintptr_t    l3 = l3ndx(ptr);
    void        *ptr1;
    void        *ptr2;
    void       **pptr;
    struct mag **item;

    ptr1 = _mdir[l1];
    if (!ptr1) {
        _mdir[l1] = ptr1 = mapanon(_mapfd, NL2KEY * sizeof(void *));
        if (ptr1 == MAP_FAILED) {
#ifdef ENOMEM
            errno = ENOMEM;
#endif

            exit(1);
        }
    }
    pptr = ptr1;
    ptr2 = pptr[l2];
    if (!ptr2) {
        pptr[l2] = ptr2 = mapanon(_mapfd, NL3KEY * sizeof(struct mag *));
        if (ptr2 == MAP_FAILED) {
#ifdef ENOMEM
            errno = ENOMEM;
#endif

            exit(1);
        }
    }
    item = &((struct mag **)ptr2)[l3];
    *item = mag;

    return;
}
#else
static struct mag *
findmag(void *ptr)
{
    uintptr_t   l1 = l1ndx(ptr);
    uintptr_t   l2 = l2ndx(ptr);
    void       *ptr1;
    struct mag *mag = NULL;

    ptr1 = _mdir[l1];
    if (ptr1) {
        mag = ((struct mag **)ptr1)[l2];
    }

    return mag;
}

static void
addblk(void *ptr,
       struct mag *mag)
{
    uintptr_t    l1 = l1ndx(ptr);
    uintptr_t    l2 = l2ndx(ptr);
    void        *ptr1;
    struct mag **item;

    ptr1 = _mdir[l1];
    if (!ptr1) {
        _mdir[l1] = ptr1 = mapanon(_mapfd, NL2KEY * sizeof(struct mag *));
        if (ptr1 == MAP_FAILED) {
#ifdef ENOMEM
            errno = ENOMEM;
#endif

            exit(1);
        }
    }
    item = &((struct mag **)ptr1)[l2];
    *item = mag;

    return;
}
#endif
#else
#define findmag(ptr)     (_mdir[slabid(ptr)])
#define addblk(ptr, mag) (_mdir[slabid(ptr)] = (mag))
#endif
#endif

static struct mag *
gethdr(long aid)
{
    struct arn  *arn;
    long         cur;
    struct mag **hbuf;
    struct mag  *mag = NULL;
    uint8_t     *ptr;
    
    arn = _atab[aid];
    hbuf = arn->htab;
    if (!arn->nhdr) {
        hbuf = mapanon(_mapfd, roundup2(NBUFHDR * sizeof(void *), PAGESIZE));
        if (hbuf != MAP_FAILED) {
#if (INTSTAT)
            nhdrbytes[aid] += roundup2(NBUFHDR * sizeof(void *), PAGESIZE);
#endif
            arn->htab = hbuf;
            arn->hcur = NBUFHDR;
            arn->nhdr = NBUFHDR;
        }
    }
    cur = arn->hcur;
    if (gte2(cur, NBUFHDR)) {
        mag = mapanon(_mapfd, roundup2(NBUFHDR * nbhdr(), PAGESIZE));
        if (mag == MAP_FAILED) {
#ifdef ENOMEM
            errno = ENOMEM;
#endif
            
            return NULL;
        } else {
#if (VALGRIND)
            if (RUNNING_ON_VALGRIND) {
                VALGRIND_MALLOCLIKE_BLOCK(mag, PAGESIZE, 0, 0);
            }
#endif
        }
        ptr = (uint8_t *)mag;
        while (cur) {
            mag = (struct mag *)ptr;
            *hbuf++ = mag;
            mag->bptr = mag->stk;
            cur--;
            ptr += nbhdr();
        }
    }
    hbuf = arn->htab;
#if (SYSDBG)
    _nhbuf++;
#endif
    mag = hbuf[cur++];
    arn->hcur = cur;

    return mag;
}

#if (TUNEBUF)
static void
tunebuf(long val)
{
    static long tunesz = 0;
    long        nb = _nbheap + _nbmap;

    return;

    if (!tunesz) {
        tunesz = val;
    }
    if (val == 64 && nb >= 64 * 1024) {
        nmagslablog2up(nmagslablog2m64, val, nb);
    } else if (val == 128 && nb >= 128 * 1024) {
        nmagslablog2up(nmagslablog2m128, val, nb);
#if (!NEWHACK)
    } else if (val == 256 && nb >= 256 * 1024) {
        nmagslablog2up(nmagslablog2m256, val, nb);
#endif
    } else if (val == 512 && nb >= 512 * 1024) {
        nmagslablog2up(nmagslablog2m512, val, nb);
    }

    return;
}
#endif

static void *
getslab(long aid,
        long bid)
{
    uint8_t     *ptr = NULL;
    long         nb = nbmag(bid);
#if (TUNEBUF)
    unsigned long tmp;
    static long tunesz = 0;
#endif

    if (!ismapbkt(bid)) {
        mlk(&_conf.heaplk);
        ptr = growheap(nb);
        munlk(&_conf.heaplk);
        if (ptr != SBRK_FAILED) {
#if (INTSTAT)
            nheapbytes[aid] += nb;
#endif
#if (TUNEBUF)
            _nbheap += nb;
#if (STAT)
            _nheapreq[bid]++;
#endif
#endif
        }
    } else {
        ptr = mapanon(_mapfd, nbmap(bid));
        if (ptr != MAP_FAILED) {
#if (INTSTAT)
            nmapbytes[aid] += nbmap(bid);
#endif
#if (STAT)
            _nmapreq[bid]++;
#endif
        }
    }
#if (TUNEBUF)
    if (ptr != MAP_FAILED && ptr != SBRK_FAILED) {
        tmp = _nbmap + _nbheap;
        if (!tunesz) {
            tunesz = 64;
        }
        if ((tmp >> 10) >= tunesz) {
            tunebuf(tunesz);
        }
    }
#endif

    return ptr;
}

static void
freemap(struct mag *mag)
{
    struct arn  *arn;
    long         cur;
    long         aid = mag->aid;
    long         bid = mag->bid;
    long         bsz = blksz(bid);
    long         max = mag->max;
    struct mag **hbuf;

    arn = _atab[aid];
    mlk(&arn->lktab[bid]);
    cur = arn->hcur;
    hbuf = arn->htab;
#if (HACKS)
    if (!cur || _fcnt[bid] < 4) {
#else
    if (!cur) {
#endif
        mag->prev = NULL;
        mlk(&_flktab[bid]);
        mag->next = _ftab[bid];
        _ftab[bid] = mag;
#if (HACKS)
        _fcnt[bid]++;
#endif
        munlk(&_flktab[bid]);
    } else {
        if (!unmapanon(clrptr(mag->adr), max * bsz)) {
#if (VALGRIND)
            if (RUNNING_ON_VALGRIND) {
                VALGRIND_FREELIKE_BLOCK(clrptr(mag->adr), 0);
            }
#endif
#if (INTSTAT)
            nmapbytes[aid] -= max * bsz;
#endif
#if (TUNEBUF)
            _nbmap -= max * bsz;
#endif
            if (gt2(max, 1)) {
                if (!istk(bid)) {
#if (INTSTAT)
                    nstkbytes[aid] -= (mag->max << 1) << sizeof(void *); 
#endif
                    unmapstk(mag);
                    mag->bptr = NULL;
#if (VALGRIND)
                    if (RUNNING_ON_VALGRIND) {
                        VALGRIND_FREELIKE_BLOCK(mag, 0);
                    }
#endif
                }
            }
            mag->adr = NULL;
            hbuf[--cur] = mag;
            arn->hcur = cur;
        }
    }
    munlk(&arn->lktab[bid]);

    return;
}

#define blkalnsz(sz, aln)                                               \
    (((aln) <= MINSZ)                                                   \
     ? max(sz, aln)                                                     \
     : (sz) + (aln))
static void *
getmem(size_t size,
       size_t align,
       long zero)
{
    struct arn  *arn;
    long         aid;
    long         sz = blkalnsz(max(size, MINSZ), align);
    long         bid = bktid(sz);
    uint8_t     *retptr = NULL;
    long         bsz = blksz(bid);
    uint8_t     *ptr = NULL;
    long         max = nblk(bid);
    struct mag  *mag = NULL;
    void       **stk;
    long         l;
    long         n;
    long         get = 0;
    
    if (!(_conf.flags & CONF_INIT)) {
        initmall();
    }

    aid = thrid();
    arn = _atab[aid];
    mlk(&arn->lktab[bid]);
    mag = arn->btab[bid];
    if (!mag) {
        mag = arn->ftab[bid];
    }
    if (!mag) {
        mlk(&_flktab[bid]);
        mag = _ftab[bid];
        if (mag) {
            mag->aid = aid;
            _ftab[bid] = mag->next;
            mag->next = NULL;
#if (HACKS)
            _fcnt[bid]--;
#endif
        }
        munlk(&_flktab[bid]);
        if (mag) {
            if (gt2(max, 1)) {
                mag->next = arn->btab[bid];
                if (mag->next) {
                    mag->next->prev = mag;
                }
                arn->btab[bid] = mag;
            }
        }
    } else if (mag->cur == mag->max - 1) {
        if (mag->next) {
            mag->next->prev = NULL;
        }
        arn->btab[bid] = mag->next;
        mag->next = NULL;
    }
    if (!mag) {
        get = 1;
        if (!ismapbkt(bid)) {
            ptr = getslab(aid, bid);
            if (ptr == (void *)-1L) {
                ptr = NULL;
            }
        } else {
            ptr = mapanon(_mapfd, nbmap(bid));
            if (ptr == MAP_FAILED) {
                ptr = NULL;
            }
#if (INTSTAT)
            else {
                nmapbytes[aid] += nbmap(bid);
            }
#endif
        }
        mag = gethdr(aid);
        if (mag) {
            mag->aid = aid;
            mag->cur = 0;
            mag->max = max;
            mag->bid = bid;
            mag->adr = ptr;
            if (ptr) {
                if (gt2(max, 1)) {
                    if (istk(bid)) {
                        stk = (void **)mag->stk;
                    } else {
                        stk = mapstk(max);
                    }
                    mag->bptr = stk;
                    if (stk != MAP_FAILED) {
#if (INTSTAT)
                        nstkbytes[aid] += (max << 1) << sizeof(void *); 
#endif
#if (VALGRIND)
                        if (RUNNING_ON_VALGRIND) {
                            VALGRIND_MALLOCLIKE_BLOCK(stk, max * sizeof(void *), 0, 0);
                        }
#endif
                        n = max << nmagslablog2(bid);
                        for (l = 0 ; l < n ; l++) {
                            stk[l] = ptr;
                            ptr += bsz;
                        }
                        mag->prev = NULL;
                        if (ismapbkt(bid)) {
                            mlk(&_flktab[bid]);
                            mag->next = _ftab[bid];
                            _ftab[bid] = mag;
#if (HACKS)
                            _fcnt[bid]++;
#endif
                        } else {
                            mag->next = arn->btab[bid];
                            if (mag->next) {
                                mag->next->prev = mag;
                            }
                            arn->btab[bid] = mag;
                        }
                    }
                }
            }
        }
    }
    if (mag) {
        ptr = getblk(mag);
        retptr = clrptr(ptr);
#if (VALGRIND)
        if (RUNNING_ON_VALGRIND) {
            if (retptr) {
                VALGRIND_MALLOCLIKE_BLOCK(retptr, bsz, 0, 0);
            }
        }
#endif
        if ((zero) && chkflg(ptr, BDIRTY)) {
            zeroblk(retptr, bsz);
        }
        ptr = retptr;
#if (RZSZ)
        markred(ptr);
        markred(ptr + RZSZ + size);
#endif
        if (retptr) {
#if (RZSZ)
            retptr = ptr + RZSZ;
#endif
            if (align) {
                if ((uintptr_t)(retptr) & (align - 1)) {
                    retptr = (uint8_t *)roundup2((uintptr_t)ptr, align);
                }
                ptr = setflg(retptr, BALIGN);
            }
            putptr(mag, retptr, ptr);
            addblk(retptr, mag);
        }
    }
    if ((get) && ismapbkt(bid)) {
        munlk(&_flktab[bid]);
    }
    munlk(&arn->lktab[bid]);
#if (X11VIS)
//    mlk(&x11vislk);
    if (x11visinit) {
//        ptr = clrptr(ptr);
        ptr = retptr;
        if (ptr) {
            long     l = blksz(bid) >> BLKMINLOG2;
            uint8_t *vptr = ptr;
            
            while (l--) {
                x11vismarkres(vptr);
                vptr += MINSZ;
            }
        }
        if (retptr) {
            long     l = sz >> BLKMINLOG2;
            uint8_t *vptr = retptr;
            
            while (l--) {
                x11vismarkused(ptr);
                vptr += MINSZ;
            }
        }
        XSetWindowBackgroundPixmap(x11visdisp,
                                   x11viswin,
                                   x11vispmap);
        XClearWindow(x11visdisp,
                     x11viswin);
        XFlush(x11visdisp);
    }
//    munlk(&x11vislk);
#endif
#ifdef ENOMEM
    if (!retptr) {
        errno = ENOMEM;
        fprintf(stderr, "%lx failed to allocate %ld bytes\n", aid, 1UL << bid);
        fflush(stderr);

        abort();
    }
#if (INTSTAT)
    else {
        nalloc[aid][bid]++;
    }
#endif
#endif

    return retptr;
}

static void
putmem(void *ptr)
{
#if (RZSZ)
    uint8_t    *u8p = ptr;
#endif
    struct arn *arn;
    void       *mptr;
    struct mag *mag = (ptr) ? findmag(ptr) : NULL;
    long        aid = -1;
    long        tid = thrid();
    long        bid = -1;
    long        max;
    long        glob = 0;
    long        freed = 0;

    if (mag) {
#if (VALGRIND)
        if (RUNNING_ON_VALGRIND) {
            VALGRIND_FREELIKE_BLOCK(ptr, 0);
        }
#endif
        aid = mag->aid;
        if (aid < 0) {
            glob++;
            mag->aid = aid = tid;
        }
        bid = mag->bid;
        max = mag->max;
        arn = _atab[aid];
        mlk(&arn->lktab[bid]);
        if (gt2(max, 1) && magempty(mag)) {
            mag->next = arn->btab[bid];
            if (mag->next) {
                mag->next->prev = mag;
            }
            arn->btab[bid] = mag;
        }
        mptr = getptr(mag, ptr);
#if (RZSZ)
        if (!chkflg(mptr, BALIGN)) {
            u8p = mptr - RZSZ;
            if (chkred(u8p) || chkred(u8p + blksz(bid) - RZSZ)) {
                fprintf(stderr, "red-zone violation\n");
            }
            ptr = clrptr(mptr);
        }
#endif
        if (mptr) {
            putptr(mag, ptr, NULL);
            mptr = setflg(mptr, BDIRTY);
            putblk(mag, mptr);
            if (magfull(mag)) {
                if (gt2(max, 1)) {
                    if (mag->prev) {
                        mag->prev->next = mag->next;
                    } else {
                        arn->btab[bid] = mag->next;
                    }
                    if (mag->next) {
                        mag->next->prev = mag->prev;
                    }
                }
                if (!isbufbkt(bid) && ismapbkt(bid)) {
                    freed = 1;
                } else {
#if (FIXES)
                    _mdir[slabid(ptr)] = NULL;
#endif
                    mag->prev = mag->next = NULL;
                    mlk(&_flktab[bid]);
                    mag->next = _ftab[bid];
                    _ftab[bid] = mag;
#if (HACKS)
                    _fcnt[bid]++;
#endif
                    munlk(&_flktab[bid]);
                }
            }
        }
        munlk(&arn->lktab[bid]);
        if (freed) {
            freemap(mag);
        }
#if (X11VIS)
//    mlk(&x11vislk);
        if (x11visinit) {
            ptr = mptr;
            if (ptr) {
                if (freed) {
                    long     l = nbmap(bid) >> BLKMINLOG2;
                    uint8_t *vptr = ptr;
                    
                    while (l--) {
                        x11vismarkfreed(vptr);
                        vptr += MINSZ;
                    }
                } else {
                    long     l = blksz(bid) >> BLKMINLOG2;
                    uint8_t *vptr = ptr;
                    
                    while (l--) {
                        x11vismarkfreed(vptr);
                        vptr += MINSZ;
                    }
                }
            }
            XSetWindowBackgroundPixmap(x11visdisp,
                                       x11viswin,
                                           x11vispmap);
            XClearWindow(x11visdisp,
                         x11viswin);
            XFlush(x11visdisp);
        }
//    munlk(&x11vislk);
#endif
    }

    return;
}

/* STD: ISO/POSIX */

void *
malloc(size_t size)
{
    void *ptr = getmem(size, 0, 0);

    return ptr;
}

void *
calloc(size_t n, size_t size)
{
    size_t  sz = n * (size + (RZSZ << 1));
    void   *ptr = getmem(sz, 0, 1);

    return ptr;
}

void *
_realloc(void *ptr,
         size_t size,
         long rel)
{
    void       *retptr = ptr;
    long        sz = blkalnsz(max(size + (RZSZ << 1), MINSZ), 0);
    struct mag *mag = (ptr) ? findmag(ptr) : NULL;
    long        bid = bktid(sz);
    uintptr_t   bsz = (mag) ? blksz(mag->bid) : 0;

    if (!ptr) {
        retptr = getmem(size, 0, 0);
    } else if ((mag) && mag->bid != bid) {
        retptr = getmem(size, 0, 0);
        if (retptr) {
            memcpy(retptr, ptr, min(sz, bsz));
            putmem(ptr);
            ptr = NULL;
        }
    }
    if ((rel) && (ptr)) {
        putmem(ptr);
    }

    return retptr;
}

void *
realloc(void *ptr,
        size_t size)
{
    void *retptr = _realloc(ptr, size, 0);

    return retptr;
}

void
free(void *ptr)
{
    if (ptr) {
        putmem(ptr);
    }

    return;
}

#if (_ISOC11_SOURCE)
void *
aligned_alloc(size_t align,
              size_t size)
{
    void *ptr = NULL;
    if (!powerof2(align) || (size % align)) {
        errno = EINVAL;
    } else {
        ptr = getmem(size, align, 0);
    }

    return ptr;
}

#endif

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
int
posix_memalign(void **ret,
               size_t align,
               size_t size)
{
    void *ptr = getmem(size, align, 0);
    int   retval = -1;

    if (!powerof2(align) || (size % sizeof(void *))) {
        errno = EINVAL;
    } else {
        ptr = getmem(size, align, 0);
        if (ptr) {
            retval ^= retval;
        }
    }

    *ret = ptr;

    return retval;
}
#endif

/* STD: UNIX */

#if ((_BSD_SOURCE)                                                      \
    || (_XOPEN_SOURCE >= 500 || ((_XOPEN_SOURCE) && (_XOPEN_SOURCE_EXTENDED))) \
    && !(_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600))
void *
valloc(size_t size)
{
    void *ptr = getmem(size, PAGESIZE, 0);

    return ptr;
}
#endif

void *
memalign(size_t align,
         size_t size)
{
    void *ptr = NULL;

    if (!powerof2(align)) {
        errno = EINVAL;
    } else {
        ptr = getmem(size, align, 0);
    }

    return ptr;
}

#if (_BSD_SOURCE)
void *
reallocf(void *ptr,
         size_t size)
{
    void *retptr = _realloc(ptr, size, 1);

    return retptr;
}
#endif

#if (_GNU_SOURCE)
void *
pvalloc(size_t size)
{
    size_t  sz = roundup2(size, PAGESIZE);
    void   *ptr = getmem(sz, PAGESIZE, 0);

    return ptr;
}
#endif

void
cfree(void *ptr)
{
    if (ptr) {
        free(ptr);
    }

    return;
}

size_t
malloc_usable_size(void *ptr)
{
    struct mag *mag = findmag(ptr);
    size_t      sz = usrsz(mag->bid);

    return sz;
}

size_t
malloc_good_size(size_t size)
{
    size_t rzsz = RZSZ;
    size_t sz = usrsz(bktid(size)) - (rzsz << 1);

    return sz;
}

size_t
malloc_size(void *ptr)
{
    struct mag *mag = findmag(ptr);
    size_t      sz = (mag) ? blksz(mag->bid) : 0;

    return sz;
}

