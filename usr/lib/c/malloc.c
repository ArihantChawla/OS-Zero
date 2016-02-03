/*
 * Zero Malloc Revision 2
 *
 * - a beta version of a new malloc for Zero.
 *
 * Copyright (C) Tuomo Petteri Venäläinen 2014-2015
 */

#define MALLOCHDRHACKS  1
#define MALLOCNEWHDR    1
#define MALLOCHDRPREFIX 1
#define MALLOCTLSARN 1
#define DEBUGMTX 0
#define GNUTRACE 1
#define MALLOCTRACE 1

/* use zero malloc on a GNU system such as a Linux distribution */
#define GNUMALLOC 0

#if defined(__BIGGEST_ALIGNMENT__)
#define MALLOCALIGNMENT   __BIGGEST_ALIGNMENT__
#endif

#if !defined(MALLOCDEBUG)
#define MALLOCDEBUG       1
#endif
#if !defined(GNUTRACE)
#define GNUTRACE          0
#endif

#define MALLOCSMALLADR    0

/*
 * TODO
 * ----
 * - fix mallinfo() to return proper information
 */
#define MALLOCSTAT        0
#define MALLOCSTKNDX      1
#define MALLOCCONSTSLABS  1
#define MALLOCDYNARN      0
#define MALLOCGETNPROCS   0

#if defined(NVALGRIND)
#define MALLOCVALGRIND    0
#else
#define MALLOCVALGRIND    1
#endif
#define MALLOCSMALLSLABS  0
#define MALLOCSIG         1
#define MALLOC4LEVELTAB   1

#define MALLOCNOPTRTAB    0
#define MALLOCNARN        16
#define MALLOCEXPERIMENT  0
#define MALLOCNBUFHDR     16

#define ZMALLOCDEBUGHOOKS 0
#define MALLOCSTEALMAG    0
#define MALLOCNEWHACKS    0

#define MALLOCNOSBRK      0 // do NOT use sbrk()/heap, just mmap()
#define MALLOCDIAG        0 // run [heavy] internal diagnostics for debugging
#define MALLOCFREEMDIR    1 // under construction
#define MALLOCFREEMAP     0 // use free block bitmaps; bit 1 for allocated
#define MALLOCHACKS       0 // enable experimental features
#define MALLOCBUFMAP      1 // buffer mapped slabs to global pool

/*
 * THANKS
 * ------
 * - Matthew 'kinetik' Gregan for pointing out bugs, giving me cool routines to
 *   find more of them, and all the constructive criticism etc.
 * - Thomas 'Freaky' Hurst for patience with early crashes, 64-bit hints, and
 *   helping me find some bottlenecks.
 * - Henry 'froggey' Harrington for helping me fix issues on AMD64.
 * - Dale 'swishy' Anderson for the enthusiasm, encouragement, and everything
 *   else.
 * - Martin 'bluet' Stensgård for an account on an AMD64 system for testing
 *   earlier versions.
 */

/*
 *        malloc buffer layers
 *        --------------------
 *
 *                --------
 *                | mag  |----------------
 *                --------               |
 *                    |                  |
 *                --------               |
 *                | slab |               |
 *                --------               |
 *        --------  |  |   -------  -----------
 *        | heap |--|  |---| map |--| headers |
 *        --------         -------  -----------
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
 *          - supports both heap (sbrk()) and mapped (mmap()) regions
 *
 *        heap
 *        ----
 *        - process heap segment
 *          - sbrk() interface; needs global lock
 *          - mostly for small size allocations
 *
 *        map
 *        ---
 *        - process map segment
 *          - mmap() interface; thread-safe
 *          - returns readily zeroed memory
 *
 *        headers
 *        -------
 *        - mapped internal book-keeping for magazines
 *          - pointer stacks
 *          - table to map allocation pointers to magazine pointers
 *            - may differ because of alignments etc.
 *          - optionally, a bitmap to denote allocated slices in magazines
 */

/*
 * TODO
 * ----
 * - free inactive subtables from mdir
 */

#define ZMALLOCHOOKS 0
#if !defined(ZMALLOCDEBUGHOOKS)
#define ZMALLOCDEBUGHOOKS 0
#endif

struct mag;
static void * maginit(struct mag *mag, long bktid, void *base);
static void * maginitslab(struct mag *mag, long bktid, void *base);
static void * maginittab(struct mag *mag, long bktid, void *base);

#if defined(MALLOCDEBUG)
#define assert(expr)                                                    \
    do {                                                                \
        if (!(expr)) {                                                  \
            *((long *)NULL) = 0;                                        \
        }                                                               \
    } while (0)
//#include <assert.h>
#endif
#include <features.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#if (MALLOCFREEMAP)
#include <limits.h>
#endif
#include <errno.h>
#include <malloc.h>
#if (GNUTRACE) && (MALLOCTRACE)
#include <execinfo.h>
#endif
#if (MALLOCGETNPROCS) && 0
#include <sys/sysinfo.h>
#endif
#define PTHREAD 1
#define ZEROMTX 1
#if defined(ZEROMTX) && (ZEROMTX)
#undef PTHREAD
#define MUTEX volatile long
#include <zero/mtx.h>
#include <zero/spin.h>
#if (DEBUGMTX) && 0
#define __mallocinitmtx(mp)  mtxinit(mp)
#define __malloclkmtx(mp)   (fprintf(stderr, "LK: %d\n", __LINE__),    \
                             mtxlk(mp))
#define __mallocunlkmtx(mp) (fprintf(stderr, "UNLK: %d\n", __LINE__),  \
                             mtxunlk(mp))
#define __mallocinitspin(mp) spininit(mp)
#define __malloclkspin(mp)   (fprintf(stderr, "LK: %d\n", __LINE__), spinlk(mp))
#define __mallocunlkspin(mp) (fprintf(stderr, "UNLK: %d\n", __LINE__), spinunlk(mp))
#else
#define __mallocinitmtx(mp)  mtxinit(mp)
#define __malloclkmtx(mp)    mtxlk(mp)
#define __mallocunlkmtx(mp)  mtxunlk(mp)
#if 0
#define __mallocinitspin(mp) spininit(mp)
#define __malloclkspin(mp)   spinlk(mp)
#define __mallocunlkspin(mp) spinunlk(mp)
#endif
#endif
#elif (PTHREAD)
#define MUTEX pthread_mutex_t
#define __mallocinitmtx(mp) pthread_mutex_init(mp, NULL)
#define __malloclkmtx(mp)   pthread_mutex_lock(mp)
#define __mallocunlkmtx(mp) pthread_mutex_unlock(mp)
#endif
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/trix.h>
#if (GNUTRACE) && (MALLOCTRACE)
#include <zero/gnu.h>
#endif
#if (ZMALLOCDEBUGHOOKS)
#include <zero/asm.h>
#endif

#if (MALLOCVALGRIND)
#include <valgrind/valgrind.h>
#endif

#if defined(GNUMALLOC) && (GNUMALLOC)
void * zero_malloc(size_t size);
void * zero_realloc(void *ptr, size_t size);
void * zero_memalign(size_t align,  size_t size);
void   zero_free(void *ptr);
#endif /* GNUMALLOC */

#if (MALLOCHDRPREFIX)
#if (MALLOCNEWHDR)
#if (MALLOCHDRHACKS)
#define MEMHDRSIZE   (sizeof(void *) + 2)
#define MEMHDRBKTOFS (offsetof(struct memhdr, bkt))
/*
 * this structure is here for informative purposes; note that in core, the
 * header is right before the allocated address so you need to index it with
 * negative offsets
 */
struct memhdr {
    void *mag;
#if (MALLOCHDRHACKS)
    uint8_t bkt;
    uint8_t aln;
#endif
};
#else
#define MEMHDRSIZE   PTRSIZE
#endif
#endif
#define getmag(ptr)      (((void **)(ptr))[-1])
#define setmag(ptr, mag) (((void **)(ptr))[-1] = (mag))
#define getbkt(ptr)      (*((uint8_t *)(ptr)[-
#if 0
#define getmag(ptr)  (((struct memhdr *)(ptr))[-1].mag)
#define setmag(p, m) (((struct memhdr *)(ptr))[-1].mag = (m))
#endif
//#define setptr(a, p) ((&((struct memhdr *)(a))[-1])->base = (p))
#if (MALLOCSTKNDX)
#define getptr(ptr)                                                     \
    ((getmag(ptr))->ptrtab[magptrid(getmag(ptr), ptr)])
#define setptr(mag, ptr, orig)                                          \
    ((mag)->ptrtab[magptrid(mag, ptr)] = (orig))
#endif
#else
#define MEMHDRSIZE (2 * PTRSIZE)
struct memhdr {
    void *base;
    void *mag;
};
#define getptr(ptr)  ((&((struct memhdr *)(ptr))[-1])->base)
#define setptr(a, p) ((&((struct memhdr *)(a))[-1])->base = (p))
#define getmag(ptr)  (((struct memhdr *)(ptr))[-1]).mag)
#define setmag(p, m) (((struct memhdr *)(ptr))[-1]).mag = (m))
#endif

/* invariant parameters */
#define MALLOCMINSIZE        (1UL << MALLOCMINLOG2)
//#define MALLOCMINLOG2        CLSIZELOG2
#if defined(MALLOCALIGNMENT) && (MALLOCALIGNMENT == 32)
#define MALLOCMINLOG2        5  // SIMD types
#elif defined(MALLOCALIGNMENT) && (MALLOCALIGNMENT == 16)
#define MALLOCMINLOG2        4  // SIMD types
#else
#define MALLOCMINLOG2        3  // double
#endif
#define MALLOCNBKT           PTRBITS
/* allocation sizes */
#define MALLOCSLABLOG2       16
#define MALLOCBIGSLABLOG2    20
#define MALLOCBIGMAPLOG2     24
#if 0
#if (MALLOCBUFMAP)
#define MALLOCSMALLMAPLOG2   21
#define MALLOCMIDMAPLOG2     23
#endif
#if (MALLOCSMALLSLABS)
#define MALLOCBIGSLABLOG2    20
#define MALLOCSLABLOG2       16
#else
#define MALLOCSUPERSLABLOG2  22
#define MALLOCSLABLOG2       20
#define MALLOCTINYSLABLOG2   8
#define MALLOCSMALLSLABLOG2  14
#define MALLOCMIDSLABLOG2    16
#define MALLOCBIGSLABLOG2    18
#define MALLOCSMALLMAPLOG2   22
#define MALLOCMIDMAPLOG2     24
#define MALLOCBIGMAPLOG2     26
#endif
#endif /* 0 */
/* use non-pointers in allocation tables */
#if (MALLOCSTKNDX)
#if (MALLOCCONSTSLABS)
#if (MALLOCSLABLOG2 - MALLOCMINLOG2 <= 8)
#define MAGPTRNDX            uint8_t
#elif (MALLOCSLABLOG2 - MALLOCMINLOG2 <= 16)
#define MAGPTRNDX            uint16_t
#elif (MALLOCSLABLOG2 - MALLOCMINLOG2 <= 32)
#define MAGPTRNDX            uint32_t
#else
#define MAGPTRNDX            uint64_t
#endif
#else /* !MALLOCCONSTSLABS */
#if (MALLOCSUPERSLABLOG2 - MALLOCMINLOG2 <= 16)
#define MAGPTRNDX            uint16_t
#else
#define MAGPTRNDX            uint32_t
#endif
#endif
#endif

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

/*
 * magazines for bucket bktid have 1 << magnblklog2(bktid) blocks of
 * 1 << bktid bytes
 */
#if (MALLOCBUFMAP)
#if (MALLOCCONSTSLABS)
#define magnbufmaplog2 0
#if 0
#define magnbufmaplog2(bktid)                                           \
    (((bktid) <= MALLOCSMALLMAPLOG2)                                    \
     ? 3                                                                \
     : (((bktid) <= MALLOCMIDMAPLOG2)                                   \
        ? 2                                                             \
        : 1))
#endif
#else
#define magnbufmaplog2(bktid)                                           \
    (((bktid) <= MALLOCSMALLMAPLOG2)                                    \
     ? 4                                                                \
     : (((bktid) <= MALLOCMIDMAPLOG2)                                   \
        ? 3                                                             \
        : 2))
#endif
#define magnbufmap(bktid)                                               \
    (1UL << magnbufmaplog2(bktid))
#endif /* MALLOCBUFMAP */
#if (MALLOCCONSTSLABS)
#define magnbytelog2(bktid)                                             \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? MALLOCSLABLOG2                                                   \
     : (bktid))
#else /* !MALLOCCONSTSLABS */
#define magnbytelog2(bktid)                                             \
    (((bktid) <= MALLOCTINYSLABLOG2)                                    \
     ? MALLOCSMALLSLABLOG2                                              \
     : (((bktid) <= MALLOCSMALLSLABLOG2)                                \
        ? MALLOCMIDSLABLOG2                                             \
        : (((bktid) <= MALLOCMIDSLABLOG2)                               \
           ? MALLOCBIGSLABLOG2                                          \
           : (((bktid) <= MALLOCSLABLOG2)                               \
              ? MALLOCSUPERSLABLOG2                                     \
              : (((bktid) <= MALLOCSMALLMAPLOG2)                        \
                 ? MALLOCMIDMAPLOG2                                     \
                 : (((bktid) <= MALLOCMIDMAPLOG2)                       \
                    ? MALLOCBIGMAPLOG2                                  \
                    : (bktid)))))))
#endif
#define magnblklog2(bktid)                                              \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? (MALLOCSLABLOG2 - (bktid))                                       \
     : 0)
#define magnblk(bktid)                                                  \
    (1UL << magnblklog2(bktid))

struct bkt {
    volatile long  lk;
    void          *ptr;
    volatile long  nref;
    uint8_t        _pad[CLSIZE - 2 * sizeof(long) - sizeof(void *)];
};

#if 0
#define maglkbit(mag)   (!m_cmpsetbit((volatile long *)&mag->adr, 0))
#define magunlkbit(mag) (m_cmpclrbit((volatile long *)&mag->adr, 0))
#endif
#define maglkbit(mag)   1
#define magunlkbit(mag) 0
#define MAGLOCK         0x01
#define MAGMAP          0x02
#define MAGGLOB         0x04
#define BLKDIRTY        0x01
#define ADRMASK         (MALLOCMINSIZE - 1)
#define MALLOCHDRSIZE   PAGESIZE
/* magazines for larger/fewer allocations embed the tables in the structure */
/* magazine header structure */
struct mag {
    struct memtab *tab;
    void          *base;
    void          *adr;
    long           cur;
    long           lim;
#if (!MALLOCTLSARN)
    long           arnid;
#endif
#if (MALLOCFREEMAP)
    uint8_t       *freemap;
#endif
    long           bktid;
    struct mag    *prev;
    struct mag    *next;
#if (MALLOCHDRPREFIX)
    void         **stk;
#if (MALLOCNEWHDR)
    void         **ptrtab;
#endif
#elif (MALLOCFREEMAP)
    void         **stk;
    void         **ptrtab;
#elif (MALLOCSTKNDX)
    MAGPTRNDX     *stk;
    MAGPTRNDX     *ptrtab;
#endif
//    unsigned char  data[EMPTY] ALIGNED(CLSIZE);
};

#define maghdrsz()                                                      \
    (rounduppow2(sizeof(struct mag), CLSIZE))
#if (MALLOCFREEMAP)
#define magtabsz(bktid)                                                 \
    ((maghdrsz()                                                        \
      + (magnblk(bktid) << 1) * sizeof(void *)                          \
      + ((magnblk(bktid) + CHAR_BIT) >> 3)))
#else
#define magtabsz(bktid)                                                 \
    (maghdrsz() + (magnblk(bktid) << 1) * sizeof(void *))
#endif
#define magembedtab(bktid)                                              \
    (magtabsz(bktid) <= MALLOCHDRSIZE)

#if (MALLOCDEBUG) || (MALLOCDIAG)
void
magprint(struct mag *mag)
{
    fprintf(stderr, "MAG %p\n", mag);
    fprintf(stderr, "\tadr\t%p\n", mag->base);
    fprintf(stderr, "\tcur\t%ld\n", mag->cur);
    fprintf(stderr, "\tlim\t%ld\n", mag->lim);
    fprintf(stderr, "\tbktid\t%ld\n", mag->bktid);
#if (MALLOCFREEMAP)
    fprintf(stderr, "\tfreemap\t%p\n", mag->freemap);
#endif
    fprintf(stderr, "\tstk\t%p\n", mag->stk);
    fprintf(stderr, "\tptrtab\t%p\n", mag->ptrtab);
    fflush(stderr);

    return;
}
#endif

struct memtab {
    MUTEX          lk;
    void          *ptr;
#if (MALLOCBUFMAP)
    unsigned long  n;
    uint8_t        _pad[CLSIZE - 2 * sizeof(long) - sizeof(struct mag *)];
#else
    uint8_t        _pad[CLSIZE - sizeof(long) - sizeof(struct mag *)];
#endif
};

/* magazine list header structure */

struct magbkt {
    long        nref;
    struct mag *tab;
};

#define MALLOCARNSIZE      rounduppow2(sizeof(struct arn), PAGESIZE)
/* arena structure */
struct arn {
    struct memtab  magbkt[MALLOCNBKT];
#if (!MALLOCTLSARN)
    MUTEX          nreflk;
    long           nref;
#endif
};

#define MALLOPT_PERTURB_BIT 0x00000001
struct mallopt {
    int action;
    int flg;
    int perturb;
    int mmapmax;
    int mmaplog2;
};

/* malloc global structure */
#define MALLOCINIT 0x00000001L
struct malloc {
    struct memtab     magbkt[MALLOCNBKT];
    struct memtab     freetab[MALLOCNBKT];
    struct memtab     hdrbuf[MALLOCNBKT];
#if (!MALLOCTLSARN)
    struct arn      **arntab;           // arena structures
#endif
    MUTEX            *mlktab;
    struct magtab   **mdir;             // allocation header lookup structure
    MUTEX             initlk;           // initialization lock
    MUTEX             heaplk;           // lock for sbrk()
#if (!MALLOCTLSARN)
    long              curarn;
    long              narn;             // number of arenas in action
#endif
    long              flags;            // allocator flags
    int               zerofd;           // file descriptor for mmap()
    struct mallopt    mallopt;          // mallopt() interface
    struct mallinfo   mallinfo;         // mallinfo() interface
};

static struct malloc g_malloc ALIGNED(PAGESIZE);
#if (MALLOCTLSARN)
THREADLOCAL struct arn thrarn;
#else
THREADLOCAL          pthread_key_t _thrkey;
THREADLOCAL long     _arnid = -1;
MUTEX                _arnlk;
#endif
//long                 curarn;
#if (MALLOCSTAT)
long long            nheapbyte;
long long            nmapbyte;
long long            ntabbyte;
#endif

#if 0
#if defined(GNUMALLOC) && (GNUMALLOC)
#if !defined(__MALLOC_HOOK_VOLATILE)
#define MALLOC_HOOK_MAYBE_VOLATILE /**/
#else
#define MALLOC_HOOK_MAYBE_VOLATILE __MALLOC_HOOK_VOLATILE
#endif
extern void *(* MALLOC_HOOK_MAYBE_VOLATILE __malloc_hook)(size_t size,
                                                          const void *caller);
extern void *(* MALLOC_HOOK_MAYBE_VOLATILE __realloc_hook)(void *ptr,
                                                           size_t size,
                                                           const void *caller);
extern void *(* MALLOC_HOOK_MAYBE_VOLATILE __memalign_hook)(size_t align,
                                                            size_t size,
                                                            const void *caller);
extern void  (* MALLOC_HOOK_MAYBE_VOLATILE __free_hook)(void *ptr,
                                                        const void *caller);
extern void  (* MALLOC_HOOK_MAYBE_VOLATILE __malloc_initialize_hook)(void);
extern void  (* MALLOC_HOOK_MAYBE_VOLATILE __after_morecore_hook)(void);
#elif defined(_GNU_SOURCE) && defined(GNUMALLOCHOOKS) && !defined(__GLIBC__)
void  (* MALLOC_HOOK_MAYBE_VOLATILE __malloc_initialize_hook)(void);
void  (* MALLOC_HOOK_MAYBE_VOLATILE __after_morecore_hook)(void);
void *(* MALLOC_HOOK_MAYBE_VOLATILE __malloc_hook)(size_t size,
                                                   const void *caller);
void *(* MALLOC_HOOK_MAYBE_VOLATILE __realloc_hook)(void *ptr,
                                                    size_t size,
                                                    const void *caller);
void *(* MALLOC_HOOK_MAYBE_VOLATILE __memalign_hook)(size_t align,
                                                     size_t size,
                                                     const void *caller);
void  (* MALLOC_HOOK_MAYBE_VOLATILE __free_hook)(void *ptr,
                                                 const void *caller);
#endif
#endif /* 0 */

#if (MALLOCSTAT)
void
mallocstat(void)
{
    fprintf(stderr, "HEAP: %lld KB\tMAP: %lld KB\tTAB: %lld KB\n",
            nheapbyte >> 10,
            nmapbyte >> 10,
            ntabbyte >> 10);
    fflush(stderr);

    return;
}
#endif

/* allocation pointer flag bits */
#if 0
#define BLKDIRTY    0x01
#define BLKFLGMASK  (MALLOCMINSIZE - 1)
#endif
/* clear flag bits at allocation time */
#define clrptr(ptr)                                                     \
    ((void *)((uintptr_t)ptr & ~ADRMASK))
#define clradr(adr)                                                     \
    ((uintptr_t)(adr) & ADRMASK)
#define ptrdiff(p1, p2)                                                 \
    ((uintptr_t)(p2) - (uintptr_t)(p1))
#define magptrid(mag, ptr)                                              \
    ((ptrdiff((mag)->base, ptr) >> ((mag)->bktid)))

#if (MALLOCFREEMAP)
#endif
#if (MALLOCSTKNDX)
#if (MALLOCFREEMAP)
#define magnbytetab(bktid)                                              \
    ((magnblk(bktid) << 1) * sizeof(MAGPTRNDX)                          \
     + rounduppow2(((magnblk(bktid) + CHAR_BIT) >> 3), PAGESIZE))
#else /*  MALLOCSTKNDX && !MALLOCFREEMAP */
#define magnbytetab(bktid)                                              \
    ((magnblk(bktid) << 1) * sizeof(MAGPTRNDX))
#endif /* MALLOCFREEMAP */
#elif (MALLOCFREEMAP) /* !MALLOCSTKNDX */
#if (MALLOCSTKNDX)
#define magnbytetab(bktid)                                              \
    ((magnblk(bktid) << 1) * sizeof(MAGPTRNDX)                            \
     + rounduppow2((magbnlk(bktid) >> 3) + CHAR_BIT, PAGESIZE))
#else
#define magnbytetab(bktid)                                              \
    ((magnblk(bktid) << 1) * sizeof(void *)                             \
     + rounduppow2((1UL << magnblklog2(bktid)) / CHAR_BIT, PAGESIZE))
#endif
#elif (MALLOCNOPTRTAB)
#define magnbytetab(bktid)                                              \
    ((1UL << magnblk(bktid)) * sizeof(void *))
#endif
#define magnbyte(bktid) (1UL << magnbytelog2(bktid))
#define ptralign(ptr, pow2)                                             \
    (!((uintptr_t)ptr & (align - 1))                                    \
     ? ptr                                                              \
     : ((void *)rounduppow2((uintptr_t)ptr, align)))
#define blkalignsz(sz, aln)                                             \
    (((aln) <= PAGESIZE)                                                \
     ? max(sz, aln)                                                     \
     : (sz) + (aln))

#if (MALLOCSTKNDX)
#define magputptr(mag, ptr1, ptr2)                                      \
    ((mag)->ptrtab[magptrid(mag, ptr1)] = magptrid(mag, ptr2))
#define maggetptr(mag, ptr)                                             \
    ((mag)->ptrtab[magptrid(mag, magptrid(mag, ptr))])
#else /* !MALLOCSTKNDX */
#define magputptr(mag, ptr1, ptr2)                                      \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr1)] = (ptr2))
#define maggetptr(mag, ptr)                                             \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr)])
#endif /* MALLOCSTKNDX */

#define mdirl1ndx(ptr) (((uintptr_t)(ptr) >> MDIRL1NDX) & ((1UL << MDIRNL1BIT) - 1))
#define mdirl2ndx(ptr) (((uintptr_t)(ptr) >> MDIRL2NDX) & ((1UL << MDIRNL2BIT) - 1))
#define mdirl3ndx(ptr) (((uintptr_t)(ptr) >> MDIRL3NDX) & ((1UL << MDIRNL3BIT) - 1))
#define mdirl4ndx(ptr) (((uintptr_t)(ptr) >> MDIRL4NDX) & ((1UL << MDIRNL4BIT) - 1))

#if (PTRBITS == 32)
#define MDIRNL1BIT     10
#define MDIRNL2BIT     10
#define MDIRNL3BIT     (PTRBITS - MDIRNL1BIT - MDIRNL2BIT - MALLOCMINLOG2)
#define MDIRNL1KEY     (1L << MDIRNL1BIT)
#define MDIRNL2KEY     (1L << MDIRNL2BIT)
#define MDIRNL3KEY     (1L << MDIRNL3BIT)
#elif (MALLOC4LEVELTAB)
#define MDIRNL1BIT     16
#define MDIRNL2BIT     12
#define MDIRNL3BIT     12
#if 0
#define MDIRNL1BIT     12
#define MDIRNL2BIT     12
#define MDIRNL3BIT     12
#endif
#if (MALLOCSMALLADR)
#define MDIRNL4BIT     (ADRBITS - MDIRNL1BIT - MDIRNL2BIT - MDIRNL3BIT - MALLOCMINLOG2)
#else
#define MDIRNL4BIT     (PTRBITS - MDIRNL1BIT - MDIRNL2BIT - MDIRNL3BIT - MALLOCMINLOG2)
#endif
#define MDIRNL1KEY     (1L << MDIRNL1BIT)
#define MDIRNL2KEY     (1L << MDIRNL2BIT)
#define MDIRNL3KEY     (1L << MDIRNL3BIT)
#define MDIRNL4KEY     (1L << MDIRNL4BIT)
#define MDIRL1NDX      (MDIRL2NDX + MDIRNL2BIT)
#define MDIRL2NDX      (MDIRL3NDX + MDIRNL3BIT)
#define MDIRL3NDX      (MDIRL4NDX + MDIRNL4BIT)
#define MDIRL4NDX      MALLOCMINLOG2
#else /* PTRBITS != 32 && !MALLOC4LEVELTAB */
#define MDIRNL1BIT     12
#define MDIRNL2BIT     16
#if (MALLOCSMALLADR)
#define MDIRNL3BIT     (ADRBITS - MDIRNL1BIT - MDIRNL2BIT - MALLOCMINLOG2)
#else
#define MDIRNL3BIT     (PTRBITS - MDIRNL1BIT - MDIRNL2BIT - MALLOCMINLOG2)
#endif
#define MDIRNL1KEY     (1UL << MDIRNL1BIT)
#define MDIRNL2KEY     (1UL << MDIRNL2BIT)
#define MDIRNL3KEY     (1UL << MDIRNL3BIT)
#define MDIRL1NDX      (MDIRL2NDX + MDIRNL2BIT)
#define MDIRL2NDX      (MDIRL3NDX + MDIRNL3BIT)
#define MDIRL3NDX      MALLOCMINLOG2
#endif

#if (MALLOCSIG)
void
mallquit(int sig)
{
    fprintf(stderr, "QUIT (%d)\n", sig);
#if (MALLOCSTAT)
    mallocstat();
#else
    fflush(stderr);
#endif
    
    exit(sig);
}
#endif

#if (MALLOCDIAG)
void
mallocdiag(void)
{
#if (!MALLOCTLSARN)
    long        arnid;
#endif
    long        bktid;
    long        cnt;
    struct arn *arn;
    struct mag *mag;

    for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
        __malloclkmtx(&g_malloc.freelktab[bktid]);
        mag = g_malloc.freetab[bktid];
        cnt = 0;
        while (mag) {
            if (mag->prev) {
                if (mag->prev->next != mag) {
                    fprintf(stderr, "mag->next INVALID on free list\n");
                    magprint(mag);

                    exit(1);
                }
            }
            if (mag->next) {
                if (mag->next->prev != mag) {
                    fprintf(stderr, "mag->prev INVALID on free list\n");
                    magprint(mag);

                    exit(1);
                }
            }
            if (mag->cur) {
                fprintf(stderr, "mag->cur NON-ZERO on free list\n");
                magprint(mag);

                exit(1);
            }
            cnt++;
        }
        __mallocunlkmtx(&g_malloc.freelktab[bktid]);
    }
#if (!MALLOCTLSARN)
    for (arnid = 0 ; arnid < g_malloc.narn ; arnid++) {
        arn = g_malloc.arntab[arnid];
        if (arn) {
            for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
#if (!MALLOCTLSARN)
                __malloclkmtx(&arn->magbkt[bktid].lk);
#endif
                mag = arn->magbkt[bktid].ptr;
#else
#if (!MALLOCTLSARN)
                __mallocunlkmtx(&arn->maglktab[bktid]);
#endif
                mag = arn->magbkt[bktid].ptr;
#endif
                cnt = 0;
                while (mag) {
                    if (mag->prev) {
                        if (mag->prev->next != mag) {
                            fprintf(stderr, "mag->next INVALID on partial list\n");
                            magprint(mag);
                            
                            exit(1);
                        }
                    } else if (cnt) {
                        fprintf(stderr,
                                "mag->prev INCORRECT on partial list\n");
                        magprint(mag);
                        
                        exit(1);
                    }
                    if (mag->next) {
                        if (mag->next->prev != mag) {
                            fprintf(stderr, "mag->prev INVALID on partial list\n");
                            magprint(mag);
                            
                            exit(1);
                        }
                    }
                    if (!mag->cur) {
                        fprintf(stderr, "mag->cur == 0 on partial list\n");
                        magprint(mag);
                        
                        exit(1);
                    }
                    if (mag->cur >= mag->lim) {
                        fprintf(stderr, "mag->cur >= mag->lim on partial list\n");
                        magprint(mag);
                        
                        exit(1);
                    }
                    if (mag->next) {
                        if (mag->next->prev != mag) {
                            fprintf(stderr, "mag->prev != mag on partial list\n");
                            magprint(mag);
                            
                            exit(1);
                        }
                    }
                    if (mag->cur >= mag->lim) {
                        fprintf(stderr, "mag->cur >= mag->lim on partial list\n");
                        magprint(mag);

                        exit(1);
                    }
                    mag = mag->next;
                    cnt++;
                }
#if (!MALLOCTLSARN)
                __mallocunlkmtx(&arn->maglktab[bktid]);
#endif
            }
        }
    }
    
    return;
}
#endif /* MALLOCDIAG */

static __inline__ long
blkbktid(size_t size)
{
    unsigned long bktid = PTRBITS;
    unsigned long nlz;
    
    nlz = lzerol(size);
    bktid -= nlz;
    if (powerof2(size)) {
        bktid--;
    }
    
    return bktid;
}

static struct mag *
maggethdr(long bktid)
{
#if (MALLOCTLSARN)
    struct arn *arn = &thrarn;
#endif
    struct mag *mag;
    void       *ret;
    struct mag *hdr;
    struct mag *tail;
    struct mag *src = NULL;
    long        lim = magnblk(bktid);
    size_t      sz;
    uint8_t    *ptr;

    __malloclkmtx(&g_malloc.hdrbuf[bktid].lk);
    mag = g_malloc.hdrbuf[bktid].ptr;
    if (mag) {
        if (mag->next) {
            mag->next->prev = NULL;
        }
        g_malloc.hdrbuf[bktid].ptr = mag->next;
    }
    __mallocunlkmtx(&g_malloc.hdrbuf[bktid].lk);
    if (!mag) {
        if (magembedtab(bktid)) {
            lim = MALLOCNBUFHDR;
            sz = MALLOCHDRSIZE;
        } else {
            lim = MALLOCHDRSIZE / maghdrsz();
            sz = maghdrsz();
        }
        ret = mapanon(g_malloc.zerofd, rounduppow2(lim * sz, PAGESIZE));
        if (ret == MAP_FAILED) {
            
            return NULL;
        }
        mag = ret;
        ptr = ret;
        mag->adr = mag;
        mag->bktid = bktid;
        hdr = mag;
        tail = NULL;
        while (--lim) {
            ptr += sz;
            hdr = (struct mag *)ptr;
            hdr->adr = hdr;
            hdr->bktid = bktid;
            hdr->prev = tail;
            if (tail) {
                tail->next = hdr;
            } else {
                mag->next = hdr;
                hdr->prev = mag;
            }
            tail = hdr;
        }
        hdr = mag->next;
        if (hdr) {
            hdr->prev = NULL;
            __malloclkmtx(&g_malloc.hdrbuf[bktid].lk);
            tail->next = g_malloc.hdrbuf[bktid].ptr;
            if (tail->next) {
                tail->next->prev = tail;
            }
            g_malloc.hdrbuf[bktid].ptr = hdr;
            __mallocunlkmtx(&g_malloc.hdrbuf[bktid].lk);
        }
    }
    if (mag) {
        mag->prev = NULL;
        mag->next = NULL;
    }
    
    return mag;
}

static void *
maginitslab(struct mag *mag, long bktid, void *base)
{
    void       **stk;
    void        *ptrval;
    uint8_t     *ptr;
    struct mag  *hdr;
    size_t       lim = magnblk(bktid);
    size_t       sz = 1UL << bktid;
    long         ndx;

    if (!mag->stk) {
        maginittab(mag, bktid, base);
    }
    if (!mag->base) {
        if (!base) {
            ptrval = SBRK_FAILED;
#if (!MALLOCNOSBRK)
            if (bktid <= MALLOCSLABLOG2) {
                /* try to allocate slab from heap */
                __malloclkmtx(&g_malloc.heaplk);
                ptrval = growheap(rounduppow2(lim * sz, PAGESIZE));
                __mallocunlkmtx(&g_malloc.heaplk);
            }
#endif
            if (ptrval == SBRK_FAILED) {
                /* try to map slab */
                ptrval = mapanon(g_malloc.zerofd,
                                 rounduppow2(lim * sz, PAGESIZE));
                if (ptrval == MAP_FAILED) {
                    
                    return NULL;
                }
            }
        } else {
            ptrval = base;
        }
        ptr = ptrval;
        mag->base = ptr;
        mag->adr = ptr;
        mag->cur = 0;
        mag->lim = lim;
        mag->bktid = bktid;
        stk = mag->stk;
        ptr = ptrval;
        if (ptr) {
            ndx = 0;
            /* initialise allocation stack */
            while (lim--) {
#if (MALLOCHDRPREFIX)
                stk[ndx] = ptr;
#elif (MALLOCSTKNDX)
                stk[ndx] = (MAGPTRNDX)ndx;
#elif (MALLOCHACKS)
                stk[ndx] = (uintptr_t)ptr;
#else
                stk[ndx] = ptr;
#endif
                ptr += sz;
                ndx++;
            }
        }
    }

    return mag;
}

static void *
maginittab(struct mag *mag, long bktid, void *base)
{
    void        *ret;
    void       **stk;
    void        *ptrval;
    uint8_t     *ptr;
    long         lim = 1UL << bktid;
    long         n = magnblk(bktid);
    size_t       sz;
    struct mag  *hdr;
    struct mag  *head;
    struct mag  *last;
    struct mag  *tail;

    if (magembedtab(bktid)) {
        mag->stk = (uint8_t *)clradr(mag->adr) + maghdrsz();
        mag->ptrtab = &mag->stk[n];
#if (MALLOCFREEMAP)
        mag->freemap = &mag->stk[n << 1];
#endif
    } else {
        sz = magnbytetab(bktid);
        ret = mapanon(g_malloc.zerofd, rounduppow2(sz, PAGESIZE));
        if (ret == MAP_FAILED) {
            
            return NULL;
        }
        mag->stk = ret;
        mag->ptrtab = &mag->stk[n];
#if (MALLOCFREEMAP)
        mag->freemap = &mag->stk[n << 1];
#endif
    }

    return mag;
 }
    
static void *
maginit(struct mag *mag, long bktid, void *base)
{
    if (!mag->stk) {
        maginittab(mag, bktid, base);
    }
#if (!MALLOCTLSARN)
    mag->arnid = arnid;
#endif
    if ((base) || !mag->base) {
        if (!maginitslab(mag, bktid, base)) {
            
            return NULL;
        }
    }

    return mag;
}

static void
magputhdr(struct mag *mag)
{
    long bktid = mag->bktid;

/* add magazine header to header cache */
    if (mag) {
        mag->prev = NULL;
        mag->adr = mag;
        __malloclkmtx(&g_malloc.hdrbuf[bktid].lk);
        mag->next =  g_malloc.hdrbuf[bktid].ptr;
        if (mag->next) {
            mag->next->prev = mag;
        }
        g_malloc.hdrbuf[bktid].ptr = mag;
        __mallocunlkmtx(&g_malloc.hdrbuf[bktid].lk);
    }
    
    return;
}
    
#if (!MALLOCHDRPREFIX)
    
static struct mag *
findmag(void *ptr)
{
    uintptr_t      l1 = mdirl1ndx(ptr);
    uintptr_t      l2 = mdirl2ndx(ptr);
    uintptr_t      l3 = mdirl3ndx(ptr);
    uintptr_t      l4 = mdirl4ndx(ptr);
#if (MALLOCFREEMDIR)
    struct memtab *mptr1;
    struct memtab *mptr2;
#else
    void          *ptr1;
    void          *ptr2;
#endif
    struct mag    *mag = NULL;

    __malloclkmtx(&g_malloc.mlktab[l1]);
#if (MALLOCFREEMDIR)
    mptr1 = g_malloc.mdir[l1];
    if (mptr1) {
        mptr2 = mptr1->ptr;
        if (mptr2) {
            mptr1 = &mptr2[l2];
            if (mptr1) {
                mptr2 = mptr1->ptr;
                if (mptr2) {
                    mptr1 = &mptr2[l3];
                    if (mptr1) {
                        mptr2 = mptr1->ptr;
                        if (mptr2) {
                            mag = ((void **)mptr2)[l4];
                        }
                    }
                }
            }
        }
    }
#else
    ptr1 = g_malloc.mdir[l1];
    ptr2 = ((void **)ptr1)[l2];
    if (ptr2) {
        ptr1 = ((void **)ptr2)[l3];
        if (ptr1) {
            mag = ((void **)ptr1)[l4];
        }
    }
#endif
    __mallocunlkmtx(&g_malloc.mlktab[l1]);

    return mag;
}

static void
setmag(void *ptr,
       struct mag *mag)
{
    uintptr_t       l1 = mdirl1ndx(ptr);
    uintptr_t       l2 = mdirl2ndx(ptr);
    uintptr_t       l3 = mdirl3ndx(ptr);
    uintptr_t       l4 = mdirl4ndx(ptr);
#if (MALLOCFREEMDIR)
    long            empty;
    long            fail = 0;
    struct memtab  *mptr1;
    struct memtab  *mptr2;
    struct memtab  *ptab[3] = { NULL, NULL, NULL };
//    struct memtab   *ptab[3] = { NULL, NULL, NULL };
#else
    void           *ptr1;
    void           *ptr2;
    void          **pptr;
#endif

    __malloclkmtx(&g_malloc.mlktab[l1]);
#if (MALLOCFREEMDIR)
    if (!mag) {
        mptr1 = g_malloc.mdir[l1];
        if (mptr1) {
            mptr2 = mptr1->ptr;
            if (mptr2) {
                mptr1 = ((void **)mptr2)[l2];
                if (mptr1) {
                    ptab[0] = mptr2;
                    mptr2 = mptr1->ptr;
                    if (mptr2) {
                        mptr1 = ((void **)mptr2)[l3];
                        if (mptr1) {
                            ptab[1] = mptr2;
                            mptr2 = mptr1->ptr;
                            if (mptr2) {
                                mptr1 = ((void **)mptr2)[l4];
                                if (mptr1) {
                                    ptab[2] = mptr2;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
#else
    ptr1 = g_malloc.mdir[l1];
    if (ptr1) {
        ptr2 = ((void **)ptr1)[l2];
        if (ptr2) {
            ptr1 = ((void **)ptr2)[l3];
            if (ptr1) {
                *((void **)ptr1)[l4] = mag;
            }
        }
    }
#endif
#if (MALLOCFREEMDIR)
    if (mag) {
        mptr1 = g_malloc.mdir[l1];
        if (!mptr1) {
            empty = 1;
            mptr1 = mapanon(g_malloc.zerofd,
                            MDIRNL2KEY * sizeof(struct memtab));
            if (mptr1 != MAP_FAILED) {
                ptab[0] = mptr1;
                g_malloc.mdir[l1] = mptr1;
                VALGRINDALLOC(mptr1, MDIRNL2KEY * sizeof(struct memtab), 0);
            }
        }
        if (mptr1) {
            ptab[0] = mptr1;
            if (mptr1) {
                mptr2 = mptr1->ptr;
                if (!mptr2) {
                    mptr2 = mapanon(g_malloc.zerofd,
                                    MDIRNL3KEY * sizeof(struct memtab));
                    if (mptr2 != MAP_FAILED) {
                        ptab[1] = mptr2;
                        mptr1->ptr = mptr2;
                        mptr1->nref++;
                        VALGRINDALLOC(mptr2,
                                      MDIRNL3KEY * sizeof(struct memtab), 0);
                    } else {
                        fail = 1;
                    }
                }
            }
        }
        if (!fail) {
            mptr1 = mptr2->ptr;
            if (!mptr1) {
                mptr1 = mapanon(g_malloc.zerofd,
                                MDIRNL4KEY * sizeof(void *));
                if (mptr1 != MAP_FAILED) {
                    ptab[2] = mptr1;
                    mptr2->ptr = mptr1;
                    mptr2->nref++;
                    VALGRINDALLOC(mptr2, MDIRNL4KEY * sizeof(void *), 0);
                } else {
                    fail = 1;
                }
            }
        }
        if (!mag || (fail)) {
            mptr1 = ptab[2];
            mptr2 = ptab[1];
            if (mptr1) {
                if (!--mptr2->nref) {
                    unmapanon(mptr1, MDIRNL4KEY * sizeof(void *));
                    VALGRINDFREE(mptr1);
                }
            }
            mptr1 = ptab[0];
            if (mptr1) {
                if (!--mptr1->nref) {
                    unmapanon(mptr2, MDIRNL3KEY * sizeof(struct memtab));
                    VALGRINDFREE(mptr2);
                }
            }
            mptr2 = ptab[0];
            if (mptr2) {
                if (empty) {
                    unmapanon(mptr1, MDIRNL2KEY * sizeof(struct memtab));
                    VALGRINDFREE(mptr1);
                }
            }
        }
    }
#endif
    __mallocunlkmtx(&g_malloc.mlktab[l1]);
    
    return;
}

#endif /* !MALLOCHDRPREFIX */

static void
prefork(void)
{
#if (!MALLOCTLSARN)
    struct arn *arn;
#endif
    long        ndx;

    __malloclkmtx(&g_malloc.initlk);
#if (!MALLOCTLSARN)
    __malloclkmtx(&_arnlk);
#endif
    __malloclkmtx(&g_malloc.heaplk);
#if (!MALLOCTLSARN)
    for (ndx = 0 ; ndx < g_malloc.narn ; ndx++) {
        arn = g_malloc.arntab[ndx];
        __malloclkmtx(&arn->nreflk);
        for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
            __malloclkmtx(&arn->magbkt[ndx].lk);
        }
    }
#endif
    for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
        __malloclkmtx(&g_malloc.hdrbuf[ndx].lk);
        __malloclkmtx(&g_malloc.magbkt[ndx].lk);
        __malloclkmtx(&g_malloc.freetab[ndx].lk);
    }
    for (ndx = 0 ; ndx < MDIRNL1KEY ; ndx++) {
        __malloclkmtx(&g_malloc.mlktab[ndx]);
    }
    
    return;
}

static void
postfork(void)
{
#if (!MALLOCTLSARN)
    struct arn *arn;
#endif
    long        ndx;

    for (ndx = 0 ; ndx < MDIRNL1KEY ; ndx++) {
        __mallocunlkmtx(&g_malloc.mlktab[ndx]);
    }
    for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
        __mallocunlkmtx(&g_malloc.freetab[ndx].lk);
        __mallocunlkmtx(&g_malloc.magbkt[ndx].lk);
        __mallocunlkmtx(&g_malloc.hdrbuf[ndx].lk);
    }
#if (!MALLOCTLSARN)
    for (ndx = 0 ; ndx < g_malloc.narn ; ndx++) {
        arn = g_malloc.arntab[ndx];
        __mallocunlkmtx(&arn->nreflk);
    }
#endif
#if (!MALLOCDYNARN) && (!MALLOCTLSARN)
    for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
//        __mallocunlkmtx(&arn->hdrbuf[ndx].lk);
        __mallocunlkmtx(&arn->magbkt[ndx].lk);
    }
#endif
    __mallocunlkmtx(&g_malloc.heaplk);
#if (!MALLOCTLSARN)
    __mallocunlkmtx(&_arnlk);
#endif
    __mallocunlkmtx(&g_malloc.initlk);
    
    return;
}

#if defined(GNUMALLOC) && (GNUMALLOC)
static void   gnu_malloc_init(void);
static void * gnu_malloc_hook(size_t size, const void *caller);
static void * gnu_realloc_hook(void *ptr, size_t size, const void *caller);
static void * gnu_memalign_hook(size_t align, size_t size);
static void   gnu_free_hook(void *ptr);
#endif /* defined(GNUMALLOC) */

#if (GNUMALLOC)
void
#else
static void
#endif
mallinit(void)
{
#if (!MALLOCTLSARN)
    long     narn;
    long     arnid;
#endif
    long     bktid;
#if (!MALLOCNOSBRK)
    void    *heap;
    long     ofs;
#endif
    uint8_t *ptr;
#if (MALLOCNEWHACKS) && 0
    long     bkt;
#endif

    __malloclkmtx(&g_malloc.initlk);
    if (g_malloc.flags & MALLOCINIT) { 
        __mallocunlkmtx(&g_malloc.initlk);
       
        return;
    }
#if defined(GNUMALLOC) && (GNUMALLOC)
    __malloc_hook = gnu_malloc_hook;
    __realloc_hook = gnu_realloc_hook;
    __memalign_hook = gnu_memalign_hook;
    __free_hook = gnu_free_hook;
#endif
#if (MALLOCNEWHACKS) && 0
    for (bkt = 0 ; bkt < MALLOCNBKT ; bkt++) {
        setnbytelog2(bkt, _nbytelog2tab[bkt]);
    }
#endif
#if (MALLOCSIG)
    signal(SIGQUIT, mallquit);
    signal(SIGINT, mallquit);
    signal(SIGSEGV, mallquit);
    signal(SIGABRT, mallquit);
#endif
#if (MALLOCSTAT)
    atexit(mallocstat);
#endif
#if (MMAP_DEV_ZERO)
    g_malloc.zerofd = open("/dev/zero", O_RDWR);
#endif
#if (MALLOCGETNPROCS) && 0
    narn = 2 * get_nprocs_conf();
#elif (!MALLOCTLSARN)
    narn = MALLOCNARN;
#endif
#if (!MALLOCTLSARN)
    g_malloc.arntab = mapanon(g_malloc.zerofd,
                              narn * sizeof(struct arn **));
    g_malloc.narn = narn;
    ptr = mapanon(g_malloc.zerofd, narn * MALLOCARNSIZE);
    if (ptr == MAP_FAILED) {
        errno = ENOMEM;

        exit(1);
    }
    arnid = narn;
    while (arnid--) {
        g_malloc.arntab[arnid] = (struct arn *)ptr;
        ptr += MALLOCARNSIZE;
    }
    arnid = narn;
    while (arnid--) {
        for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
            __mallocinitmtx(&g_malloc.arntab[arnid]->nreflk);
            __mallocinitmtx(&g_malloc.arntab[arnid]->magbkt[bktid].lk);
            __mallocinitmtx(&g_malloc.arntab[arnid]->hdrbuf[bktid].lk);
        }
    }
#endif /* !MALLOCTLSARN */
    bktid = MALLOCNBKT;
    while (bktid--) {
        __mallocinitmtx(&g_malloc.freetab[bktid].lk);
        __mallocinitmtx(&g_malloc.magbkt[bktid].lk);
    }
#if (!MALLOCNOSBRK)
    __malloclkmtx(&g_malloc.heaplk);
    heap = growheap(0);
    ofs = (1UL << PAGESIZELOG2) - ((long)heap & (PAGESIZE - 1));
    if (ofs != PAGESIZE) {
        growheap(ofs);
    }
    __mallocunlkmtx(&g_malloc.heaplk);
#endif /* !MALLOCNOSBRK */
    g_malloc.mlktab = mapanon(g_malloc.zerofd, MDIRNL1KEY * sizeof(long));
#if (MALLOCFREEMDIR)
    g_malloc.mdir = mapanon(g_malloc.zerofd,
                            MDIRNL1KEY * sizeof(struct memtab));
#else
    g_malloc.mdir = mapanon(g_malloc.zerofd,
                            MDIRNL1KEY * sizeof(void *));
#endif
#if (MALLOCSTAT)
    ntabbyte += MDIRNL1KEY * sizeof(long);
    ntabbyte += MDIRNL1KEY * sizeof(void *);
#endif
#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zmalloc_initialize_hook) {
        __zmalloc_initialize_hook();
    }
#endif
    pthread_atfork(prefork, postfork, postfork);
    g_malloc.flags |= MALLOCINIT;
    __mallocunlkmtx(&g_malloc.initlk);
    
    return;
}

/*
 * M_MMAP_MAX       - maximum # of allocation requests serviced simultaneously
 * M_MMAP_THRESHOLD - minimum size for mmap()
 */
int
mallopt(int parm, int val)
{
    int  ret = 0;
    long num;
    
    switch (parm) {
        case M_MXFAST:
            fprintf(stderr, "mallopt: M_MXFAST not supported\n");

            break;
        case M_NLBLKS:
            fprintf(stderr, "mallopt: M_NLBLKS not supported\n");

            break;
        case M_GRAIN:
            fprintf(stderr, "mallopt: M_GRAIN not supported\n");

            break;
        case M_KEEP:
            fprintf(stderr, "mallopt: M_KEEP not supported\n");

            break;
        case M_TRIM_THRESHOLD:
            fprintf(stderr, "mallopt: M_TRIM_THRESHOLD not supported\n");

            break;
        case M_TOP_PAD:
            fprintf(stderr, "mallopt: M_TOP_PAD not supported\n");

            break;
        case M_MMAP_THRESHOLD:
            num = sizeof(long) - tzerol(val);
            if (powerof2(val)) {
                num++;
            }
            ret = 1;
            g_malloc.mallopt.mmaplog2 = num;

            break;
        case M_MMAP_MAX:
            g_malloc.mallopt.mmapmax = val;
            ret = 1;

            break;
        case M_CHECK_ACTION:
            g_malloc.mallopt.action |= val & 0x07;
            ret = 1;
            
            break;
        case M_PERTURB:
            g_malloc.mallopt.flg |= MALLOPT_PERTURB_BIT;
            g_malloc.mallopt.perturb = val;

            break;
        default:
            fprintf(stderr, "MALLOPT: invalid parm %d\n", parm);

            break;
    }

    return ret;
}

int
malloc_info(int opt, FILE *fp)
{
    int retval = -1;

    if (opt) {
        fprintf(fp, "malloc_info: opt-argument non-zero\n");
    }
    fprintf(fp, "malloc_info not implemented\n");
    
    return retval;
}

struct mallinfo
mallinfo(void)
{
    return g_malloc.mallinfo;
}

void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(1)))
__attribute__ ((alloc_align(2)))
__attribute__ ((assume_aligned(MALLOCMINSIZE)))
__attribute__ ((malloc))
#endif
_malloc(size_t size,
        size_t align,
        long zero)
{
    void          **stk;
    struct memtab  *tab = NULL;
    struct arn     *arn;
    struct mag     *mag;
    struct mag     *hdr;
    struct mag     *next;
    void           *adr;
    uint8_t        *ptr = NULL;
    uint8_t        *ptrval = NULL;
#if (!MALLOCTLSARN)
    long            arnid;
#endif
#if (MALLOCHDRPREFIX)
    size_t          sz = rounduppow2(size + MEMHDRSIZE,
                                     max(align, MALLOCALIGNMENT));
#else
    size_t          sz = 1UL << blkbktid(size + align);
#endif
    long            bktid = blkbktid(sz);
//    long         mapped = 0;
    long            lim;
    long            n;
    size_t          incr;
#if (MALLOCSTEALMAG)
    long            id;
    long            stolen = 0;
#endif
    int             val;
    
    if (!(g_malloc.flags & MALLOCINIT)) {
        mallinit();
    }
//    align = max(align, MALLOCALIGNMENT);
#if (MALLOCTLSARN)
    arn = &thrarn;
#endif
    /* try to allocate from a partially used magazine */
#if (!MALLOCTLSARN)
    __malloclkmtx(&arn->magbkt[bktid].lk);
#endif
    mag = arn->magbkt[bktid].ptr;
    if (mag) {
        mag->next = arn->magbkt[bktid].ptr;
        if (mag->next) {
            mag->next->prev = NULL;
        }
        mag->tab = NULL;
        arn->magbkt[bktid].ptr = mag->next;
    }
#if (!MALLOCTLSARN)
    __mallocunlkmtx(&arn->magbkt[bktid].lk);
#endif
    if (!mag) {
        __malloclkmtx(&g_malloc.magbkt[bktid].lk);
        mag = g_malloc.magbkt[bktid].ptr;
        if (mag) {
            mag->next = g_malloc.magbkt[bktid].ptr;
            if (mag->next) {
                mag->next->prev = NULL;
            }
            g_malloc.magbkt[bktid].ptr = mag->next;
            mag->tab = NULL;
        }
        __mallocunlkmtx(&g_malloc.magbkt[bktid].lk);
        if (!mag) {
            __malloclkmtx(&g_malloc.freetab[bktid].lk);
            mag = g_malloc.freetab[bktid].ptr;
            if (mag) {
                mag->next = g_malloc.freetab[bktid].ptr;
                if (mag->next) {
                    mag->next->prev = NULL;
                }
                g_malloc.freetab[bktid].ptr = mag->next;
                mag->tab = NULL;
            }
            __mallocunlkmtx(&g_malloc.freetab[bktid].lk);
        }
        if (!mag) {
            mag = maggethdr(bktid);
            if (mag) {
                if (!maginit(mag, bktid, NULL)) {
                    
                    return NULL;
                }
            }
        }
        stk = mag->stk;
        lim = mag->lim;
#if (MALLOCHDRPREFIX)
        ptrval = stk[mag->cur++];
#elif (MALLOCSTKNDX)
        ptrval = magndx2ptr(mag, stk[mag->cur]);
        mag->cur++;
#elif (MALLOCHACKS)
        ptrval = mag->stk[mag->cur++];
#else
        ptrval = mag->stk[mag->cur++];
#endif
        if (!ptrval) {
            assert(ptrval);
        }
        ptr = ptrval;
        if (mag->cur == lim || mag->cur == 1) {
            tab = mag->tab;
            if (tab) {
                mag->prev = NULL;
                __malloclkmtx(&tab->lk);
                if ((mag->prev) && (mag->next)) {
                    mag->next->prev = mag->prev;
                    mag->prev->next = mag->next;
                } else if (mag->prev) {
                    mag->prev->next = NULL;
                } else if (mag->next) {
                    mag->next->prev = NULL;
                    tab->ptr = mag->next;
                } else {
                    tab->ptr = NULL;
                }
                mag->tab = NULL;
                __mallocunlkmtx(&tab->lk);
            }
        }
        if (mag->cur == 1) {
            if (bktid <= MALLOCBIGSLABLOG2) {
                mag->prev = NULL;
                mag->next = arn->magbkt[bktid].ptr;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                arn->magbkt[bktid].ptr = mag;
            } else {
                mag->prev = NULL;
                __malloclkmtx(&g_malloc.magbkt[bktid].lk);
                mag->next = g_malloc.magbkt[bktid].ptr;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                g_malloc.magbkt[bktid].ptr = mag;
                __mallocunlkmtx(&g_malloc.magbkt[bktid].lk);
            }
        }
    }
    ptr = clrptr(ptrval);
    if (ptr) {
        VALGRINDPOOLALLOC(mag->base,
                          ptr,
                          size);
        if ((zero) && (((uintptr_t)ptrval & BLKDIRTY))) {
            memset(ptr, 0, 1UL << bktid);
        } else if (g_malloc.mallopt.flg & MALLOPT_PERTURB_BIT) {
            int perturb = g_malloc.mallopt.perturb;
            
            perturb = (~perturb) & 0xff;
            memset(ptr, perturb, 1UL << bktid);
        }
#if (MALLOCHDRPREFIX)
        /* store unaligned source pointer and mag address */
        adr = ptr;
        ptr += MEMHDRSIZE;
        ptr = (void *)rounduppow2((uintptr_t)(ptr),
                                  max(align, MALLOCALIGNMENT));
#else
        if ((uintptr_t)ptr & (align - 1)) {
            ptr = ptralign(ptr, align);
        }
#endif
        /* store unaligned source pointer */
#if (MALLOCHDRPREFIX) && (!MALLOCNEWHDR)
        setptr(mag, ptr, adr);
#else
        magputptr(mag, ptr, adr);
#endif
        /* add magazine to lookup structure using ptr as key */
        setmag(ptr, mag);
#if (MALLOCFREEMAP)
        if (bitset(mag->freemap, magptrid(mag, ptr))) {
            fprintf(stderr, "trying to reallocate block");
            fflush(stderr);
            
            abort();
        }
        setbit(mag->freemap, magptrid(mag, ptr));
#endif
    }
    
    return ptr;
}

void
_free(void *ptr)
{
    struct memtab *tab;
    struct arn    *arn;
    struct mag    *mag;
    void          *adr = NULL;
#if (!MALLOCTLSARN)
    long           arnid;
#endif
    long           lim;
    long           bktid;
    long           freemap = 0;
    int            val;

    if (!ptr) {

        return;
    }
#if (MALLOCTLSARN)
    arn = &thrarn;
#endif
#if (MALLOCHDRPREFIX)
    mag = getmag(ptr);
#else
    mag = findmag(ptr);
#endif
    if (mag) {
        VALGRINDPOOLFREE(mag->base,
                         ptr);
        /* remove pointer from allocation lookup structure */
#if (!MALLOCHDRPREFIX)
        setmag(ptr, NULL);
#endif
        bktid = mag->bktid;
#if (MALLOCFREEMAP)
        if (!bitset(mag->freemap, magptrid(mag, ptr))) {
            fprintf(stderr, "trying to free an unused block\n");
            
            abort();
        }
        clrbit(mag->freemap, magptrid(mag, ptr));
#endif
        if (g_malloc.mallopt.flg & MALLOPT_PERTURB_BIT) {
            int   perturb = g_malloc.mallopt.perturb;
#if (MALLOCHDRPREFIX) && (!MALLOCNEWHDR)
            void *vptr = getptr(ptr);
#else
            void *vptr = maggetptr(mag, ptr);
#endif
            
            perturb &= 0xff;
            memset(vptr, perturb, 1UL << bktid);
        }
//        arn = g_malloc.arntab[arnid];
        bktid = mag->bktid;
#if (MALLOCHDRPREFIX)
        mag->stk[--mag->cur] = maggetptr(mag, ptr);
#elif (MALLOCSTKNDX)
        mag->stk[--mag->cur] = magptr2ndx(mag, ptr);
#elif (MALLOCHACKS)
        mag->stk[--mag->cur] = (uintptr_t)ptr | BLKDIRTY;
#else
        mag->stk[--mag->cur] = (void *)((uintptr_t)ptr | BLKDIRTY);
#endif
        lim = mag->lim;
        if (!mag->cur) {
            tab = mag->tab;
            if (tab) {
                __malloclkmtx(&tab->lk);
                if ((mag->prev) && (mag->next)) {
                    mag->next->prev = mag->prev;
                    mag->prev->next = mag->next;
                } else if (mag->prev) {
                    mag->prev->next = NULL;
                } else if (mag->next) {
                    mag->next->prev = NULL;
                    tab->ptr = mag->next;
                } else {
                    tab->ptr = NULL;
                }
                __mallocunlkmtx(&tab->lk);
                mag->tab = NULL;
            }
            if ((uintptr_t)mag->base & MAGMAP) {
                freemap = 1;
            } else {
                mag->prev = NULL;
                __malloclkmtx(&g_malloc.freetab[bktid].lk);
                mag->next = g_malloc.freetab[bktid].ptr;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                mag->tab = &g_malloc.freetab[bktid];
                g_malloc.freetab[bktid].ptr = mag;
                __mallocunlkmtx(&g_malloc.freetab[bktid].lk);
            }
        } else if (mag->cur == lim - 1) {
            /* queue an unqueued earlier fully allocated magazine */
            mag->prev = NULL;
            if (bktid <= MALLOCBIGSLABLOG2) {
                mag->next = arn->magbkt[bktid].ptr;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                arn->magbkt[bktid].ptr = mag;
            } else {
                __malloclkmtx(&g_malloc.magbkt[bktid].lk);
                mag->next = g_malloc.magbkt[bktid].ptr;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                mag->tab = &g_malloc.magbkt[bktid];
                g_malloc.magbkt[bktid].ptr = mag;
                __mallocunlkmtx(&g_malloc.magbkt[bktid].lk);
            }
        }
        if (freemap) {
            /* unmap slab */
            adr = (void *)mag->base;
            VALGRINDRMPOOL(adr);
            unmapanon(adr, magnbyte(bktid));
#if (MALLOCSTAT)
            nmapbyte -= magnbyte(bktid);
#endif
            mag->base = NULL;
            mag->adr = mag;
            magputhdr(mag);
        }
#if (MALLOCSTAT)
        mallocstat();
#endif
    }
    
    return;
}

/* Internal function for realloc() and reallocf() */
void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MALLOCMINSIZE)))
#endif
_realloc(void *ptr,
         size_t size,
         long rel)
{
    uint8_t       *retptr = NULL;
#if (MALLOCHDRPREFIX)
    size_t         sz = rounduppow2(size + MEMHDRSIZE, MALLOCALIGNMENT);
#else
    size_t         sz = 1UL << blkbktid(size);
#endif
#if (MALLOCHDRPREFIX)
    struct mag    *mag = (ptr) ? getmag(ptr) : NULL;
#else
    struct mag    *mag = (ptr) ? findmag(ptr) : NULL;
#endif
    long           bktid = blkbktid(sz);

    if (!(g_malloc.flags & MALLOCINIT)) {
        mallinit();
    }
    if (!ptr) {
        retptr = _malloc(sz, 0, 0);
    } else if ((mag) && mag->bktid < bktid) {
        retptr = _malloc(sz, 0, 0);
        if (retptr) {
            memcpy(retptr, ptr, sz);
            _free(ptr);
            ptr = NULL;
        }
    } else {
        retptr = ptr;
    }
    ptr = retptr;
#if (MALLOCHDRPREFIX)
    retptr = (void *)rounduppow2((uintptr_t)retptr + MALLOCALIGNMENT,
                                 MALLOCALIGNMENT);
#endif
    if (((rel) && (ptr)) || (retptr != ptr)) {
        _free(ptr);
    }
#if (MALLOCDEBUG)
    assert(retptr != NULL);
#endif
    if (!retptr) {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
#if (MALLOCSTAT)
        mallocstat();
#endif
    }

    return retptr;
}

/* API FUNCTIONS */

void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(MALLOCMINSIZE)))
__attribute__ ((malloc))
#endif
#if defined(GNUMALLOC) && (GNUMALLOC)
zero_malloc(size_t size)
#else
malloc(size_t size)
#endif
{
    void *ptr = NULL;

    if (!size) {
#if defined(_GNU_SOURCE)
        ptr = _malloc(MALLOCMINSIZE, 0, 0);
#endif
        
        return ptr;
    }
#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zmalloc_hook) {
        void *caller = NULL;

        m_getretadr(caller);
        ptr = __zmalloc_hook(size, (const void *)caller);

        return ptr;
    }
#endif
    ptr = _malloc(size, 0, 0);
    if (!ptr) {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif

        return NULL;
    }

    return ptr;
}

void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(1, 2)))
__attribute__ ((assume_aligned(MALLOCMINSIZE)))
__attribute__ ((malloc))
#endif
#if defined(GNUMALLOC) && (GNUMALLOC)
zero_calloc(size_t n, size_t size)
#else
calloc(size_t n, size_t size)
#endif
{
#if (MALLOCHDRPREFIX)
    size_t sz = n * rounduppow2(size + MEMHDRSIZE,
                                MALLOCALIGNMENT);
#else
    size_t sz = max(n * size, MALLOCMINSIZE);
#endif
    void *ptr = NULL;

    if (!n || !size) {
#if defined(_GNU_SOURCE)
        ptr = _malloc(MALLOCMINSIZE, 0, 1);
#endif
        
        return ptr;
    }
#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zmalloc_hook) {
        void *caller = NULL;

        m_getretadr(caller);
        ptr = __zmalloc_hook(size, (const void *)caller);

        return ptr;
    }
#endif
    ptr = _malloc(sz, 0, 1);
    if (ptr) {
//        VALGRINDALLOC(ptr, sz, 1);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif

    return ptr;
}

void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MALLOCMINSIZE)))
#endif
#if defined(GNUMALLOC) && (GNUMALLOC)
zero_realloc(void *ptr,
             size_t size)
#else
realloc(void *ptr,
        size_t size)
#endif
{
    void *retptr = NULL;

#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zrealloc_hook) {
        void *caller = NULL;

        m_getretadr(caller);
        ptr = __zrealloc_hook(ptr, size, (const void *)caller);

        return ptr;
    }
#endif
    if (!size && (ptr)) {
        _free(ptr);
//        VALGRINDFREELIKE(ptr);
    } else {
        retptr = _realloc(ptr, size, 0);
        if (retptr) {
            if (retptr != ptr) {
                _free(ptr);
//                VALGRINDFREELIKE(ptr);
            }
//            VALGRINDALLOC(retptr, size, 0);
        }
    }
#if (MALLOCDEBUG)
    assert(retptr != NULL);
#endif

    return retptr;
}

void
#if defined(GNUMALLOC) && (GNUMALLOC)
zero_free(void *ptr)
#else
free(void *ptr)
#endif
{
#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zfree_hook) {
        void *caller = NULL;

        m_getretadr(caller);
        __zfree_hook(ptr, (const void *)caller);

        return;
    }
#endif
    if (ptr) {
        _free(ptr);
//        VALGRINDFREELIKE(ptr);
    }

    return;
}

#if defined(__ISOC11_SOURCE) && (_ISOC11_SOURCE)
void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((alloc_align(1)))
__attribute__ ((assume_aligned(MALLOCMINSIZE)))
__attribute__ ((malloc))
#endif
aligned_alloc(size_t align,
              size_t size)
{
    void   *ptr = NULL;
    size_t  aln = max(align, MALLOCMINSIZE);

#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zmemalign_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmemalign_hook(align, size, (const void *)caller);

        return ptr
    }
#endif
    if (!powerof2(aln) || (size & (aln - 1))) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, aln, 0);
    }
    if (ptr) {
//        VALGRINDALLOC(ptr, size, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif

    return ptr;
}
#endif

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)

int
#if defined(__GNUC__)
__attribute__ ((alloc_size(3)))
__attribute__ ((alloc_align(2)))
#endif
posix_memalign(void **ret,
               size_t align,
               size_t size)
{
    void   *ptr = NULL;
    size_t  aln = max(align, MALLOCMINSIZE);
    int     retval = -1;

#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zmemalign_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmemalign_hook(align, size, (const void *)caller);

        return ptr
    }
#endif
    if (!powerof2(align) || (align & (sizeof(void *) - 1))) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, aln, 0);
        if (ptr) {
            retval = 0;
        }
    }
    if (ptr) {
//        VALGRINDALLOC(ptr, size, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif
    *ret = ptr;

    return retval;
}
#endif

/* STD: UNIX */

#if (defined(_BSD_SOURCE)                                                      \
     || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500                 \
         || (defined(_XOPEN_SOURCE) && defined(_XOPEN_SOURCE_EXTENDED))) \
     && !((defined(_POSIX_SOURCE) && _POSIX_C_SOURCE >= 200112L)        \
          || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 600)))
void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(PAGESIZE)))
__attribute__ ((malloc))
#endif
valloc(size_t size)
{
    void *ptr;

#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zmemalign_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmemalign_hook(PAGESIZE, size, (const void *)caller);

        return ptr
    }
#endif
    ptr = _malloc(size, PAGESIZE, 0);
    if (ptr) {
//        VALGRINDALLOC(ptr, size, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif
    
    return ptr;
}
#endif

void *
#if defined(__GNUC__)
__attribute__ ((alloc_align(1)))
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MALLOCMINSIZE)))
__attribute__ ((malloc))
#endif
#if defined(GNUMALLOC) && (GNUMALLOC)
zero_memalign(size_t align,
              size_t size)
#else
memalign(size_t align,
         size_t size)
#endif
{
    void   *ptr = NULL;
    size_t  aln = max(align, MALLOCMINSIZE);

#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zmemalign_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmemalign_hook(align, size, (const void *)caller);

        return ptr;
    }
#endif
    if (!powerof2(align)) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, aln, 0);
    }
    if (ptr) {
//        VALGRINDALLOC(ptr, size, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif

    return ptr;
}

#if defined(_BSD_SOURCE)
void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MALLOCMINSIZE)))
#endif
reallocf(void *ptr,
         size_t size)
{
    void *retptr;

#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zrealloc_hook) {
        void *caller = NULL;

        m_getretadr(caller);
        ptr = __zrealloc_hook(ptr, size, (const void *)caller);

        return ptr;
    }
#endif
    if (ptr) {
        retptr = _realloc(ptr, size, 1);
    } else if (size) {
        retptr = _malloc(size, 0, 0);
    } else {

        return NULL;
    }
    if (ptr) {
//        VALGRINDFREELIKE(ptr);
    }
    if (retptr) {
//        VALGRINDALLOC(retptr, size, 0);
    }
#if (MALLOCDEBUG)
    assert(retptr != NULL);
#endif

    return retptr;
}
#endif

#if defined(_GNU_SOURCE)
void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(PAGESIZE)))
__attribute__ ((malloc))
#endif
pvalloc(size_t size)
{
#if (MALLOCHDRPREFIX)
    size_t  sz = size + MEMHDRSIZE;
#else
    size_t  sz = size;
#endif
    void   *ptr = _malloc(sz, PAGESIZE, 0);

#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zmemalign_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmemalign_hook(PAGESIZE, size, (const void *)caller);

        return ptr;
    }
#endif
    if (ptr) {
//        VALGRINDALLOC(ptr, size, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif

    return ptr;
}
#endif

#if defined(_MSVC_SOURCE)

void *
#if defined(__GNUC__)
__attribute__ ((alloc_align(2)))
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(MALLOCMINSIZE)))
__attribute__ ((malloc))
#endif
_aligned_malloc(size_t size,
                size_t align)
{
    void   *ptr = NULL;
    size_t  aln = max(align, MALLOCMINSIZE);

#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zmemalign_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmemalign_hook(align, size, (const void *)caller);

        return ptr;
    }
#endif
    if (!powerof2(align)) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, aln, 0);
    }
    if (ptr) {
//        VALGRINDALLOC(ptr, size, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif

    return ptr;
}

void
_aligned_free(void *ptr)
{
#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zfree_hook) {
        void *caller = NULL;

        m_getretadr(caller);
        __zfree_hook(ptr, (const void *)caller);

        return;
    }
#endif
    if (ptr) {
        _free(ptr);
//        VALGRINDFREELIKE(ptr);
    }

    return;
}

#endif /* _MSVC_SOURCE */

#if defined(_INTEL_SOURCE)

void *
#if defined(__GNUC__)
__attribute__ ((alloc_align(2)))
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(MALLOCMINSIZE)))
__attribute__ ((malloc))
#endif
_mm_malloc(int size,
           int align)
{
    void   *ptr = NULL;
    size_t  aln = max((unsigned long)align, MALLOCMINSIZE);

#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zmemalign_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmemalign_hook(align, size, (const void *)caller);

        return ptr;
    }
#endif
    if (!powerof2(align)) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, aln, 0);
    }
    if (ptr) {
//        VALGRINDALLOC(ptr, size, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif

    return ptr;
}

void
_mm_free(void *ptr)
{
#if (ZMALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (ZMALLOCHOOKS)) && 0
    if (__zfree_hook) {
        void *caller = NULL;

        m_getretadr(caller);
        __zfree_hook(ptr, (const void *)caller);

        return;
    }
#endif
    if (ptr) {
        _free(ptr);
//        VALGRINDFREELIKE(ptr);
    }

    return;
}

#endif /* _INTEL_SOURCE */

void
cfree(void *ptr)
{
    if (ptr) {
        _free(ptr);
//        VALGRINDFREELIKE(ptr);
    }

    return;
}

size_t
malloc_usable_size(void *ptr)
{
#if (MALLOCHDRPREFIX)
    struct mag *mag = getmag(ptr);
#else
    struct mag *mag = findmag(ptr);
#endif
#if (MALLOCHDRPREFIX)
    size_t      sz = ((mag)
                      ? ((1UL << mag->bktid)
                         - (ptrdiff(ptr, maggetptr(mag, ptr))))
                      : 0);
#else
    size_t      sz = (mag) ? (1UL << mag->bktid) : 0;
#endif
    
    return sz;
}

size_t
malloc_good_size(size_t size)
{
    size_t sz = 1UL << blkbktid(size);

#if (MALLOCHDRPREFIX)
    sz -= MALLOCALIGNMENT;
#endif

    return sz;
}

size_t
malloc_size(void *ptr)
{
#if (MALLOCHDRPREFIX)
    struct mag *mag = getmag(ptr);
#else
    struct mag *mag = findmag(ptr);
#endif
    size_t      sz = (mag) ? (1UL << (mag)->bktid) : 0;
    
    return sz;
}

#if (GNUMALLOC)

static void
gnu_malloc_init(void)
{
    mallinit();
}

static void *
gnu_malloc_hook(size_t size, const void *caller)
{
    void *adr = zero_malloc(size);

    return adr;
}

static void *
gnu_realloc_hook(void *ptr, size_t size, const void *caller)
{
    void *adr = zero_realloc(ptr, size);

    return adr;
}

static void *
gnu_memalign_hook(size_t align, size_t size)
{
    void *adr = zero_memalign(align, size);

    return adr;
}

static void
gnu_free_hook(void *ptr)
{
    zero_free(ptr);

    return;
}

#endif /* GNUMALLOC */

