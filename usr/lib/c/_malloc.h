#ifndef ___MALLOC_H__
#define ___MALLOC_H__

#define MALLOCLFDEQ       1
#define MALLOCBUFMAP      1
#define MALLOCBUFMAPTABS  1
#define ZEROFMTX          1

#define MALLOCHDRPREFIX   1
#define MALLOCHASH        0
#define MALLOCRAZOHASH    0
#define MALLOCARRAYHASH   0
#define MALLOCNEWHASH     0
#define MALLOCPRIOLK      1     // use locks lifted from locklessinc.com
#define MALLOCTAILQ       0
#define MALLOCATOMIC      1
#define MALLOCLAZYUNMAP   1

#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <malloc.h>
#include <zero/asm.h>
#include <zero/param.h>
#include <zero/trix.h>
#if (MALLOCLFDEQ)
#define LFDEQ_VAL_T    struct mag *
#define LFDEQ_VAL_NONE NULL
#include <zero/tagptr.h>
#include <zero/lfdeq.h>
#endif

/* internal stuff for zero malloc - not for the faint at heart to modify :) */

#define MALLOCNEWMULTITAB 0
#define MALLOCMULTITAB    0
#define MALLOCTKTLK       0
#define MALLOCNBTAIL      0
#define MALLOCNBDELAY     0
#define MALLOCNBSTK       0
#define MALLOCNBHEADLK    0
#define MALLOCFREEMAP     0
#define MALLOCSLABTAB     0

#define PTHREAD           1

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
#define MALLOCNEWHDR      0
/*
#define MALLOCNEWHDR      1
#define MALLOCHDRPREFIX   1
*/
#define MALLOCSMALLADR    0
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

#define MALLOCNOSBRK      0 // do NOT use sbrk()/heap, just mmap()
#define MALLOCFREETABS    0 // use free block bitmaps; bit 1 for allocated
#define MALLOCBUFMAG      0 // buffer mapped slabs to global pool

/* use zero malloc on a GNU system such as a Linux distribution */
#define GNUMALLOC         0

/* HAZARD: modifying anything below might break anything and everything BAD */

/* allocator parameters */

/* compiler-specified [GCC] alignment requirement for allocations */
#if defined(__BIGGEST_ALIGNMENT__)
#define MALLOCALIGNMENT      max(__BIGGEST_ALIGNMENT__, 2 * PTRSIZE)
#endif
#if (!defined(MALLOCALIGNMENT))
#define MALLOCALIGNMENT      4
#endif
#if (MALLOCALIGNMENT == 4)
#define MALLOCALIGNMENTSHIFT 2
#elif (MALLOCALIGNMENT == 8)
#define MALLOCALIGNMENTSHIFT 3
#elif (MALLOCALIGNMENT == 16)
#define MALLOCALIGNMENTSHIFT 4
#elif (MALLOCALIGNMENT == 32)
#define MALLOCALIGNMENTSHIFT 5
#elif (MALLOCALIGNMENT == 64)
#define MALLOCALIGNMENTSHIFT 6
#endif

#define MALLOCNHASHITEM   (1U << MALLOCNHASHBIT)
#if (PTRBITS > 32)
#define MALLOCNHASHBIT    20
#else
#define MALLOCNHASHBIT    20
#endif

/* <= MALLOCSLABLOG2 are tried to get from heap #if (!MALLOCNOSBRK) */
/* <= MALLOCSLABLOG2 are allocated 1UL << MALLOCBIGSLABLOG2 bytes per slab */
#define MALLOCSLABLOG2    17
//#define MALLOCBIGSLABLOG2 23
#define MALLOCBIGMAPLOG2  21
#define MALLOCHUGEMAPLOG2 24

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
#define VALGRINDMKPOOL(adr, z)
#define VALGRINDMARKPOOL(adr, sz)
#define VALGRINDRMPOOL(adr)
#define VALGRINDMKSUPER(adr)
#define VALGRINDPOOLALLOC(pool, adr, sz)
#define VALGRINDPOOLFREE(pool, adr)
#define VALGRINDALLOC(adr, sz, z)
#define VALGRINDFREE(adr)
#endif

#if defined (MALLOCPRIOLK) && (MALLOCPRIOLK) && !(DEBUGMTX)
#include <zero/priolk.h>
#include <zero/mtx.h>
#define LOCK struct priolk
#define __mallocinitlk(mp)
//#define __malloctrylk(mp)     priotrylk(mp)
#define __malloclk(mp)        priolk(mp)
#define __mallocunlk(mp)      priounlk(mp)
#elif defined(MALLOCTKTLK) && (MALLOCTKTLK) && !(DEBUGMTX)
#include <zero/tktlk.h>
#define LOCK union zerotktlk
#define __mallocinitlk(mp)
#define __malloctrylk(mp)     tkttrylk(mp)
#define __malloclk(mp)        tktlkspin(mp)
#define __mallocunlk(mp)      tktunlk(mp)
#elif defined(ZEROFMTX)
#define LOCK zerofmtx
#include <zero/mtx.h>
#include <zero/spin.h>
#if defined(DEBUGMTX) && (DEBUGMTX)
#define __mallocinitlk(mp)    fmtxinit(mp)
#define __malloctrylk(mp)     fmtxtrylk(mp)
#define __malloclk(mp)       (fprintf(stderr, "%p\tLK: %d\n", mp, __LINE__), \
                              fmtxlk(mp))
#define __mallocunlk(mp)     (fprintf(stderr, "%p\tUNLK: %d\n", mp, __LINE__), \
                              fmtxunlk(mp))
#define __mallocinitspin(mp)  spininit(mp)
#define __malloclkspin(mp)   (fprintf(stderr, "LK: %d\n", __LINE__),   \
                              spinlk(mp))
#define __mallocunlkspin(mp) (fprintf(stderr, "UNLK: %d\n", __LINE__), \
                              spinunlk(mp))
#elif (PTHREAD) && !(DEBUGMTX)
#include <pthread.h>
#define LOCK pthread_mutex_t
#define __mallocinitlk(mp)    pthread_mutex_init(mp, NULL)
#define __malloclk(mp)        pthread_mutex_lock(mp)
#define __mallocunlk(mp)      pthread_mutex_unlock(mp)
#else
#define __mallocinitlk(mp)    fmtxinit(mp)
#define __malloctrylk(mp)     fmtxtrylk(mp)
#define __malloclk(mp)        fmtxlk(mp)
#define __mallocunlk(mp)      fmtxunlk(mp)
#define __mallocinitspin(mp)  spininit(mp)
#define __malloctrylkspin(mp) spintrylk(mp)
#define __malloclkspin(mp)    spinlk(mp)
#define __mallocunlkspin(mp)  spinunlk(mp)
#endif
#endif

/* internal macros */
#define ptralign(ptr, pow2)                                             \
    ((void *)rounduppow2((uintptr_t)(ptr), (pow2)))

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

#if (MALLOCMULTITAB)

#define MALLOCPAGETAB     1

#if (PTRBITS == 32)

#define PAGEDIRNL1BIT     10
#define PAGEDIRNL2BIT     10

#define PAGEDIRL1NDX      (PAGEDIRNL2BIT + PAGESIZELOG2)
#define PAGEDIRL2NDX      PAGESIZELOG2

#elif (MALLOCMULTITAB) || (MALLOCNEWMULTITAB)

#if (PTRBITS == 64) && (!MALLOCSMALLADR)

#define PAGEDIRNL1BIT     16
#define PAGEDIRNL2BIT     16
#define PAGEDIRNL3BIT     (PTRBITS - PAGEDIRNL1BIT - PAGEDIRNL2BIT      \
                           - PAGESIZELOG2)

#elif (PTRBITS == 64) /* && MALLOCSMALLADR */

#define PAGEDIRNL1BIT     20
#define PAGEDIRNL2BIT     PAGESIZELOG2

#if (ADRHIBITCOPY)

#define PAGEDIRNL3BIT     (ADRBITS + 1 - PAGEDIRNL1BIT - PAGEDIRNL2BIT)

#elif (ADRHIBITZERO)

#define PAGEDIRNL3BIT     (ADRBITS - PAGEDIRNL1BIT - PAGESIZENL2BIT)

#endif /* ADRHIBITCOPY */

#endif /* PTRBITS == 64 && !MALLOCSMALLADR */

#define PAGEDIRL1NDX      (PAGEDIRL2NDX + PAGEDIRNL2BIT)
#if defined(PAGEDIRNL3BIT) && (PAGEDIRNL3BIT)
#define PAGEDIRL2NDX      (PAGEDIRL3NDX + PAGEDIRNL3BIT)
#define PAGEDIRL3NDX      PAGESIZELOG2
#else
#define PAGEDIRL2NDX      PAGESIZELOG2
#endif

#endif /* PTRBITS == 32 */

#define PAGEDIRNL1KEY     (1L << PAGEDIRNL1BIT)
#define PAGEDIRNL2KEY     (1L << PAGEDIRNL2BIT)
#if defined(PAGEDIRNL3BIT) && (PAGEDIRNL3BIT)
#define PAGEDIRNL3KEY     (1L << PAGEDIRNL3BIT)
#endif

#define pagedirl1ndx(ptr) (((uintptr_t)(ptr) >> PAGEDIRL1NDX)           \
                           & ((1UL << PAGEDIRNL1BIT) - 1))
#define pagedirl2ndx(ptr) (((uintptr_t)(ptr) >> PAGEDIRL2NDX)           \
                           & ((1UL << PAGEDIRNL2BIT) - 1))
#if defined(PAGEDIRL3NDX)
#define pagedirl3ndx(ptr) (((uintptr_t)(ptr) >> PAGEDIRL3NDX)           \
                           & ((1UL << PAGEDIRNL3BIT) - 1))
#endif

#endif /* MALLOCMULTITAB */

#if (MALLOCNEWMULTITAB)

#define MALLOC_TAB_LK_POS 0x00
#define MALLOC_TAB_LK_BIT 0x01L
#define mttrylktab(tab) (!m_cmpsetbit((volatile long *)(&(tab)->ptr),   \
                                      MALLOC_TAB_LK_POS))
#define mtlktab(tab)                                                    \
    do {                                                                \
        long _res;                                                      \
                                                                        \
        do {                                                            \
            _res = m_cmpsetbit((volatile long *)(&(tab)->ptr),          \
                               MALLOC_TAB_LK_POS);                      \
        } while (_res);                                                 \
    } while (0)
#define mtunlktab(tab)                                                  \
    m_clrbit((volatile long *)(&(tab)->ptr),                            \
             MALLOC_TAB_LK_POS)

struct memtab {
    void          *ptr;
    volatile long  nref;
};

#else

#define maglkbit(mag)   1
#define magunlkbit(mag) 0

struct memtab {
    void          *ptr;
    volatile long  nref;
};

#endif /* MALLOCNEWMULTITAB */

#if (MALLOCHDRPREFIX)
#define MALLOCHDRBASE    (MALLOCALIGNMENT >= (2 * PTRSIZE))
struct memhdr {
    void *mag;
#if (MALLOCHDRBASE)
    void *base;
#endif
};
#define MEMHDRSIZE         sizeof(struct memhdr)
#define MEMHDRMAGOFS       (offsetof(struct memhdr, mag) / sizeof(void *))
#define setmag(ptr, mag)   ((((void **)(ptr))[-(1 + MEMHDRMAGOFS)] = (mag)))
#define getmag(ptr)        ((((void **)(ptr))[-(1 + MEMHDRMAGOFS)]))
#if (MALLOCHDRBASE)
#define MEMHDRBASEOFS      (offsetof(struct memhdr, base) / sizeof(void *))
#define setbase(ptr, base) ((((void **)(ptr))[-(1 + MEMHDRBASEOFS)] = (ptr)))
#define getbase(ptr)       ((((void **)(ptr))[-(1 + MEMHDRBASEOFS)]))
#endif
#endif /* MALLOCHDRPREFIX */

#define MAGMAP        0x01UL
#define MALLOCHDRSIZE PAGESIZE
/* magazines for larger/fewer allocations embed the tables in the structure */
/* magazine header structure */
struct mag {
    struct arn    *arn;
    void          *base;
    void          *adr;
    void          *ptr;
    long           cur;
    long           lim;
    void          *stk;
#if !defined(MALLOCHDRBASE) || (!MALLOCHDRBASE)
    void          *ptrtab;
#endif
    long           bktid;
#if (MALLOCLAZYUNMAP)
    long           nfree;
#endif
    struct mag    *prev;
    struct mag    *next;
    size_t         size;
#if (MALLOCFREEMAP)
    volatile long  freelk;
    uint8_t       *freemap;
#endif
    struct magtab *tab;
};

struct magtab {
#if (MALLOCLFDEQ)
    struct lfdeq   lfdeq;
#endif
    struct mag    *ptr;
    unsigned long  n;
};

#define MALLOCARNSIZE rounduppow2(sizeof(struct arn), PAGESIZE)
/* arena structure */
struct arn {
    struct magtab magbuf[MALLOCNBKT];
};

/* magazine list header structure */

struct magbkt {
    volatile long  nref;
    struct mag    *tab;
};

static void * maginitslab(struct mag *mag, long bktid, long *zeroret);
static void * maginittab(struct mag *mag, long bktid);
static void * maginit(struct mag *mag, long bktid, long *zeroret);

#define ptrdiff(ptr1, ptr2)                                             \
    ((uintptr_t)(ptr2) - (uintptr_t)(ptr1))
#define magptrid(mag, ptr)                                              \
    (((uintptr_t)(ptr) - (uintptr_t)(mag)->base) >> (mag)->bktid)
#if (!MALLOCHDRPREFIX)
#define magputptr(mag, ptr, orig)                                       \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr)] = (orig))
#define maggetptr(mag, ptr)                                             \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr)])
#endif

#define maghdrsz(bktid)                                                 \
    (rounduppow2(sizeof(struct mag), CLSIZE))
#if (MALLOCFREEMAP)
#if (MALLOCHDRPREFIX) && (MALLOCHDRBASE)
#define magtabsz(bktid)                                                 \
    (maghdrsz()                                                         \
     + ((!magnblklog2(bktid)                                            \
         ? 0                                                            \
         : (magnblk(bktid) * sizeof(void *)                             \
            + ((magnblk(bktid) * CHAR_BIT) >> 3)))))
#else
#define magtabsz(bktid)                                                 \
    (maghdrsz()                                                         \
     + ((!magnblklog2(bktid)                                            \
         ? 0                                                            \
         : ((magnblk(bktid) << 1) * sizeof(void *)                      \
            + ((magnblk(bktid) + CHAR_BIT) >> 3)))))
#endif
#else /* !MALLOCFREEMAP */
#if (MALLOCHDRPREFIX) && (MALLOCHDRBASE)
#define magtabsz(bktid)                                                 \
    (maghdrsz()                                                         \
     + ((!magnblklog2(bktid)                                            \
         ? 0                                                            \
         : (magnblk(bktid) * sizeof(void *)))))
#else
#define magtabsz(bktid)                                                 \
    (maghdrsz()                                                         \
     + (!magnblklog2(bktid)                                             \
        ? 0                                                             \
        : (magnblk(bktid) << 1) * sizeof(void *)))
#endif
#endif

/*
 * magazines for bucket bktid have 1 << magnblklog2(bktid) blocks of
 * 1 << bktid bytes
 */
#if (MALLOCBUFMAP)
#define magnmaplog2(bktid)                                              \
    (((bktid) <= MALLOCBIGMAPLOG2)                                      \
     ? (min(MALLOCBIGMAPLOG2 - (bktid) + 2, 2))                         \
     : (((bktid <= MALLOCHUGEMAPLOG2)                                   \
         ? (min(MALLOCHUGEMAPLOG2 - (bktid) + 1, 1))                    \
         : 0)))

#define magnblklog2(bktid)                                              \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? (MALLOCSLABLOG2 - (bktid))                                       \
     : magnmaplog2(bktid))
#define magnblk(bktid)                                                  \
    (1UL << magnblklog2(bktid))

#define magnglobbuflog2(bktid)                                          \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? (PTRBITS - MALLOCSLABLOG2 - 1)                                   \
     : (((bktid) <= MALLOCBIGMAPLOG2)                                   \
        ? (ADRBITS - (bktid) - 6)                                       \
        : (((bktid <= MALLOCHUGEMAPLOG2)                                \
            ? (ADRBITS - (bktid) - 4)                                   \
            : 0))))
#define magnglobbuf(bktid)                                              \
    (1UL << magnglobbuflog2(bktid))
#define magnarnbuflog2(bktid)                                           \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? 4                                                                \
     : (((bktid) <= MALLOCBIGMAPLOG2)                                   \
        ? 3                                                             \
        : (((bktid) <= MALLOCHUGEMAPLOG2)                               \
           ? 2                                                          \
           : 1)))
#define magnarnbuf(bktid)                                               \
    (1UL << magnarnbuflog2(bktid))
#endif


#define magembedtab(bktid) (magtabsz(bktid) <= MALLOCHDRSIZE)

#define MALLOC_LK_BIT_POS 0
#define MALLOC_LK_BIT     (1L << MALLOC_LK_BIT_POS)

#if (MALLOCBUFMAG) || (MALLOCBUFMAP)
#define bktincnmag(bkt)      ((bkt)->n++)
#define bktdecnmag(bkt)      ((bkt)->n--)
#define bktaddnmag(bkt, num) ((bkt)->n += (num))
#else
#define bktincnmag(bkt)
#define bktdecnmag(bkt)
#define bktaddnmag(bkt, num)
#endif

#define magrmhead(bkt, head)                                            \
    do {                                                                \
        if ((head)->next) {                                             \
            (head)->next->prev = NULL;                                  \
        }                                                               \
        m_syncwrite(&(bkt)->ptr, (head)->next);                         \
        (head)->arn = NULL;                                             \
        (head)->prev = NULL;                                            \
        (head)->next = NULL;                                            \
        (head)->tab = NULL;                                             \
    } while (0)

#define magrm(mag, bkt, lk)                                             \
    do {                                                                \
        struct mag  *_head = NULL;                                      \
        uintptr_t    _upval;                                            \
                                                                        \
        if (lk) {                                                       \
            do {                                                        \
                ;                                                       \
            } while (m_cmpsetbit((volatile long *)&(bkt)->ptr,          \
                                 MALLOC_LK_BIT_POS));                   \
        }                                                               \
        _upval = (uintptr_t)(bkt)->ptr;                                 \
        _upval &= ~MALLOC_LK_BIT;                                       \
        _head = (struct mag *)_upval;                                   \
        if (((mag)->prev) && ((mag)->next)) {                           \
            (mag)->next->prev = (mag)->prev;                            \
            (mag)->prev->next = (mag)->next;                            \
        } else if ((mag)->prev) {                                       \
            (mag)->prev->next = NULL;                                   \
        } else if ((mag)->next) {                                       \
            (mag)->next->prev = NULL;                                   \
            _head = (mag)->next;                                        \
        } else {                                                        \
            _head = NULL;                                               \
        }                                                               \
        bktdecnmag(bkt);                                                \
        m_syncwrite((volatile long *)&(bkt)->ptr, _head);               \
        (mag)->arn = NULL;                                              \
        (mag)->prev = NULL;                                             \
        (mag)->next = NULL;                                             \
        (mag)->tab = NULL;                                              \
    } while (0)

#if (MALLOCLFDEQ)
#define magenqueue(mag, bkt)                                            \
    lfdeqenqueue(&(bkt)->lfdeq, (LFDEQ_VAL_T)(mag))
#define magdequeue(bkt)                                                 \
    lfdeqdequeue(&(bkt)->lfdeq)
#endif
#define magpop(bkt, mag)                                                \
    do {                                                                \
        struct mag *_head;                                              \
        uintptr_t   _upval;                                             \
                                                                        \
        do {                                                            \
            ;                                                           \
        } while (m_cmpsetbit((volatile long *)&(bkt)->ptr,              \
                             MALLOC_LK_BIT_POS));                       \
        _upval = (uintptr_t)(bkt)->ptr;                                 \
        _upval &= ~MALLOC_LK_BIT;                                       \
        _head = (struct mag *)_upval;                                   \
        if (_head) {                                                    \
            bktdecnmag(bkt);                                            \
            magrmhead((bkt), _head);                                    \
        } else {                                                        \
            m_syncwrite(&(bkt)->ptr, NULL);                             \
        }                                                               \
        (mag) = _head;                                                  \
    } while (0)
#define magpush(mag, bkt, lk)                                           \
    do {                                                                \
        struct mag *_head;                                              \
        uintptr_t   _upval;                                             \
                                                                        \
        if (lk) {                                                       \
            do {                                                        \
                ;                                                       \
            } while (m_cmpsetbit((volatile long *)&(bkt)->ptr,          \
                                 MALLOC_LK_BIT_POS));                   \
        }                                                               \
        _upval = (uintptr_t)(bkt)->ptr;                                 \
        _upval &= ~MALLOC_LK_BIT;                                       \
        _head = (struct mag *)_upval;                                   \
        (mag)->prev = NULL;                                             \
        if (_head) {                                                    \
            _head->prev = (mag);                                        \
        }                                                               \
        (mag)->next = _head;                                            \
        (mag)->tab = (bkt);                                             \
        bktincnmag(bkt);                                                \
        m_syncwrite(&(bkt)->ptr, (mag));                                \
    } while (0)
#define magpushmany(first, last, bkt, n)                                \
    do {                                                                \
        struct mag *_head;                                              \
        uintptr_t   _upval;                                             \
                                                                        \
        do {                                                            \
            ;                                                           \
        } while (m_cmpsetbit((volatile long *)&(bkt)->ptr,              \
                             MALLOC_LK_BIT_POS));                       \
        _upval = (uintptr_t)(bkt)->ptr;                                 \
        _upval &= ~MALLOC_LK_BIT;                                       \
        _head = (struct mag *)_upval;                                   \
        (first)->prev = NULL;                                           \
        if (_head) {                                                    \
            _head->prev = last;                                         \
        }                                                               \
        (last)->next = _head;                                           \
        bktaddnmag(bkt, n);                                             \
        m_syncwrite(&(bkt)->ptr, (first));                              \
    } while (0)

#define MALLOPT_PERTURB_BIT 0x00000001
struct mallopt {
    int action;
    int flg;
    int perturb;
    int mmapmax;
    int mmaplog2;
};

#if (MALLOCARRAYHASH)

struct hashmag {
    void       *ptr;
    struct mag *mag;
};

/* this structure is tuned for 8 machine words (cacheline) */
#define MALLOCHASHNTAB                                                  \
    ((2 * CLSIZE - offsetof(struct hashblk, tab)) / sizeof(struct hashmag))
#define MALLOCHASHLKBITPOS     (1 << (sizeof(long) * CHAR_BIT - 1))
/* member in the info-word */
#define hashgetitemcnt(blk)    ((blk)->info & 0x0f)
#define hashsetitemcnt(blk, n) ((blk)->info &= ~0x0f, (blk)->info |= ((n) & 0x0f))
struct hashblk {
    long            info;
    struct hashblk *next;
    struct hashmag  tab[15];
};

#elif (MALLOCHASH)

struct hashmag {
    volatile long   nref;
    void           *ptr;
    struct mag     *mag;
    struct hashmag *next;
};

#endif

/* malloc global structure */
#define MALLOCINIT   0x00000001L
#define MALLOCNOHEAP 0x00000002L
struct malloc {
    struct magtab            magbuf[MALLOCNBKT];
    struct magtab            freebuf[MALLOCNBKT];
    struct magtab            hdrbuf[MALLOCNBKT];
#if (MALLOCNBKT == 64)
    uint64_t                 magemptybits;
#elif (MALLOCNBKT == 32)
    uint32_t                 magemptybits;
#endif
#if (MALLOCHASHSTAT)
    struct hashstat         *hashstat;
#endif
#if (MALLOCARRAYHASH)
    struct hashblk          *hashblkbuf;
    struct hashblk         **hashtab;
    uint8_t                 *hashmap;
#elif (PTRBITS == 32)
    struct memtab            pagedir[PTRBITS - PAGESIZELOG2];
#elif (MALLOCMULTITAB)
    LOCK                    *pagedirlktab;
    struct memtab          **pagedir;
#elif (MALLOCHASH)
    struct memtab           *pagedir;
    struct hashmag          *hashbuf;
    struct hashmag         **maghash;
#else
    void                   **pagedir;
    void                   **slabdir;
#endif
#if (MALLOCTKTLK) || (MALLOCPRIOLK)
    LOCK                     initlk;
    LOCK                     heaplk;
#else
    volatile long            initlk;    // initialization lock
    volatile long            heaplk;    // lock for sbrk()
#endif
#if (MALLOCPRIOLK)
    volatile long            priolk;
    volatile unsigned long   prioval;
#endif
    volatile long            flg;       // allocator flags
    int                      zerofd;    // file descriptor for mmap()
    struct mallopt           mallopt;   // mallopt() interface
    struct mallinfo          mallinfo;  // mallinfo() interface
};

#endif /* ___MALLOC_H__ */

