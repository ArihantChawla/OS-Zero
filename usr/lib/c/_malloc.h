#ifndef ___MALLOC_H__
#define ___MALLOC_H__

#define MALLOCPRIOLK      1     // use locks lifted from locklessinc.com
#define MALLOCLFQ         1
#define MALLOCLAZYUNMAP   1

#include <limits.h>
#include <stdint.h>
#include <malloc.h>
#include <zero/asm.h>
#include <zero/param.h>
#include <zero/trix.h>
#if (MALLOCLFQ)
#define LFQ_VAL_T    struct mag *
#define LFQ_VAL_NONE NULL
#include <zero/tagptr.h>
#include <zero/lfq.h>
#endif

/* internal stuff for zero malloc - not for the faint at heart to modify :) */

#define MALLOCMAPNDX      1
#define MALLOCNEWMULTITAB 1
#define MALLOCMULTITAB    1
#define MALLOCNEWHASH     0
#define MALLOCHASH        0
#define MALLOCTKTLK       0
#define MALLOCNBTAIL      0
#define MALLOCNBDELAY     0
#define MALLOCNBSTK       0
#define MALLOCNBHEADLK    0
#define MALLOCFREEMAP     0
#define MALLOCSLABTAB     0

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
#define MALLOCNEWHDR      0
#define MALLOCHDRPREFIX   1
/*
#define MALLOCNEWHDR      1
#define MALLOCHDRPREFIX   1
*/
#define MALLOCSMALLADR    0
#define MALLOCSTAT        0
#define MALLOCPTRNDX      1
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
#define MALLOCFREETABS    1 // use free block bitmaps; bit 1 for allocated
#define MALLOCBUFMAG      1 // buffer mapped slabs to global pool

/* use zero malloc on a GNU system such as a Linux distribution */
#define GNUMALLOC         1

/* HAZARD: modifying anything below might break anything and everything BAD */

/* allocator parameters */

/* compiler-specified [GCC] alignment requirement for allocations */
#if defined(__BIGGEST_ALIGNMENT__)
#define MALLOCALIGNMENT      __BIGGEST_ALIGNMENT__
#endif
#if (!defined(MALLOCALIGNMENT))
#define MALLOCALIGNMENT      CLSIZE
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
#if (MALLOCNEWHASH)
#define MALLOCNHASHBIT    22
#else
#define MALLOCNHASHBIT    20
#endif

#if (MALLOCNEWMULTITAB)
#define MALLOCSLABLOG2    18
//#define MALLOCBIGSLABLOG2 21
#define MALLOCBIGMAPLOG2  23
#define MALLOCHUGEMAPLOG2 25
#else
/* <= MALLOCSLABLOG2 are tried to get from heap #if (!MALLOCNOSBRK) */
/* <= MALLOCSLABLOG2 are allocated 1UL << MALLOCBIGSLABLOG2 bytes per slab */
#define MALLOCSLABLOG2    18
//#define MALLOCBIGSLABLOG2 20
#define MALLOCBIGMAPLOG2  22
#define MALLOCHUGEMAPLOG2 24
#endif

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

#if (ZEROMTX)
#include <zero/mtx.h>
#endif
#if defined (MALLOCPRIOLK) && (MALLOCPRIOLK)
#include <zero/priolk.h>
#define LOCK struct priolk
#define __mallocinitlk(mp)
//#define __malloctrylk(mp)     priotrylk(mp)
#define __malloclk(mp)        priolk(mp)
#define __mallocunlk(mp)      priounlk(mp)
#elif defined(MALLOCTKTLK) && (MALLOCTKTLK)
#include <zero/tktlk.h>
#define LOCK union zerotktlk
#define __mallocinitlk(mp)
#define __malloctrylk(mp)     tkttrylk(mp)
#define __malloclk(mp)        tktlkspin(mp)
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
#if (MALLOCMAPNDX)
#define MAPNDXBITS           max(MALLOCBIGMAPLOG2 - MALLOCSLABLOG2,     \
                                 MALLOCHUGEMAPLOG2 - MALLOCBIGMAPLOG2)
#if (MAPNDXBITS < 8)
#define MAPFREE              0xff
#define MAPNDX               uint8_t
#elif (MAPNDXBITS < 16)
#define MAPFREE              0xffff
#define MAPNDX               uint16_t
#elif (MAPNDXBITS < 32)
#define MAPFREE              0xffffffff
#define MAPNDX               uint32_t
#else
#define MAPFREE              UINT64_C(0xffffffffffffffff)
#define MAPNDX               uint64_t
#endif
#else /* !MALLOCMAPNDX */
#define MAPNDX               void *
#endif
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

#if (MALLOCNEWMULTITAB)

#define MALLOC_TAB_LK_POS 0x00
#define MALLOC_TAB_LK_BIT 0x01L
#define mttrylktab(tab) (!m_cmpsetbit((volatile long *)(&(tab)->ptr),   \
                                      MALLOC_TAB_LK_POS))
#define mtlktab(tab)                                                    \
    do {                                                                \
        long _res;                                                      \
                                                                        \
        _res = m_cmpsetbit((volatile long *)(&(tab)->ptr),              \
                           MALLOC_TAB_LK_POS);                          \
    } while (res)
#define mtunlktab(tab)                                                  \
    m_cmpclrbit((volatile long *)(&(tab)->ptr),                         \
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

#define MAGMAP          0x01
#define ADRMASK         (MAGMAP)
#define PTRFLGMASK      (MAGMAP)
#define PTRADRMASK      (~PTRFLGMASK)
#define MALLOCHDRSIZE   PAGESIZE
/* magazines for larger/fewer allocations embed the tables in the structure */
/* magazine header structure */
struct mag {
#if (MALLOCLFQ)
    struct lfqnode  node;
    long            nfree;
#endif
    struct mag     *prev;
    struct mag     *next;
    volatile long   lk;
    struct arn     *arn;
    void           *base;
    void           *adr;
    uint8_t        *ptr;
    size_t          size;
    long            cur;
    long            lim;
#if (MALLOCFREEMAP)
    volatile long   freelk;
    uint8_t        *freemap;
#endif
    long            bktid;
#if (MALLOCPTRNDX)
    PTRNDX         *stk;
    PTRNDX         *idtab;
#else
    void           *stk;
    void           *ptrtab;
#endif
};

struct magtab {
    LOCK           lk;
    struct mag    *ptr;
#if (MALLOCBUFMAG)
    unsigned long  n;
#endif
#if (MALLOCBUFMAG)
    uint8_t        _pad[CLSIZE - sizeof(LOCK) - 2 * sizeof(void *) - sizeof(long)];
#else
    uint8_t        _pad[CLSIZE - sizeof(LOCK) - 2 * sizeof(void *)];
#endif
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
    ((void *)((uint8_t *)(mag)->base + (ndx * (1UL << (mag)->bktid))))
#define maggetptr(mag, ptr)                                             \
    (((void **)(mag)->idtab)[magptrid(mag, ptr)])
#else
#define magptrid(mag, ptr)                                              \
    (((uintptr_t)clrptr(ptr) - (uintptr_t)(mag)->base) >> (mag)->bktid)
#define magputptr(mag, ptr, orig)                                       \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr)] = (orig))
#define maggetptr(mag, ptr)                                             \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr)])
#endif

/*
 * magazines for bucket bktid have 1 << magnblklog2(bktid) blocks of
 * 1 << bktid bytes
 */
#define magnmaplog2(bktid)                                              \
    (((bktid) < MALLOCBIGMAPLOG2)                                       \
     ? (MALLOCBIGMAPLOG2 - (bktid))                                     \
     : (((bktid < MALLOCHUGEMAPLOG2)                                    \
         ? (MALLOCHUGEMAPLOG2 - (bktid))                                \
         : 0)))

#if 0
#define magnblklog2(bktid)                                              \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? (MALLOCBIGSLABLOG2 - (bktid))                                    \
     : magnmaplog2(bktid))
#endif
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
     ? 3                                                                \
     : (((bktid) <= MALLOCBIGMAPLOG2)                                   \
        ? 2                                                             \
        : (((bktid) <= MALLOCHUGEMAPLOG2)                               \
           ? 1                                                          \
           : 0)))
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

#define arnrmhead(tab, head)                                            \
    do {                                                                \
        if ((head)->next) {                                             \
            (head)->next->prev = NULL;                                  \
        } else {                                                        \
            (tab)->tail = NULL;                                         \
        }                                                               \
        (tab)->ptr = (head)->next;                                      \
        (head)->arn = NULL;                                             \
    } while (0)

#define arnrmtail(tab, tail)                                            \
    do {                                                                \
        if ((tail)->prev) {                                             \
            (tab)->tail = (tail)->prev;                                 \
        } else {                                                        \
            (tab)->ptr = NULL;                                          \
            (tab)->tail = NULL;                                         \
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
            (mag)->arn->magbuf[bktid].ptr = (mag)->next;                \
        } else if ((mag)->arn) {                                        \
            (mag)->arn->magbuf[bktid].ptr = NULL;                       \
        }                                                               \
        (mag)->arn = NULL;                                              \
    } while (0)

#define magrmhead(tab, head)                                            \
    do {                                                                \
        if ((head)->next) {                                             \
            (head)->next->prev = NULL;                                  \
        }                                                               \
        (tab)->ptr = (head)->next;                                      \
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
                (mag)->arn->magbuf[bktid].ptr = (mag)->next;            \
            } else {                                                    \
                (bkt)->ptr = (mag)->next;                               \
            }                                                           \
        } else if ((mag)->arn) {                                        \
            (mag)->arn->magbuf[bktid].ptr = NULL;                       \
        } else {                                                        \
            (bkt)->ptr = NULL;                                          \
        }                                                               \
        if ((lock) && !mag->arn) {                                      \
            __mallocunlk(&(bkt)->lk);                                   \
        }                                                               \
        (mag)->arn = NULL;                                              \
    } while (0)

#define magrmhead(tab, head)                                            \
    do {                                                                \
        if ((head)->next) {                                             \
            (head)->next->prev = NULL;                                  \
        }                                                               \
        (tab)->ptr = (head)->next;                                      \
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
                (mag)->arn->magbuf[bktid].ptr = (mag)->next;            \
            } else {                                                    \
                (bkt)->ptr = (mag)->next;                               \
            }                                                           \
        } else if ((mag)->arn) {                                        \
            (mag)->arn->magbuf[bktid].ptr = NULL;                       \
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

#if (MALLOCLFQ)
#define magenqueue(mag, bkt) (mag->node.val = mag, lfqenqueue(bkt, &mag->node))
#define magdequeue(bkt)      lfqdequeue(bkt)
#endif
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
#if (MALLOCLFQ)
#define magpush(mag, bkt, lock)                                         \
    do {                                                                \
        (mag)->next = (bkt)->ptr;                                       \
        if ((mag)->next) {                                              \
            (mag)->next->prev = (mag);                                  \
        }                                                               \
        bktaddmag(bkt);                                                 \
        (bkt)->ptr = (mag);                                             \
    } while (0)
#define magpushmany(first, last, bkt, lock, n)                          \
    do {                                                                \
        (first)->prev = NULL;                                           \
        (last)->next = (bkt)->ptr;                                      \
        if ((last)->next) {                                             \
            (last)->next->prev = (last);                                \
        }                                                               \
        bktaddmany(bkt, n);                                             \
        (bkt)->ptr = (first);                                           \
    } while (0)
#else
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

struct hashmag {
    volatile long   nref;
    uintptr_t       upval;
    struct mag     *adr;
    struct hashmag *next;
};

/* malloc global structure */
#define MALLOCINIT   0x00000001L
#define MALLOCNOHEAP 0x00000002L
struct malloc {
#if (MALLOCLFQ)
    struct lfq               magbuf[MALLOCNBKT];
#if (MALLOCLAZYUNMAP) && 0
    struct lfq               mapbuf[MALLOCNBKT];
#endif
    struct lfq               freebuf[MALLOCNBKT];
    struct lfq               hdrbuf[MALLOCNBKT];
#else
    struct magtab            magbuf[MALLOCNBKT];
    struct magtab            freebuf[MALLOCNBKT];
    struct magtab            hdrbuf[MALLOCNBKT];
#endif
#if (MALLOCNBKT == 64)
    uint64_t                 magemptybits;
#elif (MALLOCNBKT == 32)
    uint32_t                 magemptybits;
#endif
#if (PTRBITS == 32)
    struct memtab            pagedir[PTRBITS - PAGESIZELOG2];
#elif (MALLOCNEWMULTITAB)
    struct memtab           *pagedir;
#elif (MALLOCHASH)
    struct hashmag          *hashbuf;
    struct hashmag         **maghash;
#elif (MALLOCFREETABS)
    LOCK                    *pagedirlktab;
    struct memtab           *pagedir;
#if 0
    LOCK                    *slabdirlktab;
    struct memtab           *slabdir;
#endif
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

#if (MALLOCHDRPREFIX)

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

#endif /* (MALLOCHDRPREFIX) */

#endif /* ___MALLOC_H__ */

