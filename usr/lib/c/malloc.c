/*
 * Zero Malloc Revision 3
 *
 * - a beta version of a new malloc for Zero.
 *
 * Copyright (C) Tuomo Petteri Venäläinen 2014-2016
 */

#define MALLOCDEBUG 1
#define MALLOCTRACE 0

#include "_malloc.h"

/*
 * TODO
 * ----
 * - lookups;
 *   - multiples of page-size with a "v-tree" / multilevel table
 *   - cacheline-aligned ones;
 *     - pointer-prefix (allocation magazines)
 * - ...
 */

#if 0
#undef  __malloctrace
#define __malloctrace()
#undef  __mallocprnttrace
#define __mallocprnttrace(str, sz, aln)                                 \
    do {                                                                \
        fprintf(stderr, "%s: %llu(%llu)\n", str, sz, aln);              \
    } while (0)
#endif

/*
 * TODO
 * ----
 * - fix mallinfo() to return proper information
 */
#if defined(NVALGRIND)
#undef  MALLOCVALGRIND
#define MALLOCVALGRIND    1
#elif !defined(MALLOCVALGRIND)
#define MALLOCVALGRIND    1
#endif
#define MALLOCSMALLSLABS  0
#define MALLOCSIG         1
#define MALLOC4LEVELTAB   1

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

#define MALLOCHOOKS 0
#if !defined(MALLOCDEBUGHOOKS)
#define MALLOCDEBUGHOOKS 0
#endif

#if (MALLOCTRACE)
void *tracetab[64];
#endif

struct mag;
static void * maginitslab(struct mag *mag, long bktid);
static void * maginittab(struct mag *mag, long bktid);
static void * maginit(struct mag *mag, long bktid);

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
#if (PTHREAD)
#include <pthread.h>
#endif
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/trix.h>
#if (GNUTRACE) && (MALLOCTRACE)
#include <zero/gnu.h>
#endif
#if (MALLOCDEBUGHOOKS)
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

#if (!PTRFLGMASK)
#define clrptr(ptr)                                                     \
    ((ptr))
#else
#define clrptr(ptr)                                                     \
    ((void *)((uintptr_t)ptr & ~PTRADRMASK))
#endif
#define clradr(adr)                                                     \
    ((uintptr_t)(adr) & ~ADRMASK)
#define ptrdiff(ptr1, ptr2)                                             \
    ((uintptr_t)(ptr2) - (uintptr_t)(ptr1))

#define magptrid(mag, ptr)                                              \
    (((uintptr_t)clrptr(ptr) - (uintptr_t)(mag)->base) >> (mag)->bktid)
#define magputptr(mag, ptr, orig)                                       \
    (((void **)(mag)->ptr)[magptrid(mag, ptr)] = (orig))
#define maggetptr(mag, ptr)                                             \
    (((void **)(mag)->ptr)[magptrid(mag, ptr)])

//#define setptr(a, p) ((&((struct memhdr *)(a))[-1])->base = (p))

/*
 * magazines for bucket bktid have 1 << magnblklog2(bktid) blocks of
 * 1 << bktid bytes
 */
#define magnblklog2(bktid)                                              \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? (MALLOCSLABLOG2 - (bktid))                                       \
     : 0)
#define magnblk(bktid)                                                  \
    (1UL << magnblklog2(bktid))

#define maghdrsz(bktid)                                                 \
    (rounduppow2(sizeof(struct mag), CLSIZE))
#if (MALLOCFREEMAP)
#define magtabsz(bktid)                                                 \
    ((maghdrsz()                                                        \
      + ((!magnblklog2(bktid)                                           \
          ? 0                                                           \
          : ((magnblk(bktid) << 1) * sizeof(void *)                     \
             + ((magnblk(bktid) + CHAR_BIT) >> 3))))))
#else
#define magtabsz(bktid)                                                 \
    ((maghdrsz()                                                        \
      + (!magnblklog2(bktid)                                            \
         ? 0                                                            \
         : (magnblk(bktid) << 1) * sizeof(void *))))
#endif
#define magembedtab(bktid)                                              \
    (magtabsz(bktid) <= MALLOCHDRSIZE)

#if (MALLOCDEBUG) || (MALLOCDIAG)
void
magprint(struct mag *mag)
{
    fprintf(stderr, "MAG %p\n", mag);
    fprintf(stderr, "\tbase\t%p\n", mag->base);
    fprintf(stderr, "\tadr\t%p\n", mag->adr);
    fprintf(stderr, "\tcur\t%ld\n", mag->cur);
    fprintf(stderr, "\tlim\t%ld\n", mag->lim);
    fprintf(stderr, "\tbktid\t%ld\n", mag->bktid);
    fprintf(stderr, "\tstk\t%p\n", mag->stk);
    fprintf(stderr, "\tptr\t%p\n", mag->ptr);
#if (MALLOCFREEMAP)
    fprintf(stderr, "\tfreemap\t%p\n", mag->freemap);
#endif
    fflush(stderr);

    return;
}
#endif

/* start of the allocator proper */

static struct malloc        g_malloc ALIGNED(PAGESIZE);
//THREADLOCAL struct arn     thrarn ALIGNED(CLSIZE);
THREADLOCAL struct arn     *thrarn;
THREADLOCAL pthread_once_t  thronce;
THREADLOCAL pthread_key_t   thrkey;
THREADLOCAL long            thrflg;
#if (MALLOCSTAT)
unsigned long long          nheapbyte;
unsigned long long          nmapbyte;
unsigned long long          nhdrbyte;
unsigned long long          ntabbyte;
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
#endif /* MALLOCSIG */

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

static void
magputhdr(struct mag *mag)
{
    long bktid = mag->bktid;

/* add magazine header to header cache */
    __malloclkmtx(&g_malloc.hdrbuf[bktid].lk);
    mag->next =  g_malloc.hdrbuf[bktid].ptr;
    if (mag->next) {
        mag->next->prev = mag;
    }
    g_malloc.hdrbuf[bktid].ptr = mag;
    __mallocunlkmtx(&g_malloc.hdrbuf[bktid].lk);
    
    return;
}
    
static struct mag *
maggethdr(long bktid)
{
    struct mag *mag;
    void       *ret;
    struct mag *hdr;
    struct mag *tail;
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
    if (mag) {
        __mallocunlkmtx(&g_malloc.hdrbuf[bktid].lk);
        mag->bktid = bktid;
    } else {
        __mallocunlkmtx(&g_malloc.hdrbuf[bktid].lk);
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

static struct mag *
magget(long bktid)
{
    struct mag *mag = maggethdr(bktid);
    
    if (mag) {
        if (!maginit(mag, bktid)) {
            
            return NULL;
        }
        _assert(!mag->cur);
        _assert(mag->lim);
        mag->prev = NULL;
        mag->next = NULL;
    }

    return mag;
}

static void
thrfreearn(void *arg)
{
    struct arn    *arn = arg;
    struct mag    *mag;
    struct mag    *tail = NULL;
    long           bktid;

    for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
        mag = arn->magbkt[bktid].ptr;
        if (mag) {
            mag->arn = NULL;
            while (mag->next) {
                mag = mag->next;
                mag->arn = NULL;
                tail = mag;
            }
            __malloclkmtx(&g_malloc.magbkt[bktid].lk);
            if (tail) {
                tail->next = g_malloc.magbkt[bktid].ptr;
                if (tail->next) {
                    tail->next->prev = tail;
                }
            } else {
                mag->next = g_malloc.magbkt[bktid].ptr;
                if (mag->next) {
                    mag->next->prev = mag;
                }
            }
            g_malloc.magbkt[bktid].ptr = mag;
            __mallocunlkmtx(&g_malloc.magbkt[bktid].lk);
        }
    }
    unmapanon(arn, rounduppow2(sizeof(struct arn), PAGESIZE));

    return;
}

static void
thrinitarn(void)
{
    pthread_key_create(&thrkey, thrfreearn);
}

static void *
thrinit(void)
{
    void *ptr = NULL;

    pthread_once(&thronce, thrinitarn);
    ptr = pthread_getspecific(thrkey);
    if (!ptr) {
        ptr = mapanon(g_malloc.zerofd, rounduppow2(sizeof(struct arn),
                                                   PAGESIZE));
        pthread_setspecific(thrkey, ptr);
    }
    
    return ptr;
}

static void *
maginitslab(struct mag *mag, long bktid)
{
    void      **stk;
    size_t      sz = 1UL << bktid;
    long        n = magnblk(bktid);
    uint8_t    *ptr;
    long        ndx;
    uintptr_t   magflg = 0;
    uintptr_t   ptrflg = 0;

    if (!mag->base) {
        _assert(powerof2(n) && powerof2(sz));
        ptr = SBRK_FAILED;
#if (!MALLOCNOSBRK)
        if (bktid <= MALLOCSLABLOG2) {
            /* try to allocate slab from heap */
            __malloclkmtx(&g_malloc.heaplk);
            ptr = growheap(rounduppow2(n * sz, PAGESIZE));
            __mallocunlkmtx(&g_malloc.heaplk);
        }
#endif
        if (ptr == SBRK_FAILED) {
            /* try to map slab */
            ptr = mapanon(g_malloc.zerofd,
                             rounduppow2(n * sz, PAGESIZE));
            if (ptr == MAP_FAILED) {
                
                return NULL;
            }
#if (MALLOCSTAT)
            nmapbyte += rounduppow2(n * sz, PAGESIZE);
#endif
            magflg |= MAGMAP;
#if (MALLOCSTAT)
        } else {
            nheapbyte += rounduppow2(n * sz, PAGESIZE);
#endif
        }
#if 0
        fprintf(stderr, "allocating %ld %ld-byte blocks (%ld)\n",
                n, sz, bktid);
#endif
        if (n == 1) {
            mag->ptr = ptr;
        }
        mag->base = ptr;
        mag->adr = (void *)((uintptr_t)ptr | magflg);
        mag->size = rounduppow2(n * sz, PAGESIZE);
        mag->cur = 0;
        mag->lim = n;
        mag->bktid = bktid;
        if (gtpow2(n, 1)) {
            stk = mag->stk;
            ndx = 0;
            /* initialise allocation stack */
            while (n--) {
                stk[ndx] = ptr;
                ndx++;
                ptr += sz;
            }
        }
        VALGRINDMKPOOL(mag->base, 0);
    }

    return mag;
}

static void *
maginittab(struct mag *mag, long bktid)
{
    void        *ret;
    long         n = magnblk(bktid);
    size_t       sz;

    if (!mag->stk) {
        if (n == 1) {
            mag->stk = NULL;
            mag->ptr = NULL;
#if (MALLOCFREEMAP)
            mag->freemap = NULL;
#endif
        } else if (magembedtab(bktid)) {
            mag->stk = (void **)((uint8_t *)mag + maghdrsz());
            mag->ptr = ((uint8_t *)mag->stk
                        + n * sizeof(void *));
#if (MALLOCFREEMAP)
            mag->freemap = ((uint8_t *)mag->stk
                            + (n << 1) * sizeof(void *));
#endif
        } else {
            sz = magtabsz(bktid);
            ret = mapanon(g_malloc.zerofd, rounduppow2(sz, PAGESIZE));
            if (ret == MAP_FAILED) {
                
                return NULL;
            }
            mag->stk = ret;
            mag->ptr = ((uint8_t *)mag->stk
                        + n * sizeof(void *));
#if (MALLOCFREEMAP)
            mag->freemap = ((uint8_t *)mag->stk
                            + (n << 1) * sizeof(void *));
#endif
#if (MALLOCSTAT)
            nmapbyte += rounduppow2(sz, magtabsz(bktid));
#endif
        }
    }

    return mag;
 }
    
static void *
maginit(struct mag *mag, long bktid)
{
    if (!mag->stk && !mag->ptr) {
        maginittab(mag, bktid);
    }
    if (!mag->base) {
        if (!maginitslab(mag, bktid)) {
            
            return NULL;
        }
    }

    return mag;
}

static void
prefork(void)
{
    long ndx;

    __malloclkmtx(&g_malloc.initlk);
    __malloclkmtx(&g_malloc.heaplk);
    for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
        __malloclkmtx(&g_malloc.hdrbuf[ndx].lk);
//        __malloclkmtx(&g_malloc.freetab[ndx].lk);
        __malloclkmtx(&g_malloc.magbkt[ndx].lk);
    }
#if (MALLOCMULTITAB)
    for (ndx = 0 ; ndx < MDIRNL1KEY ; ndx++) {
        __malloclkmtx(&g_malloc.mlktab[ndx]);
    }
#endif
    
    return;
}

static void
postfork(void)
{
    long ndx;

#if (MALLOCMULTITAB)
    for (ndx = 0 ; ndx < MDIRNL1KEY ; ndx++) {
        __mallocunlkmtx(&g_malloc.mlktab[ndx]);
    }
#endif
    for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
        __mallocunlkmtx(&g_malloc.magbkt[ndx].lk);
//        __mallocunlkmtx(&g_malloc.freetab[ndx].lk);
        __mallocunlkmtx(&g_malloc.hdrbuf[ndx].lk);
    }
    __mallocunlkmtx(&g_malloc.heaplk);
    __mallocunlkmtx(&g_malloc.initlk);
    
    return;
}

#if (MALLOCMULTITAB)
    
static struct mag *
mtfindmag(void *ptr)
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
    if (ptr1) {
        ptr2 = ((void **)ptr1)[l2];
        if (ptr2) {
            ptr1 = ((void **)ptr2)[l3];
            if (ptr1) {
                mag = ((void **)ptr1)[l4];
            }
        }
    }
#endif
    __mallocunlkmtx(&g_malloc.mlktab[l1]);

    return mag;
}

static void
mtsetmag(void *ptr,
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
    if (!mag) {
#if (MALLOCFREEMDIR)
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
#else
        ptr1 = g_malloc.mdir[l1];
        if (ptr1) {
            ptr2 = ((void **)ptr1)[l2];
            if (ptr2) {
                ptr1 = ((void **)ptr2)[l3];
                if (ptr1) {
                    ((void **)ptr1)[l4] = mag;
                }
            }
        }
#endif
    }
    if (mag) {
#if (!MALLOCFREEMDIR)
        ptr1 = g_malloc.mdir[l1];
        if (!ptr1) {
            ptr1 = mapanon(g_malloc.zerofd,
                           MDIRNL2KEY * sizeof(void *));
            if (ptr1 == MAP_FAILED) {
                abort();
            }
            g_malloc.mdir[l1] = ptr1;
        }
        if (ptr1) {
            ptr2 = ((void **)ptr1)[l2];
            if (!ptr2) {
                ptr2 = mapanon(g_malloc.zerofd,
                               MDIRNL3KEY * sizeof(void *));
                if (ptr2 == MAP_FAILED) {
                    abort();
                }
                ((void **)ptr1)[l2] = ptr2;
            }
            if (ptr2) {
                ptr1 = ((void **)ptr2)[l3];
                if (!ptr1) {
                    ptr1 = mapanon(g_malloc.zerofd,
                                   MDIRNL4KEY * sizeof(void *));
                    if (ptr1 == MAP_FAILED) {
                        abort();
                    }
                    ((void **)ptr2)[l3] = ptr1;
                }
                if (ptr1) {
                    ((void **)ptr1)[l4] = mag;
                }
            }
        }
#else  /* MALLOCFREEMDIR */
        mptr1 = g_malloc.mdir[l1];
        if (!mptr1) {
            empty = 1;
            mptr1 = mapanon(g_malloc.zerofd,
                            MDIRNL2KEY * sizeof(struct memtab));
            if (mptr1 != MAP_FAILED) {
                ptab[0] = mptr1;
                g_malloc.mdir[l1] = mptr1;
#if defined(VALGRINDINTERNALS)
                VALGRINDALLOC(mptr1, MDIRNL2KEY * sizeof(struct memtab), 0);
#endif
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
#if defined(VALGRINDINTERNALS)
                        VALGRINDALLOC(mptr2,
                                      MDIRNL3KEY * sizeof(struct memtab), 0);
#endif
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
#if defined(VALGRINDINTERNALS)
                    VALGRINDALLOC(mptr2, MDIRNL4KEY * sizeof(void *), 0);
#endif
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
#if defined(VALGRINDINTERNALS)
                    VALGRINDFREE(mptr1);
#endif
                }
            }
            mptr1 = ptab[0];
            if (mptr1) {
                if (!--mptr1->nref) {
                    unmapanon(mptr2, MDIRNL3KEY * sizeof(struct memtab));
#if defined(VALGRINDINTERNALS)
                    VALGRINDFREE(mptr2);
#endif
                }
            }
            mptr2 = ptab[0];
            if (mptr2) {
                if (empty) {
                    unmapanon(mptr1, MDIRNL2KEY * sizeof(struct memtab));
#if defined(VALGRINDINTERNALS)
                    VALGRINDFREE(mptr1);
#endif
                }
            }
        }
#endif /* MALLOCFREEMDIR */
    }
    __mallocunlkmtx(&g_malloc.mlktab[l1]);
    
    return;
}

#endif /* MALLOCMULTITAB */

#if (GNUMALLOC)
void
#else
static void
#endif
mallinit(void)
{
    long  bktid;
#if (!MALLOCNOSBRK)
    void *heap;
    long  ofs;
#endif

    __malloclkmtx(&g_malloc.initlk);
    if (g_malloc.flg & MALLOCINIT) { 
        __mallocunlkmtx(&g_malloc.initlk);
       
        return;
    }
#if defined(GNUMALLOC) && (GNUMALLOC)
    __malloc_hook = gnu_malloc_hook;
    __realloc_hook = gnu_realloc_hook;
    __memalign_hook = gnu_memalign_hook;
    __free_hook = gnu_free_hook;
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
    bktid = MALLOCNBKT;
    while (bktid--) {
        __mallocinitmtx(&g_malloc.hdrbuf[bktid].lk);
        __mallocinitmtx(&g_malloc.magbkt[bktid].lk);
//        __mallocinitmtx(&g_malloc.freetab[bktid].lk);
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
#if (MALLOCMULTITAB)
    g_malloc.mlktab = mapanon(g_malloc.zerofd, MDIRNL1KEY * sizeof(long));
#if (MALLOCFREEMDIR)
    g_malloc.mdir = mapanon(g_malloc.zerofd,
                            MDIRNL1KEY * sizeof(struct memtab));
#else
    g_malloc.mdir = mapanon(g_malloc.zerofd,
                            MDIRNL1KEY * sizeof(void *));
#endif
#endif /* MALLOCMULTITAB */
#if (MALLOCSTAT)
    ntabbyte += MDIRNL1KEY * sizeof(long);
    ntabbyte += MDIRNL1KEY * sizeof(void *);
#endif
#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zmalloc_initialize_hook) {
        __zmalloc_initialize_hook();
    }
#endif
    pthread_atfork(prefork, postfork, postfork);
    g_malloc.flg |= MALLOCINIT;
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
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
__attribute__ ((malloc))
#endif
_malloc(size_t size,
        size_t align,
        long zero)
{
    struct mag  *mag = NULL;
    struct arn  *arn;
    uint8_t     *adr;
    uint8_t     *ptr = NULL;
    size_t       sz = ((size & (PAGESIZE  - 1))
                       ? ((align)
                          ? (size + align)
                          : (size + MALLOCALIGNMENT))
                       : (size
                          + ((align <= PAGESIZE)
                             ? 0
                             : rounduppow2(size + align, PAGESIZE))));
    long         bktid = blkbktid(sz);
    long         lim;
#if (MALLOCHDRHACKS)
    uint8_t      nfo = bktid;
#endif
    
    if (!(g_malloc.flg & MALLOCINIT)) {
        mallinit();
    }
    arn = thrarn;
    if (!thrarn) {
        arn = thrinit();
        thrarn = arn;
    }
//    align = max(align, MALLOCALIGNMENT);
    /* try to allocate from a partially used magazine */
    if (arn) {
        mag = arn->magbkt[bktid].ptr;
        if (mag) {
            lim = mag->lim;
            _assert(mag->cur <= mag->lim);
            _assert((mag->stk) || (mag->ptr));
            if (lim == 1) {
                ptr = mag->ptr;
                mag->cur = 1;
            } else {
                ptr = ((void **)mag->stk)[mag->cur++];
            }
            _assert(mag->cur <= lim);
            if (mag->cur == lim) {
                mag->arn = NULL;
                if (mag->next) {
                    mag->next->prev = NULL;
                }
                arn->magbkt[bktid].ptr = mag->next;
                mag->prev = NULL;
                mag->next = NULL;
            }
        }
    }
    if (!mag) {
        __malloclkmtx(&g_malloc.magbkt[bktid].lk);
        mag = g_malloc.magbkt[bktid].ptr;
        if (!mag) {
//            __mallocunlkmtx(&g_malloc.magbkt[bktid].lk);
            mag = magget(bktid);
        } else {
            if (mag->next) {
                mag->next->prev = NULL;
            }
            g_malloc.magbkt[bktid].ptr = mag->next;
//            __mallocunlkmtx(&g_malloc.magbkt[bktid].lk);
        }
        if (mag) {
//            _assert(mag->cur);
            if (mag->lim == 1) {
                ptr = mag->ptr;
                mag->cur = 1;
            } else {
                ptr = ((void **)mag->stk)[mag->cur++];
            }
            mag->prev = NULL;
            if (mag->cur < mag->lim) {
                if (arn->magbkt[bktid].ptr || (bktid > MALLOCBIGSLABLOG2)) {
//                    __malloclkmtx(&g_malloc.magbkt[bktid].lk);
                    mag->next = g_malloc.magbkt[bktid].ptr;
                    if (mag->next) {
                        mag->next->prev = mag;
                    }
                    g_malloc.magbkt[bktid].ptr = mag;
//                    __malloclkmtx(&g_malloc.magbkt[bktid].lk);
                } else {
                    mag->arn = arn;
                    mag->next = NULL;
                    arn->magbkt[bktid].ptr = mag;
                }
            } else {
                mag->next = NULL;
            }
        }
        __mallocunlkmtx(&g_malloc.magbkt[bktid].lk);
    }
    if (!ptr) {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
        
        return NULL;
    }
    adr = clrptr(ptr);
    if (zero) {
        memset(adr, 0, size);
    } else if (g_malloc.mallopt.flg & MALLOPT_PERTURB_BIT) {
        int perturb = g_malloc.mallopt.perturb;
        
        perturb = (~perturb) & 0xff;
        memset(adr, perturb, 1UL << bktid);
    }
    if (((uintptr_t)ptr & (PAGESIZE - 1)) || (align > PAGESIZE)) {
        /* store unaligned source pointer and mag address */
        ptr += max(align, MALLOCALIGNMENT);
        if (align) {
            if ((uintptr_t)ptr & (align - 1)) {
                ptr = ptralign(ptr, align);
            }
#if (MALLOCHDRHACKS)
            nfo |= MEMHDRALNBIT;
#endif
        }
        if ((uintptr_t)ptr & (PAGESIZE - 1))  {
            setmag(ptr, mag);
        } else {
            mtsetmag(ptr, mag);
        }
    } else {
        mtsetmag(ptr, mag);
    }
    if (mag->lim == 1) {
        mag->ptr = adr;
    } else {
        magputptr(mag, ptr, adr);
    }
#if (MALLOCHDRHACKS)
    setnfo(ptr, nfo);
#endif
#if (MALLOCFREEMAP)
    __malloclkmtx(&mag->freelk);
    if (((mag->lim == 1) && (mag->freemap))
        || bitset(mag->freemap, magptrid(mag, adr))) {
        magprint(mag);
        fprintf(stderr, "trying to reallocate block");
        fflush(stderr);
        
        abort();
    } else if (mag->lim == 1) {
        mag->freemap = (void *)(~(uintptr_t)NULL);
    } else {
        setbit(mag->freemap, magptrid(mag, adr));
    }
    __mallocunlkmtx(&mag->freelk);
#endif
    VALGRINDALLOC(adr,
                  size,
                  zero);
    
    return ptr;
}

void
_free(void *ptr)
{
    struct arn *arn;
    struct mag *mag = NULL;
    void       *adr = NULL;
    long        queue = 0;
    long        bktid;
    long        lim;
#if (MALLOCFREEMAP)
    long        ndx;
#endif
#if (MALLOCHDRHACKS)
    uint8_t     nfo = 0;
#endif

    if (!ptr) {

        return;
    }
    arn = thrarn;
    if (!thrarn) {
        arn = thrinit();
        thrarn = arn;
    }
    if ((uintptr_t)ptr & (PAGESIZE - 1)) {
        mag = getmag(ptr);
    } else {
        mag = mtfindmag(ptr);
    }
#if (MALLOCHDRHACKS)
    nfo = getnfo(ptr);
#endif
    if (mag) {
        if ((uintptr_t)ptr & (PAGESIZE - 1)) {
            setmag(ptr, NULL);
        } else {
            mtsetmag(ptr, NULL);
        }
#if (MALLOCHDRHACKS)
        bktid = nfo & MEMHDRBKTMASK;
#else
        bktid = mag->bktid;
#endif
#if (MALLOCHDRHACKS)
        if (!(nfo & MEMHDRALNBIT)) {
            adr = (void *)rounddownpow2((uintptr_t)ptr, 1UL << bktid);
        } else if (mag->lim == 1) {
            adr = mag->ptr;
        } else {
            adr = maggetptr(mag, ptr);
        }
        if (!(nfo & MEMHDRFREEBIT)) {
            nfo = bktid | MEMHDRFREEBIT;
            setnfo(ptr, nfo);
        } else {
            fprintf(stderr, "duplicate free: %p\n", ptr);

            return;
        }
#else
        if (mag->lim == 1) {
            adr = mag->ptr;
        } else {
            adr = maggetptr(mag, ptr);
        }
#endif
        VALGRINDFREE(adr);
#if (MALLOCFREEMAP)
        /* FIXME: use m_cmpclrbit() */
        ndx = magptrid(mag, adr);
        __malloclkmtx(&mag->freelk);
        if ((mag->lim == 1 && (mag->freemap))
            || !bitset(mag->freemap, ndx)) {
            magprint(mag);
            fprintf(stderr, "trying to free an unused block\n");
            
            abort();
        } else if (mag->cur == 1) {
            mag->freemap = (void *)(~(uintptr_t)NULL);
        } else {
            clrbit(mag->freemap, magptrid(mag, adr));
        }
        __mallocunlkmtx(&mag->freelk);
#endif
#if 0
        if (g_malloc.mallopt.flg & MALLOPT_PERTURB_BIT) {
            int perturb = g_malloc.mallopt.perturb;
            
            perturb &= 0xff;
            memset(adr, perturb, sz);
        }
#endif
        _assert(mag->cur <= mag->lim);
        if (mag->lim == 1) {
            mag->ptr = NULL;
        } else {
            ((void **)mag->stk)[--mag->cur] = adr;
        }
        lim = mag->lim;
        if (!mag->cur) {
            if (lim == 1) {
                if ((uintptr_t)mag->adr & MAGMAP
                    && (mag->bktid > MALLOCBIGMAPLOG2)) {
                    /* unmap slab */
                    adr = (void *)mag->base;
                    VALGRINDRMPOOL(adr);
                    if ((mag->lim > 1) && !magembedtab(bktid)) {
                        unmapanon(mag->stk, rounduppow2(magtabsz(bktid),
                                                        PAGESIZE));
                        mag->stk = NULL;
                        mag->ptr = NULL;
#if (MALLOCFREEMAP)
                        mag->freemap = NULL;
#endif
#if (MALLOCSTAT)
                        ntabbyte -= magtabsz(bktid);
#endif
                    } else if (mag->lim == 1) {
                        mag->ptr = NULL;
                    }
                    unmapanon(adr, mag->size);
#if (MALLOCSTAT)
                    nmapbyte -= mag->size;
#endif
                    mag->base = NULL;
                    mag->adr = NULL;
                    mag->bktid = 0;
                    mag->size = 0;
                    mag->cur = 0;
                    mag->lim = 0;
                    magputhdr(mag);
                } else {
                    queue = 1;
                }
            } else if (!mag->arn || (mag->arn == arn)) {
                __malloclkmtx(&g_malloc.magbkt[bktid].lk);
                if ((mag->prev) && (mag->next)) {
                    mag->next->prev = mag->prev;
                    mag->prev->next = mag->next;
                } else if (mag->prev) {
                    mag->prev->next = NULL;
                } else if (mag->next) {
                    mag->next->prev = NULL;
                    if (mag->arn) {
                        arn->magbkt[bktid].ptr = mag->next;
                        mag->arn = NULL;
                    } else {
                        g_malloc.magbkt[bktid].ptr = mag->next;
                    }
                } else if (mag->arn) {
                    arn->magbkt[bktid].ptr = NULL;
                    mag->arn = NULL;
                } else {
                    g_malloc.magbkt[bktid].ptr = NULL;
                }
                __mallocunlkmtx(&g_malloc.magbkt[bktid].lk);
                mag->arn = NULL;
                queue = 1;
            }
        } else if (mag->cur == lim - 1) {
//            mag->tab = NULL;
            /* queue an unqueued earlier fully allocated magazine */
            queue = 1;
        }
        if (queue) {
            mag->prev = NULL;
            if (arn->magbkt[bktid].ptr || (bktid > MALLOCBIGSLABLOG2)) {
                mag->arn = NULL;
                __malloclkmtx(&g_malloc.magbkt[bktid].lk);
                mag->next = g_malloc.magbkt[bktid].ptr;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                g_malloc.magbkt[bktid].ptr = mag;
                __mallocunlkmtx(&g_malloc.magbkt[bktid].lk);
            } else {
                mag->arn = arn;
                mag->next = arn->magbkt[bktid].ptr;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                arn->magbkt[bktid].ptr = mag;
            }
        }
    }
    
    return;
}

/* Internal function for realloc() and reallocf() */
void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
#endif
_realloc(void *ptr,
         size_t size,
         long rel)
{
    uint8_t    *retptr = NULL;
    size_t      sz = 1UL << (blkbktid(size + MALLOCALIGNMENT));
    struct mag *mag = (ptr) ? getmag(ptr) : NULL;
    long        bktid = blkbktid(sz);

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
    if (((rel) && (ptr)) || (retptr != ptr)) {
        _free(ptr);
    }
#if (MALLOCDEBUG)
    _assert(retptr != NULL);
#endif
    if (!retptr) {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
    }

    return retptr;
}

/* API FUNCTIONS */

void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
__attribute__ ((malloc))
#endif
#if defined(GNUMALLOC) && (GNUMALLOC)
zero_malloc(size_t size)
#else
malloc(size_t size)
#endif
{
    void   *ptr = NULL;
    size_t  sz = ((size & (PAGESIZE - 1))
                  ? (size)
                  : rounduppow2(size, PAGESIZE));

    if (!size) {
#if defined(_GNU_SOURCE)
        ptr = _malloc(MALLOCALIGNMENT, 0, 0);
#endif
        
        return ptr;
    }
#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zmalloc_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmalloc_hook(size, (const void *)caller);
        
        return ptr;
    }
#endif
    ptr = _malloc(sz, 0, 0);
    if (!ptr) {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
        
        return NULL;
    }
#if (MALLOCTRACE)
    __malloctrace();
    __mallocprnttrace("malloc", size, 0);
#endif
    
    return ptr;
}

void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(1, 2)))
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
__attribute__ ((malloc))
#endif
#if defined(GNUMALLOC) && (GNUMALLOC)
zero_calloc(size_t n, size_t size)
#else
calloc(size_t n, size_t size)
#endif
{
    size_t sz = n * (size + MALLOCALIGNMENT);
    void *ptr = NULL;

    if (!n || !size) {
#if defined(_GNU_SOURCE)
        ptr = _malloc(MALLOCALIGNMENT, 0, 1);
#endif
        
        return ptr;
    }
#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zmalloc_hook) {
        void *caller = NULL;

        m_getretadr(caller);
        ptr = __zmalloc_hook(size, (const void *)caller);

        return ptr;
    }
#endif
    ptr = _malloc(sz, 0, 1);
#if (MALLOCDEBUG)
    _assert(ptr != NULL);
#endif
#if (MALLOCTRACE)
    __malloctrace();
    __mallocprnttrace("calloc", size, n);
#endif

    return ptr;
}

void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
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

#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zrealloc_hook) {
        void *caller = NULL;

        m_getretadr(caller);
        ptr = __zrealloc_hook(ptr, size, (const void *)caller);

        return ptr;
    }
#endif
    if (!size && (ptr)) {
        _free(ptr);
    } else {
        retptr = _realloc(ptr, size, 0);
        if (retptr) {
            if (retptr != ptr) {
                _free(ptr);
            }
        }
    }
#if (MALLOCDEBUG)
    _assert(retptr != NULL);
#endif
#if (MALLOCTRACE)
    __malloctrace();
    if (ptr) {
        fprintf(stderr, "REALLOC: %p\n", ptr);
    } 
    __mallocprnttrace("realloc", size, 0);
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
#if (MALLOCTRACE)
    __malloctrace();
    if (ptr) {
        fprintf(stderr, "free: %p\n", ptr);
    }
    __mallocprnttrace("free", 0, 0);
#endif
#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zfree_hook) {
        void *caller = NULL;

        m_getretadr(caller);
        __zfree_hook(ptr, (const void *)caller);

        return;
    }
#endif
    if (ptr) {
        _free(ptr);
    }

    return;
}

#if defined(__ISOC11_SOURCE) && (_ISOC11_SOURCE)
void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((alloc_align(1)))
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
__attribute__ ((malloc))
#endif
aligned_alloc(size_t align,
              size_t size)
{
    void   *ptr = NULL;
    size_t  aln = max(align, MALLOCALIGNMENT);

#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zmemalign_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmemalign_hook(align, size, (const void *)caller);

        return ptr;
    }
#endif
    if (!powerof2(aln) || (size & (aln - 1))) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, aln, 0);
    }
#if (MALLOCDEBUG)
    _assert(ptr != NULL);
#endif
#if (MALLOCTRACE)
    __malloctrace();
    __mallocprnttrace("aligned_alloc", size, align);
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
    size_t  aln = max(align, MALLOCALIGNMENT);
    int     retval = -1;

#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
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
#if (MALLOCDEBUG)
    _assert(ptr != NULL);
#endif
    *ret = ptr;
#if (MALLOCTRACE)
    __malloctrace();
    __mallocprnttrace("posix_memalign", size, align);
#endif

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

#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zmemalign_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmemalign_hook(PAGESIZE, size, (const void *)caller);

        return ptr
    }
#endif
    ptr = _malloc(size, PAGESIZE, 0);
#if (MALLOCDEBUG)
    _assert(ptr != NULL);
#endif
#if (MALLOCTRACE)
    __malloctrace();
    __mallocprnttrace("valloc", size, 0);
#endif
    
    return ptr;
}
#endif

void *
#if defined(__GNUC__)
__attribute__ ((alloc_align(1)))
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
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
    size_t  aln = max(align, MALLOCALIGNMENT);

#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
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
#if (MALLOCDEBUG)
    _assert(ptr != NULL);
#endif
#if (MALLOCTRACE)
    __malloctrace();
    __mallocprnttrace("memalign", size, align);
#endif

    return ptr;
}

#if defined(_BSD_SOURCE)
void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
#endif
reallocf(void *ptr,
         size_t size)
{
    void *retptr;

#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
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
#if (MALLOCDEBUG)
    _assert(retptr != NULL);
#endif
#if (MALLOCTRACE)
    __malloctrace();
    if (ptr) {
        fprintf(stderr, "reallocf: %p\n", ptr);
    }
    __mallocprnttrace("reallocf", size, align);
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
    void   *ptr = _malloc(rounduppow2(size, PAGESIZE), PAGESIZE, 0);

#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zmemalign_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmemalign_hook(PAGESIZE, size, (const void *)caller);

        return ptr;
    }
#endif
#if (MALLOCDEBUG)
    _assert(ptr != NULL);
#endif
#if (MALLOCTRACE)
    __malloctrace();
    __mallocprnttrace("pvalloc", size, 0);
#endif

    return ptr;
}
#endif

#if defined(_MSVC_SOURCE)

void *
#if defined(__GNUC__)
__attribute__ ((alloc_align(2)))
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
__attribute__ ((malloc))
#endif
_aligned_malloc(size_t size,
                size_t align)
{
    void   *ptr = NULL;
    size_t  aln = max(align, MALLOCALIGNMENT);

#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
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
#if (MALLOCDEBUG)
    _assert(ptr != NULL);
#endif
#if (MALLOCTRACE)
    __malloctrace();
    __mallocprnttrace("__aligned_malloc", size, align);
#endif

    return ptr;
}

void
_aligned_free(void *ptr)
{
#if (MALLOCTRACE)
    __malloctrace();
    if (ptr) {
        fprintf(stderr, "_aligned_free: %p\n", align);
    }
    __mallocprnttrace("_aligned_free", 0, 0);
#endif
#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zfree_hook) {
        void *caller = NULL;

        m_getretadr(caller);
        __zfree_hook(ptr, (const void *)caller);

        return;
    }
#endif
    if (ptr) {
        _free(ptr);
    }

    return;
}

#endif /* _MSVC_SOURCE */

#if defined(_INTEL_SOURCE)

void *
#if defined(__GNUC__)
__attribute__ ((alloc_align(2)))
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
__attribute__ ((malloc))
#endif
_mm_malloc(int size,
           int align)
{
    void   *ptr = NULL;
    size_t  aln = max((size_t)align, MALLOCALIGNMENT);

#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
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
#if (MALLOCDEBUG)
    _assert(ptr != NULL);
#endif
#if (MALLOCTRACE)
    __malloctrace();
    __mallocprnttrace("_mm_malloc", size, align);
#endif

    return ptr;
}

void
_mm_free(void *ptr)
{
#if (MALLOCTRACE)
    __malloctrace();
    if (ptr) {
        fprintf(stderr, "_mm_malloc: %p\n", ptr);
    }
    __mallocprnttrace("_mm_malloc", 0, 0);
#endif
#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zfree_hook) {
        void *caller = NULL;

        m_getretadr(caller);
        __zfree_hook(ptr, (const void *)caller);

        return;
    }
#endif
    if (ptr) {
        _free(ptr);
    }

    return;
}

#endif /* _INTEL_SOURCE */

void
cfree(void *ptr)
{
#if (MALLOCTRACE)
    __malloctrace();
    if (ptr) {
        fprintf(stderr, "cfree: %p\n", ptr);
    }
    __mallocprnttrace("cfree", 0, 0);
#endif
    if (ptr) {
        _free(ptr);
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
                         - (((mag->lim == 1) || (mag->bktid >= PAGESIZELOG2))
                            ? 0
                            : (ptrdiff(ptr, maggetptr(mag, ptr)))))
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
#if (MALLOCHDRHACKS)
    size_t      sz = 1UL << (getnfo(ptr) & MEMHDRBKTMASK);
#elif (MALLOCHDRPREFIX)
    struct mag *mag = getmag(ptr);
    size_t      sz = (mag) ? (1UL << (mag)->bktid) : 0;
#else
    struct mag *mag = findmag(ptr);
    size_t      sz = (mag) ? (1UL << (mag)->bktid) : 0;
#endif
    
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

