#ifndef ___MALLOC_H__
#define ___MALLOC_H__

#include <malloc.h>
#include <zero/param.h>

/* internal stuff for zero malloc - not for the faint at heart to modify :) */

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
#define MALLOCVALGRIND    0
#define MALLOCHDRHACKS    0
#define MALLOCNEWHDR      1
#define MALLOCHDRPREFIX   1
#define MALLOCTLSARN      1
#define MALLOCSMALLADR    0
#define MALLOCSTAT        0
#define MALLOCPTRNDX      0
#define MALLOCCONSTSLABS  1
#define MALLOCDYNARN      0
#define MALLOCGETNPROCS   0
#define MALLOCNOPTRTAB    0
#define MALLOCNARN        16
#define MALLOCEXPERIMENT  0
#define MALLOCNBUFHDR     16

#define MALLOCDEBUGHOOKS  0
#define MALLOCDIAG        0 // run [heavy] internal diagnostics for debugging
#define DEBUGMTX          0

#define MALLOCSTEALMAG    0
#define MALLOCMULTITAB    1

#define MALLOCNOSBRK      0 // do NOT use sbrk()/heap, just mmap()
#define MALLOCFREEMDIR    0 // under construction
#define MALLOCFREEMAP     0 // use free block bitmaps; bit 1 for allocated
#define MALLOCBUFMAP      1 // buffer mapped slabs to global pool

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
#define MALLOCSLABLOG2    18
#define MALLOCBIGSLABLOG2 20
#define MALLOCBIGMAPLOG2  24

#if !defined(MALLOCALIGNMENT) || (MALLOCALIGNMENT == 32)
#define MALLOCMINLOG2     5  // SIMD types
#elif !defined(MALLOCALIGNMENT) || (MALLOCALIGNMENT == 16)
#define MALLOCMINLOG2     4  // SIMD types
#elif !defined(MALLOCALIGNMENT) || (MALLOCALIGNMENT == 8)
#define MALLOCMINLOG2     3  // double
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
#define __mallocinitmtx(mp)  mtxinit(mp)
#define __malloctrylkmtx(mp) mtxtrylk(mp)
#define __malloclkmtx(mp)   (fprintf(stderr, "%p\tLK: %d\n", mp, __LINE__), \
                             mtxlk(mp))
#define __mallocunlkmtx(mp) (fprintf(stderr, "%p\tUNLK: %d\n", mp, __LINE__), \
                             mtxunlk(mp))
#define __mallocinitspin(mp) spininit(mp)
#define __malloclkspin(mp)   (fprintf(stderr, "LK: %d\n", __LINE__), spinlk(mp))
#define __mallocunlkspin(mp) (fprintf(stderr, "UNLK: %d\n", __LINE__), spinunlk(mp))
#elif (ZEROMTX)
#define __malloctrylkmtx(mp) mtxtrylk(mp)
#define __mallocinitmtx(mp)  mtxinit(mp)
#define __malloclkmtx(mp)    mtxlk(mp)
#define __mallocunlkmtx(mp)  mtxunlk(mp)
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
#define _assert(expr)                                                   \
    do {                                                                \
        if (!(expr)) {                                                  \
            *((long *)NULL) = 0;                                        \
        }                                                               \
    } while (0)
//#include <assert.h>
#endif

/* internal macros */
#define ptralign(ptr, pow2)                                             \
    ((void *)rounduppow2((uintptr_t)ptr, pow2))

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

#if (PTRBITS == 32)
#define MDIRNL1BIT     10
#define MDIRNL2BIT     10
#define MDIRNL3BIT     (PTRBITS - MDIRNL1BIT - MDIRNL2BIT - MALLOCMINLOG2)
#elif (PTRBITS == 64)
#define MDIRNL1BIT     16
#define MDIRNL2BIT     12
#define MDIRNL3BIT     12
#if (MALLOCSMALLADR)
#define MDIRNL4BIT     (ADRBITS - MDIRNL1BIT - MDIRNL2BIT - MDIRNL3BIT - MALLOCMINLOG2)
#else
#define MDIRNL4BIT     (PTRBITS - MDIRNL1BIT - MDIRNL2BIT - MDIRNL3BIT - MALLOCMINLOG2)
#endif
#else /* PTRBITS != 32 && PTRBITS != 64 */
#error fix PTRBITS for _malloc.h
#endif

#define MDIRNL1KEY     (1L << MDIRNL1BIT)
#define MDIRNL2KEY     (1L << MDIRNL2BIT)
#define MDIRNL3KEY     (1L << MDIRNL3BIT)
#if defined(MDIRNL1BIT)
#define MDIRNL4KEY     (1L << MDIRNL4BIT)
#endif

#define MDIRL1NDX      (MDIRL2NDX + MDIRNL2BIT)
#define MDIRL2NDX      (MDIRL3NDX + MDIRNL3BIT)
#define MDIRL3NDX      (MDIRL4NDX + MDIRNL4BIT)
#define MDIRL4NDX      MALLOCMINLOG2

#define mdirl1ndx(ptr) (((uintptr_t)(ptr) >> MDIRL1NDX) & ((1UL << MDIRNL1BIT) - 1))
#define mdirl2ndx(ptr) (((uintptr_t)(ptr) >> MDIRL2NDX) & ((1UL << MDIRNL2BIT) - 1))
#define mdirl3ndx(ptr) (((uintptr_t)(ptr) >> MDIRL3NDX) & ((1UL << MDIRNL3BIT) - 1))
#if defined(MDIRNL4BIT)
#define mdirl4ndx(ptr) (((uintptr_t)(ptr) >> MDIRL4NDX) & ((1UL << MDIRNL4BIT) - 1))
#endif

#endif /* MALLOCMULTITAB */

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
    volatile long   lk;
#if 0
    struct memtab  *tab;
#endif
    void           *base;
    void           *adr;
    size_t          size;
    long            cur;
    long            lim;
#if (!MALLOCTLSARN)
    long            arnid;
#endif
#if (MALLOCFREEMAP)
    volatile long   freelk;
    uint8_t        *freemap;
#endif
    long            bktid;
    struct mag     *prev;
    struct mag     *next;
#if (MALLOCPTRNDX)
    MAGPTRNDX      *stk;
    MAGPTRNDX      *ptrtab;
#elif (MALLOCHDRPREFIX)
    void          **stk;
    void          **ptrtab;
#endif
};

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

#if (MALLOCHDRHACKS)

/*
 * this structure is here for informative purposes; note that in core, the
 * header is right before the allocated address so you need to index it with
 * negative offsets
 */
struct memhdr {
    void      *mag;     // allocation magazine header
#if (MALLOCPTRNDX)
    MAGPTRNDX  ndx;
#endif
    uint8_t    bkt;     // bucket ID for block
    uint8_t    pad;     // header-imposed alignment for block
};

#if (MALLOCPTRNDX)
#define MEMHDRMAGOFS     (offsetof(struct memhdr, mag) / sizeof(void *))
#define MEMHDRSIZE       (sizeof(void *)                                \
                          + sizeof(MAGPTRNDX)                           \
                          + 2 * sizeof(int8_t))
#define MEMHDRNDXOFS     (offsetof(struct memhdr, ndx) / sizeof(MAGPTRNDX))
#define getndx(ptr)      (((MAGPTRNDX *)(ptr))[-(1 + MEMHDRNDXOFS)])
#define setndx(ptr, ndx) ((((MAGPTRNDX *)(ptr))[-(1 + MEMHDRNDXOFS)]) = (ndx))
#else /* !MALLOCPTRNDX */
#define MEMHDRSIZE       (sizeof(void *)                                \
                          + sizeof(int16_t)                             \
                          + 2 * sizeof(int8_t))
#define MEMHDRNDXOFS     (offsetof(struct memhdr, ndx) / sizeof(uint16_t))
#define getndx(ptr)      (((uint16_t *)(ptr))[-(1 + MEMHDRNDXOFS)])
#define setndx(ptr, ndx) ((((uint16_t *)(ptr))[-(1 + MEMHDRNDXOFS)]) = (ndx))
#endif /* MALLOCPTRNDX */

#define MEMHDRBKTOFS     (offsetof(struct memhdr, bkt))
#define MEMHDRPADOFS     (offsetof(struct memhdr, pad))
#define setmag(ptr, mag) ((((void **)(ptr))[-(1 + MEMHDRMAGOFS)] = (mag)))
#define getmag(ptr)      ((((void **)(ptr))[-(1 + MEMHDRMAGOFS)]))
#define getbkt(ptr)      (((uint8_t *)(ptr))[-(1 + MEMHDRBKTOFS)])
#define setbkt(ptr, bkt) ((((uint8_t *)(ptr))[-(1 + MEMHDRBKTOFS)]) = (bkt))
#define getpad(ptr)      (((uint8_t *)(ptr))[-(1 + MEMHDRPADOFS)])
#define setpad(ptr, pad) ((((uint8_t *)(ptr))[-(1 + MEMHDRPADOFS)]) = (pad))

#else /* !MALLOCHDRHACKS */

struct memhdr {
    void *mag;
};
#define MEMHDRSIZE       (sizeof(void *))
#define MEMHDRMAGOFS     (offsetof(struct memhdr, mag) / sizeof(void *))
#define setmag(ptr, mag) ((((void **)(ptr))[-(1 + MEMHDRMAGOFS)] = (mag)))
#define getmag(ptr)      ((((void **)(ptr))[-(1 + MEMHDRMAGOFS)]))
#if (MALLOCPTRNDX)
#define getndx(ptr)      (getmag(ptr)->ptrtab[getndx(ptr)])
#define setndx(ptr, ndx) (getmag(ptr)->ptrtab[getndx(ptr)] = (ndx))
#define getbkt(ptr)      (getmag(ptr)->bktid)
#define setbkt(ptr, bkt) (getmag(ptr)->bktid = (bkt))
#endif

#endif /* MALLOCHDRHACKS */

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
    long              flg;              // allocator flags
    int               zerofd;           // file descriptor for mmap()
    struct mallopt    mallopt;          // mallopt() interface
    struct mallinfo   mallinfo;         // mallinfo() interface
};

#endif /* ___MALLOC_H__ */

