#ifndef ___MALLOC_H__
#define ___MALLOC_H__

#include <stdint.h>
#include <malloc.h>
#include <zero/param.h>

/* internal stuff for zero malloc - not for the faint at heart to modify :) */

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
#define MALLOCFREETABS    0 // use free block bitmaps; bit 1 for allocated
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
#define MALLOCMINSIZE     (1UL << MALLOCMINLOG2)
#define MALLOCNBKT        PTRBITS

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

#if defined(ZEROMTX) && (ZEROMTX)
#define MUTEX volatile long
#include <zero/mtx.h>
#include <zero/spin.h>
#if (ZEROMTX) && (DEBUGMTX)
#define __mallocinitmtx(mp)   mtxinit(mp)
#define __malloctrylkmtx(mp)  mtxtrylk(mp)
#define __malloclkmtx(mp)     (fprintf(stderr, "%p\tLK: %d\n", mp, __LINE__), \
                               mtxlk(mp))
#define __mallocunlkmtx(mp)   (fprintf(stderr, "%p\tUNLK: %d\n", mp, __LINE__), \
                               mtxunlk(mp))
#define __mallocinitspin(mp)  spininit(mp)
#define __malloclkspin(mp)    (fprintf(stderr, "LK: %d\n", __LINE__),   \
                               spinlk(mp))
#define __mallocunlkspin(mp)  (fprintf(stderr, "UNLK: %d\n", __LINE__), \
                               spinunlk(mp))
#elif (ZEROMTX)
#define __mallocinitmtx(mp)   mtxinit(mp)
#define __malloctrylkmtx(mp)  mtxtrylk(mp)
#define __malloclkmtx(mp)     mtxlk(mp)
#define __mallocunlkmtx(mp)   mtxunlk(mp)
#define __mallocinitspin(mp)  spininit(mp)
#define __malloctrylkspin(mp) spintrylk(mp)
#define __malloclkspin(mp)    spinlk(mp)
#define __mallocunlkspin(mp)  spinunlk(mp)
#endif
#elif (PTHREAD)
#include <pthread.h>
#define MUTEX pthread_mutex_t
#define __mallocinitmtx(mp) pthread_mutex_init(mp, NULL)
#define __malloclkmtx(mp)   pthread_mutex_lock(mp)
#define __mallocunlkmtx(mp) pthread_mutex_unlock(mp)
#endif

#if (MALLOCPTRNDX)
#include <stdint.h>
#if (MALLOCSLABLOG2 - MALLOCMINLOG2 < 8)
#define PTRFREE              0xff
#define MAGPTRNDX            uint8_t
#elif (MALLOCSLABLOG2 - MALLOCMINLOG2 < 16)
#define PTRFREE              0xffff
#define MAGPTRNDX            uint16_t
#elif (MALLOCSLABLOG2 - MALLOCMINLOG2 < 32)
#define PTRFREE              0xffffffff
#define MAGPTRNDX            uint32_t
#else
#define PTRFREE              UINT64_C(0xffffffffffffffff)
#define MAGPTRNDX            uint64_t
#endif
#endif

#if defined(MALLOCDEBUG)
#if (MALLOCTRACE) && (GNUTRACE)
#if 0
#include <execinfo.h>
extern uintptr_t _backtrace(void *buf, size_t size, long syms, int fd);
#define __malloctrace()                                                 \
    do {                                                                \
        _backtrace(tracetab, 64, 1, 1);                                 \
    } while (0)
#define __mallocprnttrace(str, sz, aln)                                 \
    do {                                                                \
        fprintf(stderr, "%s: %ull(%ull)\n",                             \
                str,                                                    \
                (unsigned long long)(sz),                               \
                (unsigned long long)(aln));                             \
    } while (0)
#endif
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
static void   gnu_malloc_init(void);
static void * gnu_malloc_hook(size_t size, const void *caller);
static void * gnu_realloc_hook(void *ptr, size_t size, const void *caller);
static void * gnu_memalign_hook(size_t align, size_t size);
static void   gnu_free_hook(void *ptr);
#endif /* defined(GNUMALLOC) */

#if (MALLOCMULTITAB)

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

#if (MALLOCSLABTAB)
#define SLABDIRNL1KEY     (1L << SLABDIRNL1BIT)
#define SLABDIRNL2KEY     (1L << SLABDIRNL2BIT)
#if defined(SLABDIRNL3BIT) && (SLABDIRNL3BIT)
#define SLABDIRNL3KEY     (1L << SLABDIRNL3BIT)
#endif
#endif
#define PAGEDIRNL1KEY     (1L << PAGEDIRNL1BIT)
#define PAGEDIRNL2KEY     (1L << PAGEDIRNL2BIT)
#if defined(PAGEDIRNL3BIT) && (PAGEDIRNL3BIT)
#define PAGEDIRNL3KEY     (1L << PAGEDIRNL3BIT)
#ENDIF

#if (MALLOCSLABTAB)
#define SLABDIRL1NDX      (SLABDIRL2NDX + SLABDIRNL2BIT)
#if defined(SLABDIRNL3BIT) && (SLABDIRNL3BIT)
#define SLABDIRL2NDX      (SLABDIRL3NDX + SLABDIRNL3BIT)
#define SLABDIRL3NDX      MALLOCSLABLOG2
#else
#define SLABDIRL2NDX      MALLOCSLABLOG2
#endif
#define PAGEDIRL1NDX      (PAGEDIRL2NDX + PAGEDIRNL2BIT)
#if defined(PAGEDIRL3BIT) && (PAGEDIRNL3BIT)
#define PAGEDIRL2NDX      (PAGEDIRL3NDX + PAGEDIRNL3BIT)
#else
#define PAGEDIRL2NDX      PAGESIZELOG2
#endif

#define slabdirl1ndx(ptr) (((uintptr_t)(ptr) >> SLABDIRL1NDX)           \
                           & ((1UL << SLABDIRNL1BIT) - 1))
#define slabdirl2ndx(ptr) (((uintptr_t)(ptr) >> SLABDIRL2NDX)           \
                           & ((1UL << SLABDIRNL2BIT) - 1))
#if defined(SLABDIRL3BIT) && (SLABDIRNL3BIT)
#define slabdirl3ndx(ptr) (((uintptr_t)(ptr) >> SLABDIRL3NDX)           \
                           & ((1UL << SLABDIRNL3BIT) - 1))
#endif

#define pagedirl1ndx(ptr) (((uintptr_t)(ptr) >> PAGEDIRL1NDX)           \
                           & ((1UL << PAGEDIRNL1BIT) - 1))
#define pagedirl2ndx(ptr) (((uintptr_t)(ptr) >> PAGEDIRL2NDX)           \
                           & ((1UL << PAGEDIRNL2BIT) - 1))
#if defined(PAGEDIRL3BIT) && (PAGEDIRNL3BIT)
#define pagedirl3ndx(ptr) (((uintptr_t)(ptr) >> PAGEDIRL3NDX)           \
                           & ((1UL << PAGEDIRNL3BIT) - 1))
#endif

#endif /* MALLOCMULTITAB */

struct memtab {
    void          *ptr;
    volatile long  nref;
};

struct magtab {
    MUTEX          lk;
    struct mag    *ptr;
#if (MALLOCBUFMAG)
    unsigned long  n;
    uint8_t        _pad[CLSIZE - sizeof(long) - sizeof(void *) - sizeof(MUTEX)];
#else
    uint8_t        _pad[CLSIZE - sizeof(MUTEX) - sizeof(void *)];
#endif
};

#define MALLOCARNSIZE rounduppow2(sizeof(struct arn), PAGESIZE)
/* arena structure */
struct arn {
    struct magtab magbkt[MALLOCNBKT];
#if (!MALLOCTLSARN)
    MUTEX         nreflk;
    long          nref;
#endif
};

#if 0
#define maglkbit(mag)   (!m_cmpsetbit((volatile long *)&mag->adr, 0))
#define magunlkbit(mag) (m_cmpclrbit((volatile long *)&mag->adr, 0))
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
    struct arn    *arn;
    void          *base;
    void          *adr;
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
    MAGPTRNDX     *stk;
    MAGPTRNDX     *ptr;
#elif (MALLOCHDRPREFIX)
    void          *stk;
    void          *ptr;
#endif
};

/* magazine list header structure */

struct magbkt {
    long        nref;
    struct mag *tab;
};

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
    struct magtab     hdrbuf[MALLOCNBKT];
#if (!MALLOCTLSARN)
    struct arn      **arntab;           // arena structures
#endif
    MUTEX            *pagedirlktab;
#if (MALLOCFREEPAGEDIR)
    struct memtab    *pagedir;          // allocation header lookup structure
#else
    void            **pagedir;
#endif
    MUTEX             initlk;           // initialization lock
    MUTEX             heaplk;           // lock for sbrk()
#if (!MALLOCTLSARN)
    long              curarn;
    long              narn;             // number of arenas in action
#endif
    volatile long     flg;              // allocator flags
    int               zerofd;           // file descriptor for mmap()
    struct mallopt    mallopt;          // mallopt() interface
    struct mallinfo   mallinfo;         // mallinfo() interface
};

#endif /* ___MALLOC_H__ */

