#ifndef ___MALLOC_H__
#define ___MALLOC_H__

#include <zero/param.h>

/* internal stuff for zero malloc - not for the faint at heart to modify :) */

#if !defined(MALLOCDEBUG)
#define MALLOCDEBUG       0
#endif
#if !defined(GNUTRACE)
#define GNUTRACE          0
#endif

/* optional features and other hacks */
#define MALLOCVALGRIND    0
#define MALLOCHDRHACKS    1 
#define MALLOCNEWHDR      1
#define MALLOCHDRPREFIX   1
#define MALLOCTLSARN      1
#define MALLOCSMALLADR    0
#define MALLOCSTAT        0
#define MALLOCPTRNDX      1
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
#define GNUTRACE          0
#define MALLOCTRACE       0

#define MALLOCSTEALMAG    0
#define MALLOCMULTITAB    0

#define MALLOCNOSBRK      1 // do NOT use sbrk()/heap, just mmap()
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
#define MALLOCSLABLOG2    16
#define MALLOCBIGSLABLOG2 20
#define MALLOCBIGMAPLOG2  24

#if !defined(MALLOCALIGNMENT) || (MALLOCALIGNMENT == 32)
#define MALLOCMINLOG2     5  // SIMD types
#elif !defined(MALLOCALIGNMENT) || (MALLOCALIGNMENT == 16)
#define MALLOCMINLOG2     4  // SIMD types
#elif !defined(MALLOCALIGNMENT) || (MALLOCALIGNMENT == 8)
#define MALLOCMINLOG2     3  // double
#else
#error fix MALLOCMILOG2 in _malloc.h
#endif

/* invariant parameters */
#define MALLOCMINSIZE     (1UL << MALLOCMINLOG2)
#define MALLOCNBKT        PTRBITS

#if defined(MALLOCDEBUG)
#if (MALLOCTRACE) && (GNUTRACE)
#define _assert(expr)                                                   \
    do {                                                                \
        if (!(expr)) {                                                  \
            int _sz;                                                    \
                                                                        \
            _sz = backtrace(tracetab, 64);                              \
            backtrace_symbols(tracetab, _sz);                           \
            *((long *)NULL) = 0;                                        \
        }                                                               \
    } while (0)
#else
#define _assert(expr)                                                   \
    do {                                                                \
        if (!(expr)) {                                                  \
            *((long *)NULL) = 0;                                        \
        }                                                               \
    } while (0)
#endif
//#include <assert.h>
#endif

/* internal macros */
#if (MALLOCFREEMAP) && (MALLOCPTRNDX)
#define magnbytetab(bktid)                                              \
    ((magnblk(bktid) << 1) * sizeof(MAGPTRNDX)                          \
     + rounduppow2(((magnblk(bktid) * CHAR_BIT + CHAR_BIT) >> 3),       \
                   PAGESIZE))
#elif (MALLOCPTRNDX) /* !MALLOCFREEMAP */
#define magnbytetab(bktid)                                              \
    ((magnblk(bktid) << 1) * sizeof(MAGPTRNDX))
#elif (MALLOCFREEMAP)
#define magnbytetab(bktid)                                              \
    ((magnblk(bktid) << 1) * sizeof(void *)                             \
     + rounduppow2(((magnblk(bktid) * CHAR_BIT + CHAR_BIT) >> 3),       \
                   PAGESIZE))
#else
#define magnbytetab(bktid)                                              \
    ((magnblk(bktid) << 1) * sizeof(void *))
#endif /* MALLOCFREEMAP && MALLOCPTRNDX */

#define magnbyte(bktid) (1UL << magnbytelog2(bktid))
#define ptralign(ptr, pow2)                                             \
    (!((uintptr_t)ptr & (align - 1))                                    \
     ? ptr                                                              \
     : ((void *)rounduppow2((uintptr_t)ptr, align)))
#if 0
#define blkalignsz(sz, aln)                                             \
    (((aln) <= PAGESIZE)                                                \
     ? max(sz, aln)                                                     \
     : (sz) + (aln))
#endif

#if defined(GNUMALLOC) && (GNUMALLOC)
#if !defined(__MALLOC_HOOK_VOLATILE)
#define MALLOC_HOOK_MAYBE_VOLATILE /**/
#elif !defined(MALLOC_HOOK_MAYBE_VOLATILE)
#define MALLOC_HOOK_MAYBE_VOLATILE __MALLOC_HOOK_VOLATILE
#endif
#endif

#if defined(__GLIBC__)
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
#elif defined(_GNU_SOURCE) && defined(GNUMALLOCHOOKS)
void         (* MALLOC_HOOK_MAYBE_VOLATILE __malloc_initialize_hook)(void);
void         (* MALLOC_HOOK_MAYBE_VOLATILE __after_morecore_hook)(void);
void        *(* MALLOC_HOOK_MAYBE_VOLATILE __malloc_hook)(size_t size,
                                                          const void *caller);
void        *(* MALLOC_HOOK_MAYBE_VOLATILE __realloc_hook)(void *ptr,
                                                            size_t size,
                                                            const void *caller);
void        *(* MALLOC_HOOK_MAYBE_VOLATILE __memalign_hook)(size_t align,
                                                            size_t size,
                                                            const void *caller);
void         (* MALLOC_HOOK_MAYBE_VOLATILE __free_hook)(void *ptr,
                                                        const void *caller);
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

#endif /* ___MALLOC_H__ */

