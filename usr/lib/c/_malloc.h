#ifndef ___MALLOC_H__
#define ___MALLOC_H__

#include <limits.h>
#include <stdint.h>
#include <malloc.h>
#include <zero/asm.h>
#include <zero/param.h>
#include <zero/trix.h>

/* internal stuff for zero malloc - not for the faint at heart to modify :) */

#define MALLOCCASQUEUE    0
#define MALLOCTKTLK       0
#define MALLOCNBTAIL      0
#define MALLOCNBDELAY     0
#define MALLOCNBSTK       0
#define MALLOCNBHEADLK    0
#define MALLOCFREEMAP     0
#define MALLOCSLABTAB     1

#define PTHREAD           1
#define ZEROMTX           1

#if !defined(MALLOCDEBUG)
#define MALLOCDEBUG       0
#endif
#if !defined(GNUTRACE)
#define GNUTRACE          1
#endif
#if !defined(MALLOCTRACE)
#define MALLOCTRACE       0
#endif

/* optional features and other hacks */
#define MALLOCVALGRIND    1
#define MALLOCHDRHACKS    0
#define MALLOCNEWHDR      1
#define MALLOCHDRPREFIX   1
#define MALLOCTLSARN      1
#define MALLOCSMALLADR    1
#define MALLOCSTAT        1
#define MALLOCPTRNDX      0
#define MALLOCCONSTSLABS  1
#define MALLOCDYNARN      0
#define MALLOCGETNPROCS   0
#define MALLOCNOPTRTAB    0
#define MALLOCNARN        16
#define MALLOCEXPERIMENT  0
#define MALLOCNBUFHDR     4

#define MALLOCDEBUGHOOKS  0
#define MALLOCDIAG        0 // run [heavy] internal diagnostics for debugging
#define DEBUGMTX          0

#define MALLOCSTEALMAG    0
#define MALLOCMULTITAB    1

#define MALLOCNOSBRK      0 // do NOT use sbrk()/heap, just mmap()
#define MALLOCFREETABS    1 // use free block bitmaps; bit 1 for allocated
#define MALLOCBUFMAG      1 // buffer mapped slabs to global pool

/* use zero malloc on a GNU system such as a Linux distribution */
#define GNUMALLOC         0

/* HAZARD: modifying anything below might break anything and everything BAD */

/* allocator parameters */

/* compiler-specified [GCC] alignment requirement for allocations */
#if defined(__BIGGEST_ALIGNMENT__)
#define MALLOCALIGNMENT   __BIGGEST_ALIGNMENT__
#endif
#if (!defined(MALLOCALIGNMENT))
#define MALLOCALIGNMENT   CLSIZE
#endif

/* <= MALLOCSLABLOG2 are tried to get from heap #if (!MALLOCNOSBRK) */
/* <= MALLOCBIGSLABLOG2 are kept in per-thread arenas which are lock-free */
#define MALLOCSLABLOG2    19
#define MALLOCBIGSLABLOG2 22
#define MALLOCBIGMAPLOG2  24
#define MALLOCHUGEMAPLOG2 26

#if !defined(MALLOCALIGNMENT) || (MALLOCALIGNMENT == 32)
#define MALLOCMINLOG2     5     // stuff such as SIMD types
#elif !defined(MALLOCALIGNMENT) || (MALLOCALIGNMENT == 16)
#define MALLOCMINLOG2     4     // stuff such as SIMD types
#elif !defined(MALLOCALIGNMENT) || (MALLOCALIGNMENT == 8)
#define MALLOCMINLOG2     3     // IEEE double
#else
#error fix MALLOCMINLOG2 in _malloc.h
#endif

/* invariant parameters */
#define MALLOCNBKT        PTRBITS
#define MALLOCMINSIZE     (1UL << MALLOCMINLOG2)

#if (MALLOCVALGRIND) && !defined(NVALGRIND)
#define VALGRINDMKPOOL(adr, z)                                          \
    do {                                                                \
        if (RUNNING_ON_VALGRIND) {                                      \
            VALGRIND_CREATE_MEMPOOL(adr, 0, z);                         \
        }                                                               \
    } while (0)
#define VALGRINDRMPOOL(adr)                                             \
    do {                                                                \
        if (RUNNING_ON_VALGRIND) {                                      \
            VALGRIND_DESTROY_MEMPOOL(adr);                              \
        }                                                               \
    } while (0)
#define VALGRINDMKSUPER(adr)                                            \
    do {                                                                \
        if (RUNNING_ON_VALGRIND) {                                      \
            VALGRIND_CREATE_MEMPOOL(adr, 0, z);                         \
        }                                                               \
    } while (0)
#define VALGRINDPOOLALLOC(pool, adr, sz)                                \
    do {                                                                \
        if (RUNNING_ON_VALGRIND) {                                      \
            VALGRIND_MEMPOOL_ALLOC(pool, adr, sz);                      \
        }                                                               \
    } while (0)
#define VALGRINDPOOLFREE(pool, adr)                                     \
    do {                                                                \
        if (RUNNING_ON_VALGRIND) {                                      \
            VALGRIND_MEMPOOL_FREE(pool, adr);                           \
        }                                                               \
    } while (0)
#define VALGRINDALLOC(adr, sz, z)                                       \
    do {                                                                \
        if (RUNNING_ON_VALGRIND) {                                      \
            VALGRIND_MALLOCLIKE_BLOCK((adr), (sz), 0, (z));             \
        }                                                               \
    } while (0)
#define VALGRINDFREE(adr)                                               \
    do {                                                                \
        if (RUNNING_ON_VALGRIND) {                                      \
            VALGRIND_FREELIKE_BLOCK((adr), 0);                          \
        }                                                               \
    } while (0)
#else /* !MALLOCVALGRIND */
#define VALGRINMKPOOL(adr, z)
#define VALGRINDMARKPOOL(adr, sz)
#define VALGRINDRMPOOL(adr)
#define VALGRINDMKSUPER(adr)
#define VALGRINDPOOLALLOC(pool, adr, sz)
#define VALGRINDPOOLFREE(pool, adr)
#define VALGRINDALLOC(adr, sz, z)
#define VALGRINDFREE(adr)
#endif

#if defined(MALLOCTKTLK) && (MALLOCTKTLK)
#include <zero/tktlk.h>
#define LOCK union zerotktlk
#define __mallocinitlk(mp)
#define __malloctrylk(mp)     tkttrylk(mp)
#define __malloclk(mp)        tktlk(mp)
#define __mallocunlk(mp)      tktunlk(mp)
#elif defined(ZEROMTX) && (ZEROMTX)
#define LOCK volatile long
#include <zero/mtx.h>
#include <zero/spin.h>
#if defined(DEBUGMTX) && (DEBUGMTX)
#define __mallocinitlk(mp)    mtxinit(mp)
#define __malloctrylk(mp)   mtxtrylk(mp)
#define __malloclk(mp)      (fprintf(stderr, "%p\tLK: %d\n", mp, __LINE__), \
                               mtxlk(mp))
#define __mallocunlk(mp)    (fprintf(stderr, "%p\tUNLK: %d\n", mp, __LINE__), \
                               mtxunlk(mp))
#define __mallocinitspin(mp)  spininit(mp)
#define __malloclkspin(mp)    (fprintf(stderr, "LK: %d\n", __LINE__),   \
                               spinlk(mp))
#define __mallocunlkspin(mp)  (fprintf(stderr, "UNLK: %d\n", __LINE__), \
                               spinunlk(mp))
#else
#define __mallocinitlk(mp)   mtxinit(mp)
#define __malloctrylk(mp)  mtxtrylk(mp)
#define __malloclk(mp)     mtxlk(mp)
#define __mallocunlk(mp)   mtxunlk(mp)
#define __mallocinitspin(mp)  spininit(mp)
#define __malloctrylkspin(mp) spintrylk(mp)
#define __malloclkspin(mp)    spinlk(mp)
#define __mallocunlkspin(mp)  spinunlk(mp)
#endif
#elif (PTHREAD)
#include <pthread.h>
#define LOCK pthread_mutex_t
#define __mallocinitlk(mp) pthread_mutex_init(mp, NULL)
#define __malloclk(mp)   pthread_mutex_lock(mp)
#define __mallocunlk(mp) pthread_mutex_unlock(mp)
#endif

#if (MALLOCPTRNDX)
#include <stdint.h>
#if (MALLOCSLABLOG2 - MALLOCMINLOG2 < 8)
#define PTRFREE              0xff
#define PTRNDX               uint8_t
#elif (MALLOCSLABLOG2 - MALLOCMINLOG2 < 16)
#define PTRFREE              0xffff
#define PTRNDX               uint16_t
#elif (MALLOCSLABLOG2 - MALLOCMINLOG2 < 32)
#define PTRFREE              0xffffffff
#define PTRNDX               uint32_t
#else
#define PTRFREE              UINT64_C(0xffffffffffffffff)
#define PTRNDX               uint64_t
#endif
#endif

#if defined(MALLOCDEBUG)
#if (MALLOCTRACE) && (GNUTRACE)
#endif
#if (MALLOCDEBUG)
#define _assert(expr)                                                   \
    do {                                                                \
        if (!(expr)) {                                                  \
            *((long *)NULL) = 0;                                        \
        }                                                               \
    } while (0)
#else
#define _assert(expr)
#endif
//#include <assert.h>
#endif

/* internal macros */
#define ptralign(ptr, pow2)                                             \
    ((void *)rounduppow2((uintptr_t)(ptr), pow2))

#if defined(__GLIBC__) || (defined(GNUMALLOC) && (GNUMALLOC))
#if !defined(__MALLOC_HOOK_VOLATILE)
#define MALLOC_HOOK_MAYBE_VOLATILE /**/
#elif !defined(MALLOC_HOOK_MAYBE_VOLATILE)
#define MALLOC_HOOK_MAYBE_VOLATILE __MALLOC_HOOK_VOLATILE
#endif
#endif

#if defined(GNUMALLOC) && (GNUMALLOC)
void * zero_malloc(size_t size);
void * zero_realloc(void *ptr, size_t size);
void * zero_memalign(size_t align,  size_t size);
void   zero_free(void *ptr);
#endif /* GNUMALLOC */

#if defined(GNUMALLOC) && (GNUMALLOC)
static void   gnu_malloc_init(void);
static void * gnu_malloc_hook(size_t size, const void *caller);
static void * gnu_realloc_hook(void *ptr, size_t size, const void *caller);
static void * gnu_memalign_hook(size_t align, size_t size);
static void   gnu_free_hook(void *ptr);
#endif /* defined(GNUMALLOC) */

#define MALLOCPAGETAB     0
#define MALLOCSLABTAB     1

#if (PTRBITS == 32)

#if (MALLOCSLABTAB)
#define SLABDIRNL1BIT     (PTRBITS - MALLOCSLABLOG2)
#endif
#define PAGEDIRNL1BIT     10
#define PAGEDIRNL2BIT     (PTRBITS - PAGEDIRNL1BIT - PAGESIZELOG2)

#elif (PTRBITS == 64) && (!MALLOCSMALLADR)

#if (MALLOCSLABTAB)
#define SLABDIRNL1BIT     20
#define SLABDIRNL2BIT     16
#define SLABDIRNL3BIT     MALLOCSLABLOG2
#endif
#define PAGEDIRNL1BIT     20
#define PAGEDIRNL2BIT     20
#define PAGEDIRNL3BIT     (PTRBITS - PAGEDIRNL1BIT - PAGEDIRNL2BIT      \
                           - PAGESIZELOG2)

#elif (PTRBITS == 64) && (MALLOCSMALLADR)

#if (MALLOCSLABTAB)
#define SLABDIRNL1BIT      20
#define SLABDIRNL2BIT      MALLOCSLABLOG2
#endif
#define PAGEDIRNL1BIT     20
#define PAGEDIRNL2BIT     PAGESIZELOG2

#if (ADRHIBITCOPY)

#if (MALLOCSLABTAB)
#define SLABDIRNL3BIT     (ADRBITS + 1 - SLABDIRNL1BIT - SLABDIRNL2BIT)
#endif
#define PAGEDIRNL3BIT     (ADRBITS + 1 - PAGEDIRNL1BIT - PAGEDIRNL2BIT)

#elif (ADRHIBITZERO)

#if (MALLOCSLABTAB)
#define SLABDIRNL3BIT     (ADRBITS - SLABDIRNL1BIT - SLABDIRNL2BIT)
#endif
#define PAGEDIRNL3BIT     (ADRBITS - PAGEDIRNL1BIT - PAGESIZENL2BIT)

#endif

#else /* PTRBITS != 32 && PTRBITS != 64 */

#error fix PTRBITS for _malloc.h

#endif

#define SLABDIRNL1KEY     (1L << SLABDIRNL1BIT)
#define SLABDIRNL2KEY     (1L << SLABDIRNL2BIT)
#if defined(SLABDIRNL3BIT) && (SLABDIRNL3BIT)
#define SLABDIRNL3KEY     (1L << SLABDIRNL3BIT)
#endif
#define PAGEDIRNL1KEY     (1L << PAGEDIRNL1BIT)
#define PAGEDIRNL2KEY     (1L << PAGEDIRNL2BIT)
#if defined(PAGEDIRNL3BIT) && (PAGEDIRNL3BIT)
#define PAGEDIRNL3KEY     (1L << PAGEDIRNL3BIT)
#endif

#if (MALLOCSLABTAB)
#define SLABDIRL1NDX      (SLABDIRL2NDX + SLABDIRNL2BIT)
#if defined(SLABDIRNL3BIT) && (SLABDIRNL3BIT)
#define SLABDIRL2NDX      (SLABDIRL3NDX + SLABDIRNL3BIT)
#define SLABDIRL3NDX      MALLOCSLABLOG2
#else
#define SLABDIRL2NDX      MALLOCSLABLOG2
#endif
#define PAGEDIRL1NDX      (PAGEDIRL2NDX + PAGEDIRNL2BIT)
#if defined(PAGEDIRNL3BIT) && (PAGEDIRNL3BIT)
#define PAGEDIRL2NDX      (PAGEDIRL3NDX + PAGEDIRNL3BIT)
#define PAGEDIRL3NDX      PAGESIZELOG2
#else
#define PAGEDIRL2NDX      PAGESIZELOG2
#endif

#define slabdirl1ndx(ptr) (((uintptr_t)(ptr) >> SLABDIRL1NDX)           \
                           & ((1UL << SLABDIRNL1BIT) - 1))
#define slabdirl2ndx(ptr) (((uintptr_t)(ptr) >> SLABDIRL2NDX)           \
                           & ((1UL << SLABDIRNL2BIT) - 1))
#define slabdirl3ndx(ptr) (((uintptr_t)(ptr) >> SLABDIRL3NDX)           \
                           & ((1UL << SLABDIRNL3BIT) - 1))

#define pagedirl1ndx(ptr) (((uintptr_t)(ptr) >> PAGEDIRL1NDX)           \
                           & ((1UL << PAGEDIRNL1BIT) - 1))
#define pagedirl2ndx(ptr) (((uintptr_t)(ptr) >> PAGEDIRL2NDX)           \
                           & ((1UL << PAGEDIRNL2BIT) - 1))
#define pagedirl3ndx(ptr) (((uintptr_t)(ptr) >> PAGEDIRL3NDX)           \
                           & ((1UL << PAGEDIRNL3BIT) - 1))

#endif /* MALLOCMULTITAB */

struct memtab {
    void          *ptr;
    volatile long  nref;
};

#if (MALLOCNBSTK)

/* request state */
#define BKT_REQ_NONE      0L
#define BKT_MARK_POS      0
#define BKT_MARK_BIT      (1L << 0)
#define BKT_REQ_FAIL      (1L << 1)
#define BKT_POP_REQ       (1L << 2)
#define BKT_PUSH_REQ      (1L << 3)
#if (MALLOCNBTAIL)
#define BKT_POP_TAIL_REQ  (1L << 4)
#define BKT_PUSH_TAIL_REQ (1L << 5)
#endif
/* request status */
#define BKT_REQ_DONE      (1L << 31)
struct bktreq {
    long           state;       // request type and state
    long           bktid;       // bucket ID for request
    struct mag    *mag;         // mag to push or a popped one
};

#define BKTNREQ (LONGSIZE * CHAR_BIT)
struct bktreqs {
    volatile long   slotbits;    // 0 for unused slots, 1 for used
    struct bktreq **stk;
    struct bktreq  *tab;
    uint8_t         _pad[CLSIZE - 2 * sizeof(long) - sizeof(void *)];
};

#endif /* MALLOCNBSTK */

#if (MALLOCNBSTK)
#define MAG_FREE_POS    0
#define MAG_FREE_BIT    (1 << 0)
#endif
#define maglkbit(mag)   1
#define magunlkbit(mag) 0
#define MAGMAP          0x01
#define ADRMASK         (MAGMAP)
#define PTRFLGMASK      0
#define PTRADRMASK      (~PTRFLGMASK)
#define MALLOCHDRSIZE   PAGESIZE
/* magazines for larger/fewer allocations embed the tables in the structure */
/* magazine header structure */
struct mag {
    volatile long  lk;
#if (MALLOCNBSTK)
    volatile long  flg;
#endif
    struct arn    *arn;
    void          *base;
    void          *adr;
    uint8_t       *ptr;
    size_t         size;
    long           cur;
    long           lim;
#if (!MALLOCTLSARN)
    long           arnid;
#endif
#if (MALLOCFREEMAP)
    volatile long  freelk;
    uint8_t       *freemap;
#endif
    long           bktid;
    struct mag    *prev;
    struct mag    *next;
#if (MALLOCPTRNDX)
    PTRNDX        *stk;
    PTRNDX        *idtab;
#else
    void          *stk;
    void          *ptrtab;
#endif
};

struct magtab {
    LOCK             lk;
    struct mag      *ptr;
#if (MALLOCNBTAIL) || (MALLOCCASQUEUE)
    struct mag      *tail;
#endif
#if (MALLOCBUFMAG)
    unsigned long    n;
#endif
#if (MALLOCBUFMAG)
    uint8_t          _pad[rounduppow2(sizeof(LOCK)
                                      + 2 * sizeof(long),
                                      CLSIZE - sizeof(LOCK)
                                      - 2 * sizeof(long))];
#else
    uint8_t          _pad[rounduppow2(sizeof(LOCK) + sizeof(long),
                                      CLSIZE - sizeof(LOCK)
                                      - sizeof(long))];
#endif
};

#define MALLOCARNSIZE rounduppow2(sizeof(struct arn), PAGESIZE)
/* arena structure */
struct arn {
    struct magtab magbkt[MALLOCNBKT];
#if (MALLOCNBSTK)
    struct magtab hdrbuf[MALLOCNBKT];
#endif
#if (!MALLOCTLSARN)
    LOCK          nreflk;
    long          nref;
#endif
};

/* magazine list header structure */

struct magbkt {
    volatile long   nref;
#if (MALLOCNBSTK)
    volatile long   cur;
    volatile long   n;
    struct mag    **tab;
#else
    struct mag *tab;
#endif
};

static void * maginitslab(struct mag *mag, long bktid);
static void * maginittab(struct mag *mag, long bktid);
static void * maginit(struct mag *mag, long bktid);

#if (!PTRFLGMASK)
#define clrptr(ptr)                                                     \
    (ptr)
#else
#define clrptr(ptr)                                                     \
    ((void *)((uintptr_t)ptr & ~PTRADRMASK))
#endif
#define clradr(adr)                                                     \
    ((uintptr_t)(adr) & ~ADRMASK)
#define ptrdiff(ptr1, ptr2)                                             \
    ((uintptr_t)(ptr2) - (uintptr_t)(ptr1))

#if (MALLOCPTRNDX)
#define magptrid(mag, ptr)                                              \
    ((PTRNDX)((uintptr_t)clrptr(ptr) - (uintptr_t)(mag)->base) >> (mag)->bktid)
#define magputid(mag, ptr, id)                                          \
    (((mag)->idtab)[magptrid(mag, ptr)] = (id))
#define maggetid(mag, ptr)                                              \
    (((mag)->idtab)[magptrid(mag, ptr)])
#define magptr(mag, ndx)                                                \
    ((void *)((uint8_t *)((mag)->base + (ndx * (1UL << (mag)->bktid)))))
#define maggetptr(mag, ptr)                                             \
    (((void **)(mag)->idtab)[magptrid(mag, ptr)])
#endif
#define magptrid(mag, ptr)                                              \
    (((uintptr_t)clrptr(ptr) - (uintptr_t)(mag)->base) >> (mag)->bktid)
#define magputptr(mag, ptr, orig)                                       \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr)] = (orig))
#define maggetptr(mag, ptr)                                             \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr)])

/*
 * magazines for bucket bktid have 1 << magnblklog2(bktid) blocks of
 * 1 << bktid bytes
 */
#define magnmaplog2(bktid)                                              \
    (((bktid) <= MALLOCBIGMAPLOG2)                                      \
     ? 2                                                                \
     : (((bktid <= MALLOCHUGEMAPLOG2)                                   \
         ? 1                                                            \
         : 0)))
    
#define magnblklog2(bktid)                                              \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? (MALLOCSLABLOG2 - (bktid))                                       \
     : magnmaplog2(bktid))
#define magnblk(bktid)                                                  \
    (1UL << magnblklog2(bktid))

#if (MALLOCBUFMAG)
#define magnglobbuflog2(bktid)                                          \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? 3                                                                \
     : (((bktid) <= MALLOCBIGMAPLOG2)                                   \
        ? 2                                                             \
        : (((bktid <= MALLOCHUGEMAPLOG2)                                \
            ? 1                                                         \
            : 0))))
#define magnglobbuf(bktid)                                              \
    (1UL << magnglobbuflog2(bktid))

#define magnarnbuflog2(bktid)                                           \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? 2                                                                \
     : (((bktid) <= MALLOCBIGMAPLOG2)                                   \
        ? 2                                                             \
        : 0))
#define magnarnbuf(bktid)                                               \
    (1UL << magnarnbuflog2(bktid))
#else
#define magnglobbuf(bktid) 1
#define magnarnbuf(bktid)  1
#endif

#define maghdrsz(bktid)                                                 \
    (rounduppow2(sizeof(struct mag), CLSIZE))
#if (MALLOCFREEMAP)
#if (MALLOCPTRNDX)
#define magtabsz(bktid)                                                 \
    (maghdrsz()                                                         \
     + ((!magnblklog2(bktid)                                            \
         ? 0                                                            \
         : (magnblk(bktid) * sizeof(void *)                             \
            + magnblk(bktid) * sizeof(PTRNDX)                           \
            + ((magnblk(bktid) + CHAR_BIT) >> 3)))))
#else
#define magtabsz(bktid)                                                 \
    (maghdrsz()                                                         \
     + ((!magnblklog2(bktid)                                            \
         ? 0                                                            \
         : ((magnblk(bktid) << 1) * sizeof(void *)                      \
            + ((magnblk(bktid) + CHAR_BIT) >> 3)))))
#endif
#else
#if (MALLOCPTRNDX)
#define magtabsz(bktid)                                                 \
    (maghdrsz()                                                         \
     + (!magnblklog2(bktid)                                             \
        ? 0                                                             \
        : (magnblk(bktid) * sizeof(void *)                              \
           + magnblk(bktid) * sizeof(PTRNDX))))
#else
#define magtabsz(bktid)                                                 \
    (maghdrsz()                                                         \
     + (!magnblklog2(bktid)                                             \
        ? 0                                                             \
        : (magnblk(bktid) << 1) * sizeof(void *)))
#endif
#endif
#define magembedtab(bktid) (magtabsz(bktid) <= MALLOCHDRSIZE)

#if (MALLOCCASQUEUE)

#define arnrmhead(bkt, head)                                            \
    do {                                                                \
        if ((head)->next) {                                             \
            (head)->next->prev = NULL;                                  \
        } else {                                                        \
            (bkt)->tail = NULL;                                         \
        }                                                               \
        (bkt)->ptr = (head)->next;                                      \
        (head)->arn = NULL;                                             \

#define arnrmtail(bkt, tail)                                            \
    do {                                                                \
        if ((tail)->prev) {                                             \
            (bkt)->tail = (tail)->prev;                                 \
        } else {                                                        \
            (bkt)->ptr = NULL;                                          \
            (bkt)->tail = NULL;                                         \
        }                                                               \
    } while (0)

#define arnrmmag(mag)                                                   \
    do {                                                                \
        if (((mag)->prev) && ((mag)->next)) {                           \
            (mag)->next->prev = (mag)->prev;                            \
            (mag)->prev->next = (mag)->next;                            \
        } else if ((mag)->prev) {                                       \
            (mag)->prev->next = NULL;                                   \
        } else if ((mag)->next) {                                       \
            (mag)->next->prev = NULL;                                   \
            (mag)->arn->magbkt[bktid].ptr = (mag)->next;                \
        } else if ((mag)->arn) {                                        \
            (mag)->arn->magbkt[bktid].ptr = NULL;                       \
        }                                                               \
        (mag)->arn = NULL;                                              \
    } while (0)

#elif (MALLOCNBSTK)

#define arnrmhead(bkt, head)                                            \
    do {                                                                \
        if ((head)->next) {                                             \
            (head)->next->prev = NULL;                                  \
        } else {                                                        \
            (bkt)->tail = NULL;                                         \
        }                                                               \
        (bkt)->ptr = (head)->next;                                      \
        (head)->arn = NULL;                                             \
    } while (0)

#endif

#define magrmhead(bkt, head)                                            \
    do {                                                                \
        if ((head)->next) {                                             \
            (head)->next->prev = NULL;                                  \
        }                                                               \
        (bkt)->ptr = (head)->next;                                      \
    } while (0)
#define magrm(mag, bkt, lock)                                           \
    do {                                                                \
        if ((lock) && !mag->arn) {                                      \
            __malloclk(&(bkt)->lk);                                     \
        }                                                               \
        if (((mag)->prev) && ((mag)->next)) {                           \
            (mag)->next->prev = (mag)->prev;                            \
            (mag)->prev->next = (mag)->next;                            \
        } else if ((mag)->prev) {                                       \
            (mag)->prev->next = NULL;                                   \
        } else if ((mag)->next) {                                       \
            (mag)->next->prev = NULL;                                   \
            if ((mag)->arn) {                                           \
                (mag)->arn->magbkt[bktid].ptr = (mag)->next;            \
            } else {                                                    \
                (bkt)->ptr = (mag)->next;                               \
            }                                                           \
        } else if ((mag)->arn) {                                        \
            (mag)->arn->magbkt[bktid].ptr = NULL;                       \
        } else {                                                        \
            (bkt)->ptr = NULL;                                          \
        }                                                               \
        if ((lock) && !mag->arn) {                                      \
            __mallocunlk(&(bkt)->lk);                                   \
        }                                                               \
        (mag)->arn = NULL;                                              \
    } while (0)

#if (MALLOCBUFMAG)
#define bktaddmag(bkt)       ((bkt)->n++)
#define bktrmmag(bkt)        ((bkt)->n--)
#define bktaddmany(bkt, num) ((bkt)->n += (num))
#else
#define bktaddmag(bkt)
#define bktrmmag(bkt)
#define bktaddmany(bkt, num)
#endif

#if (MALLOCCASQUEUE)

#define arnpushmag(mag, tab)                                            \
    do {                                                                \
        struct mag *_next = (tab)->ptr;                                 \
                                                                        \
        (mag)->next =  _next;                                           \
        if (_next) {                                                    \
            _next->prev = (mag);                                        \
        }                                                               \
        (tab)->ptr = (mag);                                             \
    } while (0)

#define arnpopmag(bkt, mag)                                             \
    do {                                                                \
        struct mag *_mag = NULL;                                        \
                                                                        \
        _mag = (bkt)->ptr;                                              \
        if (_mag) {                                                     \
            arnrmhead((bkt), _mag);                                     \
            bktrmmag(bkt);                                              \
        }                                                               \
        (mag) = _mag;                                                   \
    } while (0)

#define magpush(mag, tab) casqueuepush(mag, &(tab)-ptr)

#define magpushmany(first, last, tab)                                   \
    casqueuepushmany(first, last, &tab->ptr)

#define magpushtail(mag, bkt)                                           \

#define magpushtailmany(first, last, bkt, n)                            \

#elif (MALLOCNBSTK)

#define arnpushmag(mag, bkt)                                            \
    do {                                                                \
        (mag)->next = (bkt)->ptr;                                       \
        if ((mag)->next) {                                              \
            (mag)->next->prev = (mag);                                  \
        } else {                                                        \
            (bkt)->tail = (mag);                                        \
        }                                                               \
        bktaddmag(bkt);                                                 \
        (bkt)->ptr = (mag);                                             \
    } while (0)

#define magpush(mag, bkt)                                               \
    do {                                                                \
        (mag)->next = (bkt)->ptr;                                       \
        if ((mag)->next) {                                              \
            (mag)->next->prev = (mag);                                  \
        } else {                                                        \
            (bkt)-tail = (mag);                                         \
        }                                                               \
        bktaddmag(bkt);                                                 \
        (bkt)->ptr = (mag);                                             \
    } while (0)
#define magpushmany(first, last, bkt, n)                                \
    do {                                                                \
        (first)->prev = NULL;                                           \
        (last)->next = (bkt)->ptr;                                      \
        if ((last)->next) {                                             \
            (last)->next->prev = (last);                                \
        } else {                                                        \
            (bkt)->tail = (last);                                       \
        }                                                               \
        bktaddmany(bkt, n);                                             \
        (bkt)->ptr = (first);                                           \
    } while (0)

#define arnpopmag(bkt, mag)                                             \
    do {                                                                \
        struct mag *_mag = NULL;                                        \
                                                                        \
        _mag = (bkt)->ptr;                                              \
        if (_mag) {                                                     \
            arnrmhead((bkt), _mag);                                     \
            bktrmmag(bkt);                                              \
        }                                                               \
        (mag) = _mag;                                                   \
    } while (0)

#if (MALLOCNBTAIL)

#define arnpushtail(mag, bkt)                                           \
    do {                                                                \
        (mag)->prev = (bkt)->tail;                                      \
        if ((mag)->prev) {                                              \
            (mag)->prev->next = (mag);                                  \
        } else {                                                        \
            (bkt)->ptr = (mag);                                         \
        }                                                               \
        bktaddmag(bkt);                                                 \
        (bkt)->tail = (mag);                                            \
    } while (0)

#define magpushtail(mag, bkt)                                           \
    do {                                                                \
        (mag)->prev = (bkt)->tail;                                      \
        if ((mag)->prev) {                                              \
            (mag)->prev->next = (mag);                                  \
        } else {                                                        \
            (bkt)->ptr = (mag);                                         \
        }                                                               \
        bktaddmag(bkt);                                                 \
        (bkt)->tail = (mag);                                            \
    } while (0)

#define magpushtailmany(first, last, bkt, n)                            \
        do {                                                            \
            (first)->prev = (bkt)->tail;                                \
            (last)->next = NULL;                                        \
            if ((first)->prev) {                                        \
                (first)->prev->next = (first);                          \
            } else {                                                    \
                (bkt)->ptr = (first);                                   \
            }                                                           \
            bktaddmany(bkt, n);                                         \
            (bkt)->tail = (last);                                       \
        } while (0)

#define arnpopmagtail(bkt, mag)                                         \
    do {                                                                \
        struct mag *_mag = NULL;                                        \
                                                                        \
        _mag = (bkt)->tail;                                             \
        if (_mag) {                                                     \
            arnrmtail((bkt), _mag);                                     \
            bktrmmag(bkt);                                              \
        }                                                               \
        (mag) = _mag;                                                   \
    } while (0)

#endif

#else

#define magpop(bkt, mag, lock)                                          \
    do {                                                                \
        struct mag *_mag = NULL;                                        \
                                                                        \
        if (lock) {                                                     \
            __malloclk(&(bkt)->lk);                                     \
        }                                                               \
        _mag = (bkt)->ptr;                                              \
        if (_mag) {                                                     \
            magrmhead((bkt), _mag);                                     \
            bktrmmag(bkt);                                              \
        }                                                               \
        if (lock) {                                                     \
            __mallocunlk(&(bkt)->lk);                                   \
        }                                                               \
        (mag) = _mag;                                                   \
    } while (0)

#define magpush(mag, bkt, lock)                                         \
    do {                                                                \
        if (lock) {                                                     \
            __malloclk(&(bkt)->lk);                                     \
        }                                                               \
        (mag)->next = (bkt)->ptr;                                       \
        if ((mag)->next) {                                              \
            (mag)->next->prev = (mag);                                  \
        }                                                               \
        bktaddmag(bkt);                                                 \
        (bkt)->ptr = (mag);                                             \
        if (lock) {                                                     \
            __mallocunlk(&(bkt)->lk);                                   \
        }                                                               \
    } while (0)
#define magpushmany(first, last, bkt, lock, n)                          \
    do {                                                                \
        (first)->prev = NULL;                                           \
        if (lock) {                                                     \
            __malloclk(&(bkt)->lk);                                     \
        }                                                               \
        (last)->next = (bkt)->ptr;                                      \
        if ((last)->next) {                                             \
            (last)->next->prev = (last);                                \
        }                                                               \
        bktaddmany(bkt, n);                                             \
        (bkt)->ptr = (first);                                           \
        if (lock) {                                                     \
            __mallocunlk(&(bkt)->lk);                                   \
        }                                                               \
    } while (0)

#endif

#define MALLOPT_PERTURB_BIT 0x00000001
struct mallopt {
    int action;
    int flg;
    int perturb;
    int mmapmax;
    int mmaplog2;
};

/* malloc global structure */
#define MALLOCINIT   0x00000001L
#define MALLOCNOHEAP 0x00000002L
struct malloc {
    struct magtab     magbkt[MALLOCNBKT];
    struct magtab     freetab[MALLOCNBKT];
#if (!MALLOCNBSTK)
    struct magtab     hdrbuf[MALLOCNBKT];
#endif
#if (MALLOCNBKT == 64)
    uint64_t          magemptybits;
#elif (MALLOCNBKT == 32)
    uint32_t          magemptybits;
#endif
#if (!MALLOCTLSARN)
    struct arn      **arntab;           // arena structures
#endif
    LOCK             *pagedirlktab;
#if (MALLOCFREETABS)
    struct memtab    *pagedir;          // allocation header lookup structure
#else
    void            **pagedir;
#endif
    LOCK             initlk;            // initialization lock
    LOCK             heaplk;            // lock for sbrk()
#if (!MALLOCTLSARN)
    long              curarn;
    long              narn;             // number of arenas in action
#endif
    volatile long     flg;              // allocator flags
    int               zerofd;           // file descriptor for mmap()
    struct mallopt    mallopt;          // mallopt() interface
    struct mallinfo   mallinfo;         // mallinfo() interface
};

#if (MALLOCNBSTK)

extern struct malloc  g_malloc;
extern struct bktreqs bktreqs[MALLOCNBKT];

static __inline__ struct mag *
magtrypop(long bktid)
{
    struct magtab *bkt = &g_malloc.magbkt[bktid];
    struct mag    *mag = NULL;
    struct mag    *head;
    struct mag    *mark;
    struct mag    *next;
    
    if (!m_cmpsetbit((volatile long *)&bkt->ptr, BKT_MARK_POS)) {
        mark = bkt->ptr;
        head = (struct mag *)((uintptr_t)mark & ~(BKT_MARK_BIT));
        if (head) {
            if (!m_cmpsetbit((volatile long *)&head->next, BKT_MARK_POS)) {
                mark = head->next;
                next = (struct mag *)((uintptr_t)mark & ~BKT_MARK_BIT);
                if (next) {
                    if (!m_cmpsetbit((volatile long *)&next->next, BKT_MARK_POS)) {
                        next->prev = NULL;
                        mag = head;
                        bkt->ptr = next;
                        m_cmpclrbit((volatile long *)&next->next, BKT_MARK_POS);
                    }
                } else {
                    m_cmpclrbit((volatile long *)&head->next, BKT_MARK_POS);
                }
                if (mag) {
                    m_cmpclrbit((volatile long *)&mag->next, BKT_MARK_POS);
                }
            }
        }
        m_cmpclrbit((volatile long *)&bkt->ptr, BKT_MARK_POS);
    }
    
    return mag;
}
static __inline__ struct mag *
magtrypush(struct mag *mag, long bktid)
{
    struct magtab *bkt = &g_malloc.magbkt[bktid];
    struct mag    *head;
    struct mag    *new;
    struct mag    *mark;
    long           done = 0;
    
    mag->prev = NULL;
    if (!m_cmpsetbit((volatile long *)&bkt->ptr, BKT_MARK_POS)) {
        if (!mag->arn || !m_cmpsetbit((volatile long *)&mag->next, BKT_MARK_POS)) {
            new = (struct mag *)((uintptr_t)mag | BKT_MARK_BIT);
            mark = bkt->ptr;
            head = (struct mag *)((uintptr_t)mark & ~BKT_MARK_BIT);
            if (head) {
                if (!m_cmpsetbit((volatile long *)&head->next, BKT_MARK_POS)) {
                    mag->next = mark;
                    head->prev = mag;
                    bkt->ptr = new;
                    done = 1;
                    m_cmpclrbit((volatile long *)&head->next, BKT_MARK_POS);
                }
            } else {
                mag->next = mark;
#if (MALLOCNBTAIL)
                bkt->tail = new;
#endif
                bkt->ptr = new;
            }
            if (!mag->arn) {
                m_cmpclrbit((volatile long *)&mag->next, BKT_MARK_POS);
            }
            mag = NULL;
            mag->arn = NULL;
        }
        m_cmpclrbit((volatile long *)&bkt->ptr, BKT_MARK_POS);
    }

    return mag;
}

#if (MALLOCNBTAIL)

static __inline__ struct mag *
magtrypoptail(long bktid)
{
    struct magtab *bkt = &g_malloc.magbkt[bktid];
    struct mag    *mag = NULL;
    struct mag    *tail;
    struct mag    *prev;
    
    if (!m_cmpsetbit((volatile long *)&bkt->ptr, BKT_MARK_POS)) {
        tail = bkt->tail;
        if (tail) {
            if (!m_cmpsetbit((volatile long *)&tail->next, BKT_MARK_POS)) {
                prev = tail->prev;
                if (prev) {
                    if (!m_cmpsetbit((volatile long *)&prev->next,
                                     BKT_MARK_POS)) {
                        prev->next = (struct mag *)BKT_MARK_BIT;
                        mag = tail;
                        bkt->tail = prev;
                    }
                } else {
                    mag = tail;
                    bkt->tail = NULL;
                    bkt->ptr = (struct mag *)BKT_MARK_BIT;
                }
                m_cmpclrbit((volatile long *)&tail->next, BKT_MARK_POS);
            }
        }
        m_cmpclrbit((volatile long *)&bkt->ptr, BKT_MARK_POS);
    }
    
    return mag;
}

static __inline__ struct mag *
magtrypushtail(struct mag *mag, long bktid)
{
    struct magtab *bkt = &g_malloc.magbkt[bktid];
    struct mag    *tail;
    struct mag    *new;
    struct mag    *prev;
    
    mag->prev = NULL;
    if (!m_cmpsetbit((volatile long *)&bkt->ptr, BKT_MARK_POS)) {
        new = (struct mag *)((uintptr_t)mag | BKT_MARK_BIT);
        tail = bkt->tail;
        if (tail) {
            if (!m_cmpsetbit((volatile long *)&tail->next, BKT_MARK_POS)) {
                prev = tail;
                mag->prev = prev;
                prev->next = mag;
                bkt->tail = new;
                m_cmpclrbit((volatile long *)&tail->next, BKT_MARK_POS);
            }
        } else {
            bkt->tail = mag;
            bkt->ptr = new;
            mag = NULL;
            mag->arn = NULL;
        }
        m_cmpclrbit((volatile long *)&bkt->ptr, BKT_MARK_POS);
    }

    return mag;
}

static __inline__ void
bkthelpreq(long bktid)
{
    struct mag      *mag;
    struct bktreq   *req;
    struct bktreq  **reqptr;
    struct bktreqs  *dest;
    long             slot;
    
    dest = &bktreqs[bktid];
    for (slot = 0 ; slot < BKTNREQ ; slot++) {
        if (!m_cmpsetbit(&dest->slotbits, slot)) {
            req = &dest->tab[slot];
            reqptr = &dest->stk[slot];
            if (req->state & BKT_POP_REQ) {
                if (!(req->state & BKT_REQ_DONE)) {
                    mag = magtrypop(bktid);
                    if (mag) {
                        req->state |= BKT_REQ_DONE;
                        req->mag = mag;
                    }
                }
            } else if (req->state & BKT_PUSH_REQ) {
                if (!(req->state & BKT_REQ_DONE)) {
                    mag = req->mag;
                    mag = magtrypush(mag, bktid);
                    if (!mag) {
                        req->state = BKT_REQ_NONE;
                        req->bktid = 0;
                        req->mag = NULL;
                        *reqptr = NULL;
                    }
                }
            } else if (req->state & BKT_POP_TAIL_REQ) {
                if (!(req->state & BKT_REQ_DONE)) {
                    mag = magtrypoptail(bktid);
                    if (mag) {
                        req->state |= BKT_REQ_DONE;
                        req->mag = mag;
                    }
                }
            } else if (req->state & BKT_PUSH_TAIL_REQ) {
                mag = req->mag;
                mag = magtrypushtail(mag, bktid);
                if (!mag) {
                    req->state = BKT_REQ_NONE;
                    req->bktid = 0;
                    req->mag = NULL;
                    *reqptr = NULL;
                }
                
            }
            m_cmpclrbit(&dest->slotbits, slot);
        }
    }
    
    return;
}

static __inline__ struct bktreq **
magpoptailreq(long bktid, long *slotret)
{
    struct bktreq   *req = NULL;
    struct bktreq  **reqptr = NULL;
    struct bktreqs  *dest;
    long             slot;

    dest = &bktreqs[(bktid)];
    for (slot = 0 ; slot < BKTNREQ ; slot++) {
        if (!m_cmpsetbit(&dest->slotbits, slot)) {
            reqptr = &dest->stk[slot];
            req->state = BKT_POP_TAIL_REQ;
            req->bktid = bktid;
            req->mag = NULL;
            *reqptr = req;
            m_cmpclrbit(&dest->slotbits, slot);

            break;
        }
    }
    if (reqptr) {
        *slotret = slot;
    }

    return reqptr;
}

static __inline__ struct mag *
magpoptailnb(long bktid)
{
    struct mag     *mag = NULL;
    struct bktreq  *req;
    struct bktreq **reqptr = NULL;
    struct bktreqs *dest;
    long            slot = -1;
    long            done = 0;
    long            nloop = 8;
    long            nspin;
    
    dest = &bktreqs[bktid];
    while (!mag && (nloop--)) {
        mag = magtrypoptail(bktid);
        if ((mag) && !reqptr) {
            
            return mag;
        }
        if (reqptr) {
            bkthelpreq(bktid);
            nspin = 8;
            do {
                if (!m_cmpsetbit(&dest->slotbits, slot)) {
                    done = 1;
                }
            } while (--nspin);
            if (done) {
                if (req->state & BKT_REQ_DONE) {
                    mag = req->mag;
                    req->state = BKT_REQ_NONE;
                    req->bktid = 0;
                    req->mag = NULL;
                    *reqptr = NULL;
                    reqptr = NULL;
                }
                m_cmpclrbit(&dest->slotbits, slot);
            }
        } else {
            reqptr = magpoptailreq(bktid, &slot);
            if (reqptr) {
                req = *reqptr;
            }
        }
    }
    if (reqptr) {
        do {
            ;
        } while (m_cmpsetbit(&dest->slotbits, slot));
        if (req->state & BKT_REQ_DONE) {
            mag = req->mag;
        }
        req->state = BKT_REQ_NONE;
        req->bktid = 0;
        req->mag = NULL;
        *reqptr = NULL;
        m_cmpclrbit(&dest->slotbits, slot);
    }
    
    return mag;
}

static __inline__ struct bktreq **
magpushtailreq(struct mag *mag, long bktid, long *slotret)
{
    struct bktreq   *req = NULL;
    struct bktreq  **reqptr = NULL;
    struct bktreqs  *dest;
    long             slot;
    
    dest = &bktreqs[bktid];
    /* try to post a request */
    for (slot = 0 ; slot < BKTNREQ ; slot++) {
        if (!m_cmpsetbit(&dest->slotbits, slot)) {
            req = &dest->tab[slot];
            reqptr = &dest->stk[slot];
            req->state = BKT_PUSH_TAIL_REQ;
            req->bktid = bktid;
            req->mag = mag;
            *reqptr = req;
            m_cmpclrbit(&dest->slotbits, slot);
            
            break;
        }
    }
    if (reqptr) {
        *slotret = slot;
    }

    return reqptr;
}

static __inline__ struct mag *
magpushtailnb(struct mag *mag, long bktid)                       
{
    struct mag     *src = mag;
    struct bktreqs *dest;
    struct bktreq  *req;
    struct bktreq **reqptr = NULL;
    long            slot;
    long            nloop = 8;
    long            nspin;
    long            done = 0;
    
    mag->prev = NULL;
    dest = &bktreqs[bktid];
    while ((mag) && (nloop--)) {
        mag = magtrypushtail(src, bktid);
        if (!mag && !reqptr) {
            
            return mag;
        }
        if (reqptr) {
            bkthelpreq(bktid);
            nspin = 8;
            do {
                if (!m_cmpsetbit(&dest->slotbits, slot)) {
                    done = 1;
                }
            } while (!done && (--nspin));
            if (done) {
                if (req->state & BKT_REQ_DONE) {
                    req->state = BKT_REQ_NONE;
                    req->bktid = 0;
                    req->mag = NULL;
                    *reqptr = NULL;
                    reqptr = NULL;
                    mag = NULL;
                    m_cmpclrbit(&dest->slotbits, slot);
                }
            }
        } else {
            reqptr = magpushtailreq(mag, bktid, &slot);
            if (reqptr) {
                req = *reqptr;
            }
        }
    }
    if (reqptr) {
        do {
            ;
        } while (m_cmpsetbit(&dest->slotbits, slot));
        if (req->state & BKT_REQ_DONE) {
            mag = NULL;
        }
        req->state = BKT_REQ_NONE;
        req->bktid = 0;
        req->mag = NULL;
        *reqptr = NULL;
        m_cmpclrbit(&dest->slotbits, slot);
    }
    
    return mag;
}

#endif /* MALLOCNBTAIL */

static __inline__ struct bktreq **
magpopreq(long bktid, struct mag *mag, long *slotret)
{
    struct bktreq   *req = NULL;
    struct bktreq  **reqptr = NULL;
    struct bktreqs  *dest;
    long             slot;

    dest = &bktreqs[(bktid)];
    for (slot = 0 ; slot < BKTNREQ ; slot++) {
        if (!m_cmpsetbit(&dest->slotbits, slot)) {
            reqptr = &dest->stk[slot];
            req->state = BKT_POP_REQ;
            req->bktid = bktid;
            req->mag = NULL;
            *reqptr = req;
            m_cmpclrbit(&dest->slotbits, slot);

            break;
        }
    }
    if (reqptr) {
        *slotret = slot;
    }

    return reqptr;
}

static __inline__ struct mag *
magpopnb(long bktid)
{
    struct mag     *mag = NULL;
    struct bktreq  *req;
    struct bktreq **reqptr = NULL;
    struct bktreqs *dest;
    long            slot = -1;
    long            done = 0;
    long            nloop = 8;
    long            nspin;
    
    dest = &bktreqs[bktid];
    while (!mag && (nloop--)) {
        mag = magtrypop(bktid);
        if ((mag) && !reqptr) {

            return mag;
        }
        if (reqptr) {
            bkthelpreq(bktid);
            nspin = 8;
            do {
                if (!m_cmpsetbit(&dest->slotbits, slot)) {
                    done = 1;
                }
            } while (--nspin);
            if (done) {
                if (req->state & BKT_REQ_DONE) {
                    mag = req->mag;
                    req->state = BKT_REQ_NONE;
                    req->bktid = 0;
                    req->mag = NULL;
                    *reqptr = NULL;
                    reqptr = NULL;
                }
                m_cmpclrbit(&dest->slotbits, slot);
            }
        } else {
            reqptr = magpopreq(bktid, mag, &slot);
            if (reqptr) {
                req = *reqptr;
            }
        }
    }
    if (reqptr) {
        do {
            ;
        } while (m_cmpsetbit(&dest->slotbits, slot));
        if (req->state & BKT_REQ_DONE) {
            mag = req->mag;
        }
        req->state = BKT_REQ_NONE;
        req->bktid = 0;
        req->mag = NULL;
        *reqptr = NULL;
        m_cmpclrbit(&dest->slotbits, slot);
    }
    
    return mag;
}

static __inline__ struct bktreq **
magpushreq(struct mag *mag, long bktid, long *slotret)
{
    struct bktreq   *req = NULL;
    struct bktreq  **reqptr = NULL;
    struct bktreqs  *dest;
    long             slot;
    
    dest = &bktreqs[bktid];
    /* try to post a request */
    for (slot = 0 ; slot < BKTNREQ ; slot++) {
        if (!m_cmpsetbit(&dest->slotbits, slot)) {
            req = &dest->tab[slot];
            reqptr = &dest->stk[slot];
            req->state = BKT_PUSH_REQ;
            req->bktid = bktid;
            req->mag = mag;
            *reqptr = req;
            m_cmpclrbit(&dest->slotbits, slot);
            
            break;
        }
    }
    if (reqptr) {
        *slotret = slot;
    }

    return reqptr;
}

static __inline__ struct mag *
magpushnb(struct mag *mag, long bktid)                       
{
    struct mag     *src = mag;
    struct bktreqs *dest;
    struct bktreq  *req;
    struct bktreq **reqptr = NULL;
    long            slot;
    long            nloop = 8;
    long            nspin;
    long            done = 0;
    
    mag->prev = NULL;
    dest = &bktreqs[bktid];
    while ((mag) && (nloop--)) {
        mag = magtrypush(src, bktid);
        if (!mag && !reqptr) {
            
            return mag;
        }
        if (reqptr) {
            bkthelpreq(bktid);
            nspin = 8;
            do {
                if (!m_cmpsetbit(&dest->slotbits, slot)) {
                    done = 1;
                }
            } while (!done && (--nspin));
            if (done) {
                if (req->state & BKT_REQ_DONE) {
                    req->state = BKT_REQ_NONE;
                    req->bktid = 0;
                    req->mag = NULL;
                    *reqptr = NULL;
                    reqptr = NULL;
                    mag = NULL;
                    m_cmpclrbit(&dest->slotbits, slot);
                }
            }
        } else {
            reqptr = magpushreq(mag, bktid, &slot);
            if (reqptr) {
                req = *reqptr;
            }
        }
    }
    if (reqptr) {
        do {
            ;
        } while (m_cmpsetbit(&dest->slotbits, slot));
        if (req->state & BKT_REQ_DONE) {
            mag = NULL;
        }
        req->state = BKT_REQ_NONE;
        req->bktid = 0;
        req->mag = NULL;
        *reqptr = NULL;
        m_cmpclrbit(&dest->slotbits, slot);
    }
    
    return mag;
}
    
static __inline__ void    
magrmnb(struct mag *mag, long bktid)
{
    struct magtab *bkt = &g_malloc.magbkt[bktid];
    struct mag    *prev = mag->prev;
    struct mag    *next = mag->next;
    
    if (!mag->arn) {
#if 0
        do {
            ;
        } while (m_cmpsetbit((volatile long *)&bkt->ptr, BKT_MARK_POS));
#endif
        if (prev) {
            do {
                ;
            } while (m_cmpsetbit((volatile long *)prev->next, BKT_MARK_POS));
        }
        m_cmpsetbit((volatile long *)&mag->next, BKT_MARK_POS);
        if (next) {
            do {
                ;
            } while (m_cmpsetbit((volatile long *)&next->next, BKT_MARK_POS));
        }
    }
    if ((prev) && (next)) {
        prev->next = next;
        next->prev = prev;
    } else if (prev) {
        prev->next = NULL;
    } else if (next) {
        next->prev = NULL;
        if ((mag)->arn) {
            (mag)->arn->magbkt[bktid].ptr = next;
        } else {
            (bkt)->ptr = mag->next;
        }
    } else if ((mag)->arn) {
#if (MALLOCNBTAIL)
        (mag)->arn->magbkt[bktid]. tail = NULL;
#endif
        (mag)->arn->magbkt[bktid].ptr = NULL;
    } else {
        bkt->ptr = NULL;
    }
    if (!mag->arn) {
        if (next) {
            m_cmpsetbit((volatile long *)&next->next, BKT_MARK_POS);
        }
        m_cmpclrbit((volatile long *)&mag->next, BKT_MARK_POS);
        if (prev) {
            m_cmpclrbit((volatile long *)&prev->next, BKT_MARK_POS);
        }
#if 0
        m_cmpclrbit((volatile long *)&bkt->ptr, BKT_MARK_POS);
#endif
    }
    (mag)->arn = NULL;

    return;
}

#endif /* MALLOCNBSTK */

#if (MALLOCHDRHACKS)
#define MALLOCHDRNFO (MALLOCALIGNMENT > PTRSIZE)
/*
 * this structure is here for informative purposes; note that in core, the
 * header is right before the allocated address so you need to index it with
 * negative offsets
 */
struct memhdr {
    void      *mag;     // allocation magazine header
#if (MALLOCHDRNFO)
    uint8_t    bkt;     // bucket ID for block + 2 flag bits
#endif
};
//#define MEMHDRSIZE       (sizeof(void *) + sizeof(uint8_t))
#if (MALLOCHDRNFO)
#define MEMHDRBKTOFS     (offsetof(struct memhdr, bkt))
#define MEMHDRALNBIT     (1 << 7)
#define MEMHDRFREEBIT    (1 << 6)
#define MEMHDRBKTMASK    ((1 << 6) - 1)
#define getnfo(ptr)      (((uint8_t *)(ptr))[-(1 + MEMHDRBKTOFS)])
#define setnfo(ptr, nfo) ((((uint8_t *)(ptr))[-(1 + MEMHDRBKTOFS)]) = (nfo))
#endif /* MALLOCHDRNFO */

#else /* !MALLOCHDRHACKS */
#define MALLOCHDRNFO     0
struct memhdr {
    void *mag;
};
//#define MEMHDRSIZE       (sizeof(void *))

#endif /* MALLOCHDRHACKS */

#define MEMHDRMAGOFS     (offsetof(struct memhdr, mag) / sizeof(void *))
#define setmag(ptr, mag) ((((void **)(ptr))[-(1 + MEMHDRMAGOFS)] = (mag)))
#define getmag(ptr)      ((((void **)(ptr))[-(1 + MEMHDRMAGOFS)]))

#endif /* ___MALLOC_H__ */

