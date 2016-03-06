/*
 * Zero Malloc Revision 3
 *
 * - a beta version of a new malloc for Zero.
 *
 * Copyright (C) Tuomo Petteri Venäläinen 2014-2016
 */

#undef MALLOCDIAG
#define MALLOCDIAG 0
//#undef MALLOCBUFMAG
//#define MALLOCBUFMAG 0
#define MALLOCVALGRINDTABS 1
#define MALLOCDEBUG 0
#define MALLOCTRACE 0
#define MALLOCSPINLOCKS 0

#include "_malloc.h"

/*
 * TODO
 * ----
 * - lookups;
 *   - multiples of page-size with a "v-tree" / multilevel table - DONE
 *   - smaller ones with a v-tree indexed by slab ID
 * - ...
 *
 * IMPROVEMENTS
 * ------------
 * - "real" thread-local arenas to avoid locking on the fast paths
 * - much better readability thanks to dropping testing kludges and such +
 *   splitting things into a few more functions (might work a bit more on this)
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
#if !defined(NVALGRIND)
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
 *                | thr  |
 *                --------
 *                    |
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
 *        thr
 *        ---
 *        - thread local magazine cache
 *
 *        mag
 *        ---
 *        - magazine cache with allocation stack of pointers into the slab *          - LIFO to reuse freed blocks of virtual memory
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
 * - free inactive subtables from pagedir
 */

#define MALLOCHOOKS 0
#if !defined(MALLOCDEBUGHOOKS)
#define MALLOCDEBUGHOOKS 0
#endif

#if (MALLOCTRACE)
void *tracetab[64];
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
#if (PTHREAD)
#include <pthread.h>
#endif
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/trix.h>
#if defined(MALLOCTKTLK) && (MALLOCTKTLK)
#include <zero/tktlk.h>
#endif
#if (GNUTRACE) && (MALLOCTRACE)
#include <zero/gnu.h>
#endif
#if (MALLOCDEBUGHOOKS)
#include <zero/asm.h>
#endif

#if (MALLOCVALGRIND)
#include <valgrind/valgrind.h>
#endif

void _memsetbk(void *ptr, int byte, size_t len);

//#define setptr(a, p) ((&((struct memhdr *)(a))[-1])->base = (p))

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

struct malloc                 g_malloc ALIGNED(PAGESIZE);
#if (MALLOCNBSTK)
struct bktreqs                bktreqs[MALLOCNBKT];
#endif
THREADLOCAL struct arn        thrarn ALIGNED(CLSIZE);
#if (MALLOCPRIOLK)
THREADLOCAL struct priolkdata thrpriolkdata;
#endif
//THREADLOCAL struct arn     *thrarn;
THREADLOCAL pthread_once_t    thronce;
THREADLOCAL pthread_key_t     thrkey;
THREADLOCAL long              thrflg;
#if (MALLOCSTAT)
unsigned long long            nheapbyte;
unsigned long long            nmapbyte;
unsigned long long            nhdrbyte;
unsigned long long            ntabbyte;
#endif

#if defined(__GLIBC__) && (GNUMALLOC)
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
    fprintf(stderr, "HEAP: %lld KB\tMAP: %lld KB\tTAB: %lld KB\tHDR: %lld KB\n",
            nheapbyte >> 10,
            nmapbyte >> 10,
            ntabbyte >> 10,
            nhdrbyte >> 10);
    fflush(stderr);

    return;
}
#endif

#if (MALLOCSIG)
void
mallquit(int sig)
{
    fprintf(stderr, "QUIT (%d)\n", sig);
#if (MALLOCDIAG)
    mallocdiag();
#endif
#if (MALLOCSTAT)
    mallocstat();
#else
    fflush(stderr);
#endif
    
    exit(sig);
}
#endif /* MALLOCSIG */

#if (MALLOCCASQUEUE)

#define CAS_QUEUE_ITEM_TYPE struct mag
#define CAS_QUEUE_TYPE      struct magtab
#include <zero/casqueue.h>

#endif


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

#if (MALLOCREDBLACKTREE)
struct rbtnode *
rbtgetnode(void)
{
    struct rbtnode *node;
    struct rbtnode *next;
    struct rbtnode *last;
    size_t          n = PAGESIZE / sizeof(struct rbtnode);

    do {
        node = g_malloc.rbtlist;
        if (!node) {
            node = mapanon(g_malloc.zerofd, PAGESIZE);
            next = node;
            next++;
            last = NULL;
            g_malloc.rbtlist = next;
            while (--n) {
                last = next;
                next++;
                last->right = next;
            }
            next->right = NULL;

            return node;
        }
    } while (!m_cmpswapptr(&g_malloc.rbtlist, node, node->right));

    return node;
}

void
rbtputnode(struct rbtnode *node)
{
    struct rbtnode *head;

    do {
        head = g_malloc.rbtlist;
    } while (!m_cmpswap(g_malloc.rbtlist, head, node));

    return;
}
#endif

static void
magputhdr(struct mag *mag)
{
#if (MALLOCNBSTK)
    struct arn *arn;
#endif
    long        bktid = mag->bktid;

#if (MALLOCCASQUEUE)
    magpush(mag, &g_malloc.hdrbuf[bktid]);
#elif (MALLOCNBSTK)
    arn = thrarn;
    arnpushmag(mag, &arn->hdrbuf[bktid]);
#else
    magpush(mag, &g_malloc.hdrbuf[bktid], 1);
#endif
    
    return;
}

static struct mag *
maggethdr(long bktid)
{
#if (MALLOCNBSTK)
    struct arn *arn;
#endif
    struct mag *mag;
    void       *ret;
    struct mag *hdr;
    struct mag *last;
    long        n;
    long        lim;
    size_t      sz;
    uint8_t    *ptr;

#if (MALLOCCASQUEUE)
    casqueuepop(&g_malloc.hdrbuf[bktid]);
#elif (MALLOCNBSTK)
    arn = thrarn;
    arnpopmag(&arn->hdrbuf[bktid], mag);
#else
    magpop(&g_malloc.hdrbuf[bktid], mag, 1);
#endif
    if (mag) {
        mag->bktid = bktid;
    } else {
        if (magembedtab(bktid)) {
            n = MALLOCNBUFHDR;
            sz = MALLOCHDRSIZE;
        } else {
            n = MALLOCHDRSIZE / maghdrsz();
            sz = maghdrsz();
        }
        ret = mapanon(g_malloc.zerofd, rounduppow2(n * sz, PAGESIZE));
        if (ret == MAP_FAILED) {
            
            return NULL;
        }
        mag = ret;
        ptr = ret;
        lim = n;
        mag->adr = mag;
        mag->bktid = bktid;
        last = NULL;
        while (--lim) {
            ptr += sz;
            hdr = (struct mag *)ptr;
            hdr->adr = hdr;
            hdr->bktid = bktid;
            hdr->prev = last;
            if (last) {
                last->next = hdr;
            } else {
                mag->next = hdr;
                hdr->prev = mag;
            }
            last = hdr;
        }
        hdr = mag->next;
#if (MALLOCNBSTK)
        if (hdr) {
            while (hdr) {
                arnpushmag(hdr, &arn->hdrbuf[bktid]);
                hdr = hdr->next;
            }
#if 0
            magpushmany(hdr, last, &arn->hdrbuf[bktid], 0);
#endif
        }
#else
        if (hdr) {
#if (MALLOCCASQUEUE)
            magpushmany(hdr, last, &g_malloc.hdrbuf[bktid]);
#else
            magpushmany(hdr, last, &g_malloc.hdrbuf[bktid], 1, n);
#endif
        }
#endif
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
        mag->prev = NULL;
        mag->next = NULL;
    }

    return mag;
}

static void
thrfreetls(void *arg)
{
    struct arn *arn = arg;
    struct mag *mag;
    struct mag *first;
    struct mag *tail = NULL;
    long        bktid;
    long        n = 0;

    for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
        mag = arn->magbkt[bktid].ptr;
        if (mag) {
#if (MALLOCBUFMAG)
            n++;
#endif
            first = mag;
            mag->arn = NULL;
            while (mag->next) {
                mag = mag->next;
#if (MALLOCBUFMAG)
                n++;
#endif
                mag->arn = NULL;
                tail = mag;
            }
            mag = first;
#if (MALLOCNBSTK)
            mag = first;
            while (mag) {
                magpushnb(mag, bktid);
                mag = mag->next;
            }
#else
            if (tail) {
#if (MALLOCCASQUEUE)
                magpushmany(first, tail, &g_malloc.magbkt[bktid]);
#else
                magpushmany(first, tail, &g_malloc.magbkt[bktid], 1, n);
#endif
            } else {
#if (MALLOCCASQUEUE)
                magpush(mag, &g_malloc.magbkt[bktid]);
#else
                magpush(mag, &g_malloc.magbkt[bktid], 1);
#endif
            }
#endif
        }
    }
//    unmapanon(arn, rounduppow2(sizeof(struct arn), PAGESIZE));
//    thrarn = NULL;

    return;
}

#if (MALLOCPRIOLK)
unsigned long
priogetval(void)
{
    unsigned long val;

    mtxlk(&g_malloc.priolk);
    val = g_malloc.prioval;
    val++;
    val &= 1UL << (sizeof(long) * CHAR_BIT  - 1);
    g_malloc.prioval = val;
    mtxunlk(&g_malloc.priolk);

    return val;
}
#endif

static void
thrinitarn(void)
{
#if (MALLOCPRIOLK)
    unsigned long val = priogetval();
#endif

    pthread_key_create(&thrkey, thrfreetls);
#if (MALLOCPRIOLK)
    priolkinit(&thrpriolkdata, val);
#endif

    return;
}

static void
thrinit(void)
{
    void *ptr = NULL;

    pthread_once(&thronce, thrinitarn);
#if 0
    ptr = pthread_getspecific(thrkey);
    if (!ptr) {
        ptr = mapanon(g_malloc.zerofd, rounduppow2(sizeof(struct arn),
                                                   PAGESIZE));
        if (ptr == MAP_FAILED) {
            abort();
        }
        pthread_setspecific(thrkey, ptr);
    }
#endif
    
    return;
}

static void *
maginitslab(struct mag *mag, long bktid)
{
#if (MALLOCPTRNDX)
    PTRNDX     *stk;
#else
    void      **stk;
#endif
    size_t      sz = 1UL << bktid;
    long        n = magnblk(bktid);
    uint8_t    *ptr;
#if (MALLOCPTRNDX)
    PTRNDX      ndx;
#else
    long        ndx;
#endif
    uintptr_t   magflg = 0;

    if (!mag->base) {
        ptr = SBRK_FAILED;
#if (!MALLOCNOSBRK)
        if (bktid <= MALLOCSLABLOG2 && !(g_malloc.flg & MALLOCNOHEAP)) {
            /* try to allocate slab from heap */
            mtxlk(&g_malloc.heaplk);
            ptr = growheap(rounduppow2(n * sz, PAGESIZE));
            mtxunlk(&g_malloc.heaplk);
            if (ptr == SBRK_FAILED) {
                g_malloc.flg |= MALLOCNOHEAP;
            }
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
        mag->base = ptr;
        if (n > 1) {
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
#if (MALLOCPTRNDX)
                while (n--) {
                    stk[ndx] = ndx;
                    ndx++;
                }
#else
                while (n--) {
                    stk[ndx] = ptr;
                    ndx++;
                    ptr += sz;
                }
#endif
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
#if (MALLOCPTRNDX)
            mag->idtab = NULL;
#else
            mag->ptrtab = NULL;
#endif
#if (MALLOCFREEMAP)
            mag->freemap = NULL;
#endif
        } else if (magembedtab(bktid)) {
#if (MALLOCPTRNDX)
            mag->stk = (PTRNDX *)((uint8_t *)mag + maghdrsz());
            mag->idtab = (PTRNDX *)((uint8_t *)mag->stk + n * sizeof(PTRNDX));
#else
            mag->stk = (void **)((uint8_t *)mag + maghdrsz());
            mag->ptrtab = ((uint8_t *)mag->stk + n * sizeof(void *));
#endif
#if (MALLOCFREEMAP)
            mag->freemap = ((uint8_t *)mag->stk + (n << 1) * sizeof(void *));
#endif
        } else {
            sz = magtabsz(bktid);
            ret = mapanon(g_malloc.zerofd, rounduppow2(sz, PAGESIZE));
            if (ret == MAP_FAILED) {
                
                return NULL;
            }
#if (MALLOCPTRNDX)
            mag->stk = (PTRNDX *)ret;
            mag->idtab = (PTRNDX *)((uint8_t *)mag->stk + n * sizeof(void *));
#else
            mag->stk = ret;
            mag->ptrtab = ((uint8_t *)mag->stk + n * sizeof(void *));
#endif
#if (MALLOCFREEMAP)
            mag->freemap = ((uint8_t *)mag->stk + (n << 1) * sizeof(void *));
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
    if (!mag->stk) {
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

    mtxlk(&g_malloc.initlk);
    mtxlk(&g_malloc.heaplk);
    for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
#if (!MALLOCNBSTK)
        __malloclk(&g_malloc.magbkt[ndx].lk);
        __malloclk(&g_malloc.hdrbuf[ndx].lk);
#endif
//        __malloclk(&g_malloc.freetab[ndx].lk);
    }
#if (MALLOCMULTITAB)
    for (ndx = 0 ; ndx < PAGEDIRNL1KEY ; ndx++) {
#if (MALLOCSPINLOCKS)
        __malloclkspin(&g_malloc.pagedirlktab[ndx]);
        __malloclkspin(&g_malloc.slabdirlktab[ndx]);
#else
        __malloclk(&g_malloc.pagedirlktab[ndx]);
        __malloclk(&g_malloc.slabdirlktab[ndx]);
#endif
    }
#endif
    
    return;
}

static void
postfork(void)
{
    long ndx;
    
#if (MALLOCMULTITAB)
    for (ndx = 0 ; ndx < PAGEDIRNL1KEY ; ndx++) {
#if (MALLOCSPINLOCKS)
        __mallocunlkspin(&g_malloc.pagedirlktab[ndx]);
#else
        __mallocunlk(&g_malloc.pagedirlktab[ndx]);
#endif
    }
    for (ndx = 0 ; ndx < SLABDIRNL1KEY ; ndx++) {
#if (MALLOCSPINLOCKS)
        __mallocunlkspin(&g_malloc.slabdirlktab[ndx]);
#else
        __mallocunlk(&g_malloc.slabdirlktab[ndx]);
#endif
    }
#endif
#if (!MALLOCNBSTK)
    for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
        __mallocunlk(&g_malloc.hdrbuf[ndx].lk);
        __mallocunlk(&g_malloc.magbkt[ndx].lk);
    }
#endif
    mtxunlk(&g_malloc.heaplk);
    mtxunlk(&g_malloc.initlk);
    
    return;
}

#if (MALLOCHASH)

#define MALLOC_HASH_MARK_POS 0
#define MALLOC_HASH_MARK_BIT (1L << 0)

static struct mag *
magfind(void *ptr)
{
    uintptr_t     uptr = (uintptr_t)ptr;
    struct mag   *mag;
    unsigned long key;

    key = hashq128uptr(uptr, MALLOCNHASHBIT);
}

#elif (MALLOCMULTITAB)

#if defined(MALLOCFREETABS) && (MALLOCFREETABS)

static struct mag *
mtfindmag(void *ptr, long type)
{
    uintptr_t      l1;
    uintptr_t      l2;
    uintptr_t      l3;
    struct memtab *mptr1;
    struct memtab *mptr2;
    struct mag    *mag = NULL;
    
    if (type == MALLOCPAGETAB) {
        l1 = pagedirl1ndx(ptr);
        l2 = pagedirl2ndx(ptr);
        l3 = pagedirl3ndx(ptr);
#if (MALLOCSPINLOCKS)
        __malloclkspin(&g_malloc.pagedirlktab[l1]);
#else
        __malloclk(&g_malloc.pagedirlktab[l1]);
#endif
        mptr1 = g_malloc.pagedir[l1].ptr;
        if (mptr1) {
            mptr2 = mptr1->ptr;
            if (mptr2) {
                mptr1 = &mptr2[l2];
                if (mptr1) {
                    mptr2 = mptr1->ptr;
                    if (mptr2) {
                        mag = ((void **)mptr2)[l3];
                    }
                }
            }
        }
#if (MALLOCSPINLOCKS)
        __mallocunlkspin(&g_malloc.pagedirlktab[l1]);
#else
        __mallocunlk(&g_malloc.pagedirlktab[l1]);
#endif
    } else {
        l1 = slabdirl1ndx(ptr);
        l2 = slabdirl2ndx(ptr);
        l3 = slabdirl3ndx(ptr);
#if (MALLOCSPINLOCKS)
        __malloclkspin(&g_malloc.slabdirlktab[l1]);
#else
        __malloclk(&g_malloc.slabdirlktab[l1]);
#endif
        mptr1 = g_malloc.slabdir[l1].ptr;
        if (mptr1) {
            mptr2 = mptr1->ptr;
            if (mptr2) {
                mptr1 = &mptr2[l2];
                if (mptr1) {
                    mptr2 = mptr1->ptr;
                    if (mptr2) {
                        mag = ((void **)mptr2)[l3];
                    }
                }
            }
        }
#if (MALLOCSPINLOCKS)
        __mallocunlkspin(&g_malloc.slabdirlktab[l1]);
#else
        __mallocunlk(&g_malloc.slabdirlktab[l1]);
#endif
    }

    return mag;
}
    
static void
mtsetmag(void *ptr,
         struct mag *mag,
         long type)
{
    uintptr_t       l1;
    uintptr_t       l2;
    uintptr_t       l3;
    long            fail = 0;
    struct memtab  *mptr1;
    struct memtab  *mptr2;
    struct memtab  *ptab[3] = { NULL, NULL, NULL };
    
    if (type == MALLOCPAGETAB) {
        l1 = pagedirl1ndx(ptr);
        l2 = pagedirl2ndx(ptr);
        l3 = pagedirl3ndx(ptr);
#if (MALLOCSPINLOCKS)
        __malloclkspin(&g_malloc.pagedirlktab[l1]);
#else
        __malloclk(&g_malloc.pagedirlktab[l1]);
#endif
        if (mag) {
            mptr1 = g_malloc.pagedir[l1].ptr;
            if (!mptr1) {
                mptr1 = mapanon(g_malloc.zerofd,
                                PAGEDIRNL2KEY * sizeof(struct memtab));
                if (mptr1 != MAP_FAILED) {
                    ptab[0] = mptr1;
                    g_malloc.pagedir[l1].ptr = (void *)mptr1;
                    mptr1->nref++;
#if defined(MALLOCVALGRINDTABS)
                    VALGRINDALLOC(mptr1,
                                  PAGEDIRNL2KEY * sizeof(struct memtab), 1);
#endif
#if (MALLOCSTAT)
                    ntabbyte += PAGEDIRNL2KEY * sizeof(struct memtab);
#endif
                    if (!fail) {
                        mptr2 = mptr1->ptr;
                        if (!mptr2) {
                            mptr2 = mapanon(g_malloc.zerofd,
                                            PAGEDIRNL3KEY * sizeof(void *));
#if (MALLOCSTAT)
                            ntabbyte += PAGEDIRNL3KEY * sizeof(void *);
#endif
                        }
                        if (mptr2 != MAP_FAILED) {
                            ptab[1] = mptr2;
                            mptr1->ptr = mptr2;
                            mptr1->nref++;
#if defined(MALLOCVALGRINDTABS)
                            VALGRINDALLOC(mptr2,
                                          PAGEDIRNL3KEY * sizeof(void *), 1);
#endif
#if (MALLOCSTAT)
                            ntabbyte += PAGEDIRNL3KEY * sizeof(struct memtab);
#endif
                        } else {
                            fail = 1;
                        }
                    }
                }
            }
        }
        if (!mag || !fail) {
            mptr1 = g_malloc.pagedir[l1].ptr;
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
                            }
                        }
                    }
                }
            }
        }
        if (!mag || (fail)) {
            mptr1 = ptab[1];
            mptr2 = ptab[0];
            if ((mptr1) && (mptr2)) {
                if (!--mptr2->nref) {
                    unmapanon(mptr1, PAGEDIRNL3KEY * sizeof(void *));
#if defined(MALLOCVALGRINDTABS)
                    VALGRINDFREE(mptr1);
#endif
#if (MALLOCSTAT)
                    ntabbyte -= PAGEDIRNL3KEY * sizeof(void *);
#endif
                }
            }
            mptr1 = &g_malloc.pagedir[l1];
            if ((mptr2) && (mptr1)) {
                if (!--mptr1->nref) {
                    unmapanon(mptr2, PAGEDIRNL2KEY * sizeof(struct memtab));
#if defined(MALLOCVALGRINDTABS)
                    VALGRINDFREE(mptr2);
#endif
#if (MALLOCSTAT)
                    ntabbyte -= PAGEDIRNL2KEY * sizeof(struct memtab);
#endif
                    g_malloc.pagedir[l1].ptr = NULL;
                }
            }
        }
#if (MALLOCSPINLOCKS)
        __mallocunlkspin(&g_malloc.pagedirlktab[l1]);
#else
        __mallocunlk(&g_malloc.pagedirlktab[l1]);
#endif
    } else {
        /* type != MALLOCPAGETAB */
        l1 = slabdirl1ndx(ptr);
        l2 = slabdirl2ndx(ptr);
        l3 = slabdirl3ndx(ptr);
#if (MALLOCSPINLOCKS)
        __malloclkspin(&g_malloc.slabdirlktab[l1]);
#else
        __malloclk(&g_malloc.slabdirlktab[l1]);
#endif
        if (mag) {
            mptr1 = g_malloc.slabdir[l1].ptr;
            if (!mptr1) {
                mptr1 = mapanon(g_malloc.zerofd,
                                SLABDIRNL2KEY * sizeof(struct memtab));
                if (mptr1 != MAP_FAILED) {
                    ptab[0] = mptr1;
                    g_malloc.slabdir[l1].ptr = (void *)mptr1;
                    mptr1->nref++;
#if defined(MALLOCVALGRINDTABS)
                    VALGRINDALLOC(mptr1,
                                  SLABDIRNL2KEY * sizeof(struct memtab), 1);
#endif
#if (MALLOCSTAT)
                    ntabbyte += SLABDIRNL2KEY * sizeof(struct memtab);
#endif
                    if (!fail) {
                        mptr2 = mptr1->ptr;
                        if (!mptr2) {
                            mptr2 = mapanon(g_malloc.zerofd,
                                            SLABDIRNL3KEY * sizeof(void *));
#if (MALLOCSTAT)
                            ntabbyte += SLABDIRNL3KEY * sizeof(void *);
#endif
                        }
                        if (mptr2 != MAP_FAILED) {
                            ptab[1] = mptr2;
                            mptr1->ptr = mptr2;
                            mptr1->nref++;
#if defined(MALLOCVALGRINDTABS)
                            VALGRINDALLOC(mptr2,
                                          SLABDIRNL3KEY * sizeof(void *), 1);
#endif
#if (MALLOCSTAT)
                            ntabbyte += SLABDIRNL3KEY * sizeof(struct memtab);
#endif
                        } else {
                            fail = 1;
                        }
                    }
                }
            }
        }
        if (!mag || !fail) {
            mptr1 = g_malloc.slabdir[l1].ptr;
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
                            }
                        }
                    }
                }
            }
        }
        if (!mag || (fail)) {
            mptr1 = ptab[1];
            mptr2 = ptab[0];
            if ((mptr1) && (mptr2)) {
                if (!--mptr2->nref) {
                    unmapanon(mptr1, SLABDIRNL3KEY * sizeof(void *));
#if defined(MALLOCVALGRINDTABS)
                    VALGRINDFREE(mptr1);
#endif
#if (MALLOCSTAT)
                    ntabbyte -= SLABDIRNL3KEY * sizeof(void *);
#endif
                }
            }
            mptr1 = &g_malloc.slabdir[l1];
            if ((mptr2) && (mptr1)) {
                if (!--mptr1->nref) {
                    unmapanon(mptr2, SLABDIRNL2KEY * sizeof(struct memtab));
#if defined(MALLOCVALGRINDTABS)
                    VALGRINDFREE(mptr2);
#endif
#if (MALLOCSTAT)
                    ntabbyte -= SLABDIRNL2KEY * sizeof(struct memtab);
#endif
                    g_malloc.slabdir[l1].ptr = NULL;
                }
            }
        }
#if (MALLOCSPINLOCKS)
        __mallocunlkspin(&g_malloc.slabdirlktab[l1]);
#else
        __mallocunlk(&g_malloc.slabdirlktab[l1]);
#endif
    }
    
    return;
}

#else /* !MALLOCFREETABS */

static void
mtsetmag(void *ptr,
         struct mag *mag,
         long type)
{
    void      *ptr1;
    void      *ptr2;
    uintptr_t  l1 = pagedirl1ndx(ptr);
    uintptr_t  l2 = pagedirl2ndx(ptr);
    uintptr_t  l3 = pagedirl3ndx(ptr);
    
    if (type == MALLOCPAGETAB) {
#if (MALLOCSPINLOCKS)
        __malloclkspin(&g_malloc.pagedirlktab[l1]);
#else
        __malloclk(&g_malloc.pagedirlktab[l1]);
#endif
        if (!mag) {
            ptr1 = g_malloc.pagedir[l1];
            if (ptr1) {
                ptr2 = ((void **)ptr1)[l2];
#if defined(PAGEDIRNL3BIT) && (PAGEDIRNL3BIT)
                if (ptr2) {
                    ((void **)ptr2)[l3] = mag;
                }
#else
                if (ptr2) {
                    *((void **)ptr2) = mag;
                }
#endif
            }
        } else {
            ptr1 = g_malloc.pagedir[l1];
            if (!ptr1) {
                ptr1 = mapanon(g_malloc.zerofd,
                               PAGEDIRNL2KEY * sizeof(void *));
                if (ptr1 == MAP_FAILED) {
                    abort();
                }
#if (MALLOCSTAT)
                ntabbyte += PAGEDIRNL2KEY * sizeof(void *);
#endif
                g_malloc.pagedir[l1] = ptr1;
            }
            if (ptr1) {
                ptr2 = ((void **)ptr1)[l2];
                if (!ptr2) {
                    ptr2 = mapanon(g_malloc.zerofd,
                                   PAGEDIRNL3KEY * sizeof(void *));
                    if (ptr2 == MAP_FAILED) {
                        abort();
                    }
#if (MALLOCSTAT)
                    ntabbyte += PAGEDIRNL3KEY * sizeof(void *);
#endif
                    ((void **)ptr1)[l2] = ptr2;
                }
                if (ptr2) {
                    ((void **)ptr2)[l3] = mag;
                }
            }
#if (MALLOCSPINLOCKS)
            __mallocunlkspin(&g_malloc.pagedirlktab[l1]);
#else
            __mallocunlk(&g_malloc.pagedirlktab[l1]);
#endif
        }
    }
    
    return;
}

static struct mag *
mtfindmag(void *ptr)
{
    uintptr_t      l1 = pagedirl1ndx(ptr);
    uintptr_t      l2 = pagedirl2ndx(ptr);
    uintptr_t      l3 = pagedirl3ndx(ptr);
#if (MALLOCFREETABS)
    struct memtab *mptr1;
    struct memtab *mptr2;
#else
    void          *ptr1;
    void          *ptr2;
#endif
    struct mag    *mag = NULL;
    
#if (MALLOCSPINLOCKS)
    __malloclkspin(&g_malloc.pagedirlktab[l1]);
#else
    __malloclk(&g_malloc.pagedirlktab[l1]);
#endif
    ptr1 = g_malloc.pagedir[l1];
    if (ptr1) {
        ptr2 = ((void **)ptr1)[l2];
        if (ptr2) {
#if !defined(PAGEDIRNL4BIT) || (!PAGEDIRNL4BIT)
            mag = ((void **)ptr2)[l3];
#else
            ptr1 = ((void **)ptr2)[l3];
            if (ptr1) {
                mag = ((void **)ptr1)[l4];
            }
#endif
        }
    }
#if (MALLOCSPINLOCKS)
    __mallocunlkspin(&g_malloc.pagedirlktab[l1]);
#else
    __mallocunlk(&g_malloc.pagedirlktab[l1]);
#endif

    return mag;
}

#endif

#endif /* MALLOCMULTITAB */

#if (MALLOCDIAG)
static void
mallocdiag(void)
{
    struct arn *arn= thrarn;
    struct mag *mag1;
    struct mag *mag2;
    long        ndx;

    if (arn) {
        for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
            mag1 = arn->magbkt[ndx].ptr;
            if (mag1) {
                if (mag1->arn != arn) {
                    fprintf(stderr, "invalid arn on arn list %ld ptr\n",
                            ndx);
                    magprint(mag1);

                    abort();
                } else if (mag1->prev) {
                    fprintf(stderr, "invalid prev on arn list %ld ptr\n",
                            ndx);
                    magprint(mag1);
                    
                    abort();
                } else if (mag1->cur >= mag1->lim) {
                    fprintf(stderr, "too high count on arn list %ld ptr\n");
                    magprint(mag1);

                    abort();
                }
                while (mag1->next) {
                    mag2 = mag1->next;
                    if (mag2->arn != arn) {
                        fprintf(stderr, "invalid arn on arn list %ld\n",
                                ndx);
                        magprint(mag2);
                        
                        abort();
                    } else if (mag2->prev != mag1) {
                        fprintf(stderr, "invalid prev on arn list %ld\n",
                                ndx);
                        magprint(mag1);
                        magprint(mag2);
                        
                        abort();
                    } else if (mag2->cur >= mag2->lim) {
                        fprintf(stderr, "too high count on arn list %ld\n");
                        magprint(mag1);
                        
                        abort();
                    }
                }
            }
        }
    }
    for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
        __malloclk(&g_malloc.magbkt[ndx].lk);
        mag1 = g_malloc.magbkt[ndx].ptr;
        if (mag1) {
            if (mag1->arn != NULL) {
                fprintf(stderr, "invalid arn on globa list %ld ptr\n",
                        ndx);
                magprint(mag1);
                
                abort();
            } else if (mag1->prev) {
                fprintf(stderr, "invalid prev on global list %ld ptr\n",
                        ndx);
                magprint(mag1);
                
                abort();
            } else if (mag1->cur >= mag1->lim) {
                fprintf(stderr, "too high count on global list %ld ptr\n");
                magprint(mag1);
                
                abort();
            }
            while (mag1->next) {
                mag2 = mag1->next;
                if (mag2->arn != NULL) {
                    fprintf(stderr, "invalid arn on global list %ld\n",
                            ndx);
                    magprint(mag2);
                    
                    abort();
                } else if (mag2->prev != mag1) {
                    fprintf(stderr, "invalid prev on global list %ld\n",
                            ndx);
                    magprint(mag1);
                    magprint(mag2);
                    
                        abort();
                } else if (mag2->cur >= mag2->lim) {
                    fprintf(stderr, "too high count on global list %ld\n");
                    magprint(mag1);
                    
                    abort();
                }
            }
        }
        __mallocunlk(&g_malloc.magbkt[ndx].lk);
    }

    return;
}
#endif /* MALLOCDIAG */

#if (GNUMALLOC)
void
#else
static void
#endif
mallinit(void)
{
    long  ndx;
#if (!MALLOCNOSBRK)
    void *heap;
    long  ofs;
#endif
#if (MALLOCNBSTK)
    void *ptr;
#endif

    mtxlk(&g_malloc.initlk);
    if (g_malloc.flg & MALLOCINIT) { 
        mtxunlk(&g_malloc.initlk);
       
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
    ndx = MALLOCNBKT;
    while (ndx--) {
#if (!MALLOCNBSTK) && (!MALLOCTKTLK)
#if (MALLOCSPINLOCKS)
        __mallocinitspin(&g_malloc.hdrbuf[ndx].lk);
#else
        __mallocinitlk(&g_malloc.hdrbuf[ndx].lk);
#endif
        __mallocinitlk(&g_malloc.magbkt[ndx].lk);
#endif
//        __mallocinitlk(&g_malloc.freetab[bktid].lk);
    }
#if (!MALLOCNOSBRK)
    mtxlk(&g_malloc.heaplk);
    heap = growheap(0);
    ofs = (1UL << PAGESIZELOG2) - ((long)heap & (PAGESIZE - 1));
    if (ofs != PAGESIZE) {
        growheap(ofs);
    }
    mtxunlk(&g_malloc.heaplk);
#endif /* !MALLOCNOSBRK */
#if (MALLOCREDBLACKTREE)
    rbtinit(&g_malloc.ptrtree);
#elif (MALLOCMULTITAB)
    g_malloc.pagedirlktab = mapanon(g_malloc.zerofd, PAGEDIRNL1KEY
                                    * sizeof(LOCK));
    if (g_malloc.pagedirlktab == MAP_FAILED) {
        abort();
    }
    ndx = PAGEDIRNL1KEY;
#if (MALLOCSTAT)
    ntabbyte += PAGEDIRNL1KEY * sizeof(LOCK);
#endif
    while (ndx--) {
#if (MALLOCSPINLOCKS)
        __mallocinitspin(&g_malloc.pagedirlktab[ndx]);
#else
        __mallocinitlk(&g_malloc.pagedirlktab[ndx]);
#endif
    }
    g_malloc.slabdirlktab = mapanon(g_malloc.zerofd, SLABDIRNL1KEY
                                    * sizeof(LOCK));
    if (g_malloc.slabdirlktab == MAP_FAILED) {
        abort();
    }
    ndx = SLABDIRNL1KEY;
#if (MALLOCSTAT)
    ntabbyte += SLABDIRNL1KEY * sizeof(LOCK);
#endif
    while (ndx--) {
#if (MALLOCSPINLOCKS)
        __mallocinitspin(&g_malloc.slabdirlktab[ndx]);
#else
        __mallocinitlk(&g_malloc.slabdirlktab[ndx]);
#endif
    }
#if (MALLOCFREETABS)
    g_malloc.pagedir = mapanon(g_malloc.zerofd,
                               PAGEDIRNL1KEY * sizeof(struct memtab));
    if (g_malloc.pagedir == MAP_FAILED) {
        abort();
    }
    g_malloc.slabdir = mapanon(g_malloc.zerofd,
                               SLABDIRNL1KEY * sizeof(struct memtab));
    if (g_malloc.slabdir == MAP_FAILED) {
        abort();
    }
#if (MALLOCSTAT)
    ntabbyte += PAGEDIRNL1KEY * sizeof(struct memtab);
    ntabbyte += SLABDIRNL1KEY * sizeof(struct memtab);
#endif
#else /* !MALLOCFREETABS */
    g_malloc.pagedir = mapanon(g_malloc.zerofd,
                               PAGEDIRNL1KEY * sizeof(void *));
    if (g_malloc.pagedir == MAP_FAILED) {
        abort();
    }
    g_malloc.slabdir = mapanon(g_malloc.zerofd,
                               SLABDIRNL1KEY * sizeof(void *));
    if (g_malloc.slabdir == MAP_FAILED) {
        abort();
    }
#if (MALLOCSTAT)
    ntabbyte += PAGEDIRNL1KEY * sizeof(void *);
    ntabbyte += SLABDIRNL1KEY * sizeof(void *);
#endif
#endif
#endif /* MALLOCMULTITAB */
#if (MALLOCNBSTK)
    ndx = MALLOCNBKT;
    while (ndx--) {
        struct bktreq *tab;

        ptr = mapanon(g_malloc.zerofd, rounduppow2(BKTNREQ * sizeof(void *),
                                                   PAGESIZE));
        if (ptr == MAP_FAILED) {
            abort();
        }
        tab = mapanon(g_malloc.zerofd,
                      rounduppow2(BKTNREQ * sizeof(struct bktreq),
                                  PAGESIZE));
        if (tab == MAP_FAILED) {
            abort();
        }
        bktreqs[ndx].stk = ptr;
        bktreqs[ndx].tab = tab;
    }
#endif
#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zmalloc_initialize_hook) {
        __zmalloc_initialize_hook();
    }
#endif
    pthread_atfork(prefork, postfork, postfork);
    g_malloc.flg |= MALLOCINIT;
    mtxunlk(&g_malloc.initlk);
    
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
#if (MALLOCPTRNDX)
    PTRNDX      ndx;
#endif
    struct mag *mag = NULL;
    struct arn *arn;
    uint8_t    *adr;
    uint8_t    *ptr = NULL;
    size_t      sz = ((size < PAGESIZE)
                      ? ((align)
                         ? (size + align)
                         : (size + MALLOCALIGNMENT))
                      : (size
                         + ((align <= PAGESIZE)
                            ? 0
                            : rounduppow2(size + align, PAGESIZE))));
    long        bktid = blkbktid(sz);
    long        lim;
#if (MALLOCHDRHACKS)
    uint8_t     nfo = bktid;
#endif
    
    if (!(g_malloc.flg & MALLOCINIT)) {
        mallinit();
    }
    arn = &thrarn;
    if (!(thrflg & MALLOCINIT)) {
        thrinit();
        thrflg |= MALLOCINIT;
    }
//    align = max(align, MALLOCALIGNMENT);
    /* try to allocate from a partially used magazine */
    if (arn) {
        mag = arn->magbkt[bktid].ptr;
        if (mag) {
            lim = mag->lim;
            if (lim == 1) {
                ptr = mag->base;
                mag->cur = 1;
            } else {
#if (MALLOCPTRNDX)
                ndx = mag->stk[mag->cur++];
                ptr = magptr(mag, ndx);
#else
                ptr = ((void **)mag->stk)[mag->cur++];
#endif
            }
            if (mag->cur == lim) {
#if (MALLOCBUFMAG)
                arn->magbkt[bktid].n--;
#endif
#if (MALLOCNBSTK) || (MALLOCCASQUEUE)
                arnrmhead(&arn->magbkt[bktid], mag);
#else
                magrmhead(&arn->magbkt[bktid], mag);
#endif
                mag->prev = NULL;
                mag->next = NULL;
            }
        }
    }
    if (!mag) {
#if (MALLOCNBSTK)
        mag = magpopnb(bktid);
#elif (MALLOCCASQUEUE)
        mag = magpop(&g_malloc.magbkt[bktid]);
#else /* !MALLOCNBSTK */
        magpop(&g_malloc.magbkt[bktid], mag, 1);
#endif
        if (!mag) {
            mag = magget(bktid);
        }
        if (mag) {
            lim = mag->lim;
            if (lim == 1) {
                ptr = mag->base;
                mag->cur = 1;
            } else {
#if (MALLOCPTRNDX)
                ndx = mag->stk[mag->cur++];
                ptr = magptr(mag, ndx);
#else
                ptr = ((void **)mag->stk)[mag->cur++];
#endif
            }
            if (mag->cur < lim) {
                mag->arn = arn;
#if (MALLOCBUFMAG)
                arn->magbkt[bktid].n++;
#endif
#if (MALLOCNBSTK) || (MALLOCCASQUEUE)
                arnpushmag(mag, &arn->magbkt[bktid]);
#else
                magpush(mag, &arn->magbkt[bktid], 0);
#endif /* MALLOCNBSTK */
            } else {
                mag->next = NULL;
            }
        }
    }
    if (ptr) {
        adr = clrptr(ptr);
        if (zero) {
//        memset(adr, 0, size);
            _memsetbk(adr, 0, sz);
        } else if (g_malloc.mallopt.flg & MALLOPT_PERTURB_BIT) {
            int perturb = g_malloc.mallopt.perturb;
            
            perturb = (~perturb) & 0xff;
//        memset(adr, perturb, 1UL << bktid);
            _memsetbk(adr, perturb, sz);
        }
#if (MALLOCREDBLACKTREE)
        if (align > sz) {
            /* store unaligned source pointer and mag address */
            if (align) {
                if ((uintptr_t)ptr & (align - 1)) {
                    ptr = ptralign(ptr, align);
                }
            }
        }
        rbtinsert(&g_malloc.ptrtree, (uintptr_t)ptr, (uintptr_t)mag);
#elif (MALLOCHDRPREFIX)
        if ((sz < (PAGESIZE >> 1)) || (align > PAGESIZE)) {
            /* store unaligned source pointer and mag address */
            ptr += max(align, MALLOCALIGNMENT);
#if (MALLOCSTAT)
            if (!align) {
                nhdrbyte += MALLOCALIGNMENT;
            }
#endif
            if (align) {
                if ((uintptr_t)ptr & (align - 1)) {
                    ptr = ptralign(ptr, align);
                }
            }
            setmag(ptr, mag);
        } else {
            mtsetmag(ptr, mag, MALLOCPAGETAB);
        }
#else /* !MALLOCREDBLACKTREE && !MALLOCHDRPREFIX */
        if (align > sz) {
            /* store unaligned source pointer and mag address */
            if (align) {
                if ((uintptr_t)ptr & (align - 1)) {
                    ptr = ptralign(ptr, align);
                }
            }
            mtsetmag(ptr, mag, MALLOCSLABTAB);
        } else {
            mtsetmag(ptr, mag, MALLOCPAGETAB);
        }
#endif
#if (MALLOCPTRNDX)
        magputid(mag, ptr, ndx);
#else
        magputptr(mag, ptr, adr);
#endif
#if (MALLOCHDRHACKS)
        setnfo(ptr, nfo);
#endif
#if (MALLOCFREEMAP)
        mtxlk(&mag->freelk);
        if (((mag->lim == 1) && (mag->freemap))
            || ((mag->lim > 1) && bitset(mag->freemap, magptrid(mag, adr)))) {
            magprint(mag);
            fprintf(stderr, "trying to reallocate block");
            fflush(stderr);
            
            abort();
        } else if (mag->lim == 1) {
            mag->freemap = (void *)(~(uintptr_t)NULL);
        } else {
            setbit(mag->freemap, magptrid(mag, adr));
        }
        mtxunlk(&mag->freelk);
#endif
        VALGRINDALLOC(ptr,
                      size,
                      zero);
#if (MALLOCDIAG)
        mallocdiag();
#endif
#if defined(ENOMEM)
    } else {
        errno = ENOMEM;
#endif
    }
    
    return ptr;
}

void
_free(void *ptr)
{
    struct arn     *arn;
#if (MALLOCREDBLACKTREE)
    struct rbtnode *rbt;
#endif
    struct mag     *mag = NULL;
    void           *adr = NULL;
    long            bktid;
    long            lim;
#if (MALLOCFREEMAP)
    long            ndx;
#endif
#if (MALLOCHDRHACKS)
    uint8_t         nfo = 0;
#endif
#if (MALLOCPTRNDX)
    PTRNDX          ndx;
#endif
#if (MALLOCNBSTK)
    long            pos;
#endif
    
    if (!ptr) {

        return;
    }
    arn = &thrarn;
    if (!(thrflg & MALLOCINIT)) {
        thrinit();
        thrflg |= MALLOCINIT;
    }
#if 0
    if (!thrarn) {
        arn = thrinit();
        thrarn = arn;
    }
#endif
#if (MALLOCREDBLACKTREE)
    rbt = rbtfind(&g_malloc.ptrtree, (uintptr_t)ptr);
    if (rbt) {
        mag = (struct mag *)rbtclrcolor(rbt);
        rbtdelete(&g_malloc.ptrtree, (uintptr_t)ptr);
    }
#elif (MALLOCHDRPREFIX)
    if ((uintptr_t)ptr & (PAGESIZE - 1)) {
        mag = getmag(ptr);
    } else {
        mag = mtfindmag(ptr, MALLOCPAGETAB);
    }
#if (MALLOCHDRHACKS)
    nfo = getnfo(ptr);
#endif
#else /* !MALLOCREDBLACKTREE && !MALLOCHDRPREFIX */
    mag = mtfindmag(ptr, MALLOCSLABTAB);
    if (!mag) {
        mag = mtfindmag(ptr, MALLOCPAGETAB);
    }
#endif
    if (mag) {
#if (MALLOCNBSTK)
        if (mag->lim > 1) {
            pos = m_fetchadd(&mag->cur, -1);
        } else {
            pos = 0;
        }
#endif
#if (!MALLOCREDBLACKTREE)
        if ((uintptr_t)ptr & (PAGESIZE - 1)) {
            setmag(ptr, NULL);
        } else {
            mtsetmag(ptr, NULL, MALLOCPAGETAB);
        }
#endif
#if (MALLOCHDRHACKS)
        bktid = nfo & MEMHDRBKTMASK;
#else
        bktid = mag->bktid;
#endif
#if (MALLOCPTRNDX)
        ndx = magptrid(mag, ptr);
#elif (MALLOCHDRHACKS)
        if (!(nfo & MEMHDRALNBIT)) {
            adr = (void *)rounddownpow2((uintptr_t)ptr, 1UL << bktid);
        } else if (mag->lim == 1) {
            adr = mag->base;
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
            adr = mag->base;
        } else {
            adr = maggetptr(mag, ptr);
        }
#endif /* MALLOCPTRNDX */
        VALGRINDFREE(ptr);
#if (MALLOCFREEMAP)
        /* FIXME: use m_cmpclrbit() */
        ndx = magptrid(mag, adr);
        mtxlk(&mag->freelk);
        if ((mag->lim == 1 && (mag->freemap))
            || ((mag->lim > 1) && !bitset(mag->freemap, ndx))) {
            magprint(mag);
            fprintf(stderr, "trying to free an unused block\n");
            
            abort();
        } else if (mag->cur == 1) {
            mag->freemap = (void *)(~(uintptr_t)NULL);
        } else {
            clrbit(mag->freemap, magptrid(mag, adr));
        }
        mtxunlk(&mag->freelk);
#endif
#if 0
        if (g_malloc.mallopt.flg & MALLOPT_PERTURB_BIT) {
            int perturb = g_malloc.mallopt.perturb;
            
            perturb &= 0xff;
            memset(adr, perturb, sz);
        }
#endif
#if (MALLOCDIAG)
        if (mag->cur > mag->lim) {
            mallocdiag();
        }
#endif
        if (mag->lim == 1) {
            mag->ptr = adr;
        } else {
#if (MALLOCPTRNDX)
            ndx = magptrid(mag, ptr);
#if (MALLOCNBSTK)
            mag->stk[pos] = ndx;
#else
            mag->stk[--mag->cur] = ndx;
#endif
#else
#if (MALLOCNBSTK)
            ((void **)mag->stk)[pos] = adr;
#else
            ((void **)mag->stk)[--mag->cur] = adr;
#endif
#endif
        }
        lim = mag->lim;
        if (!mag->cur) {
            if (lim > 1) {
#if (MALLOCCASQUEUE)
                arnrmmag(mag);
//                magrm(mag, &g_malloc.magbkt[bktid], 1);
#elif (MALLOCNBSTK)
                magrmnb(mag, bktid);
#else
                magrm(mag, &g_malloc.magbkt[bktid], 1);
#endif
            }
            if (arn->magbkt[bktid].n < magnarnbuf(bktid)) {
                mag->arn = arn;
#if (MALLOCNBSTK) || (MALLOCCASQUEUE)
                arnpushmag(mag, &arn->magbkt[bktid]);
#else
                magpush(mag, &arn->magbkt[bktid], 0);
#endif
            } else if (g_malloc.magbkt[bktid].n < magnglobbuf(bktid)) {
#if (MALLOCNBSTK)
                magpushnb(mag, bktid);
#elif (MALLOCCASQUEUE)
                magpush(mag, &g_malloc.magbkt[bktid]);
#else
                magpush(mag, &g_malloc.magbkt[bktid], 1);
#endif
            } else if ((uintptr_t)mag->adr & MAGMAP) {
                /* unmap slab */
                adr = (void *)mag->base;
                VALGRINDRMPOOL(adr);
                if ((mag->lim > 1) && !magembedtab(bktid)) {
                    unmapanon(mag->stk, rounduppow2(magtabsz(bktid),
                                                    PAGESIZE));
#if (MALLOCSTAT)
                    ntabbyte -= magtabsz(bktid);
#endif
                }
                unmapanon(adr, mag->size);
#if (MALLOCSTAT)
                nmapbyte -= mag->size;
#endif
                mag->base = NULL;
                mag->adr = NULL;
                mag->ptr = NULL;
                mag->bktid = 0;
                mag->size = 0;
                mag->cur = 0;
                mag->lim = 0;
                if (!magembedtab(bktid)) {
                    mag->stk = NULL;
#if (MALLOCPTRNDX)
                    mag->idtab = NULL;
#else
                    mag->ptrtab = NULL;
#endif
#if (MALLOCFREEMAP)
                    mag->freemap = NULL;
#endif
                }
                magputhdr(mag);
            } else {
                mag->arn = arn;
#if (MALLOCNBSTK) || (MALLOCCASQUEUE)
                arnpushmag(mag, &arn->magbkt[bktid]);
#else
                magpush(mag, &arn->magbkt[bktid], 0);
#endif
            }
        } else if (mag->cur == lim - 1) {
                /* queue an unqueued earlier fully allocated magazine */
#if (MALLOCCASQUEUE)
            arnpushtail(mag, &arn->magbkt[bktid]);
#elif (MALLOCNBSTK)
#if (MALLOCNBTAIL)
            magpushtailnb(mag, bktid);
#else
            magpushnb(mag, bktid);
#endif
#else
            magpush(mag, &g_malloc.magbkt[bktid], 1);
#endif
        }
    }
#if (MALLOCDIAG)
    mallocdiag();
#endif
    
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
    uint8_t        *retptr = NULL;
    size_t          sz = 1UL << (blkbktid(size + MALLOCALIGNMENT));
#if (MALLOCREDBLACKTREE)
    struct rbtnode *rbt = rbtfind(&g_malloc.ptrtree, (uintptr_t)ptr);
    struct mag     *mag = (struct mag *)rbtclrcolor(rbt);
#else
    struct mag     *mag = ((ptr)
                           ? (((uintptr_t)ptr & (PAGESIZE - 1))
                              ? mtfindmag(ptr, MALLOCPAGETAB)
                              : getmag(ptr))
                           : NULL);
#endif
    long            bktid = blkbktid(sz);

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

    if (sz < n * size) {

        return NULL;
    }
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
#if (MALLOCREDBLACKTREE)
    struct rbtnode *rbt = rbtfind(&g_malloc.ptrtree, (uintptr_t)ptr);
    struct mag     *mag = (struct mag *)rbtclrcolor(rbt);
#elif (MALLOCHDRPREFIX)
    struct mag *mag = (((uintptr_t)ptr & (PAGESIZE - 1))
                       ? mtfindmag(ptr, MALLOCPAGETAB)
                       : getmag(ptr));
#else
    struct mag *mag0 = mtfindmag(ptr, MALLOCSLABTAB);
    struct mag *mag = (mag0) ? mag0 : mtfindmag(ptr, MALLOCPAGETAB);
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
    if (sz < PAGESIZE) {
        sz -= MALLOCALIGNMENT;
    }
#endif

    return sz;
}

size_t
malloc_size(void *ptr)
{
#if (MALLOCREDBLACKTREE)
    struct rbtnode *rbt = rbtfind(&g_malloc.ptrtree, (uintptr_t)ptr);
    struct mag     *mag = (struct mag *)rbtclrcolor(rbt);
    size_t          sz = (mag) ? (1UL << (mag)->bktid) : 0;
#elif (MALLOCHDRPREFIX)
    struct mag     *mag = (((uintptr_t)ptr & (PAGESIZE - 1))
                           ? mtfindmag(ptr, MALLOCPAGETAB)
                           : getmag(ptr));
    size_t          sz = (mag) ? (1UL << (mag)->bktid) : 0;
#elif (MALLOCHDRHACKS)
    size_t          sz = 1UL << (getnfo(ptr) & MEMHDRBKTMASK);
#else
    struct mag     *mag0 = mtfindmag(ptr, MALLOCSLABTAB);
    struct mag     *mag = (mag0) ? mag0 : mtfindmag(ptr, MALLOCPAGETAB);
    size_t          sz = (mag) ? (1UL << (mag)->bktid) : 0;
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

