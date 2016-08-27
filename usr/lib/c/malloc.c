/*
 * Zero Malloc Revision 3
 *
 * - a beta version of a new malloc for Zero.
 *
 * Copyright (C) Tuomo Petteri Ven‰l‰inen 2014-2016
 */

#undef MALLOCDIAG
#define MALLOCDIAG 0
//#undef MALLOCBUFMAG
//#define MALLOCBUFMAG 0
#define MALLOCVALGRINDTABS 1
#define MALLOCDEBUG 0
#define MALLOCTRACE 0
#define MALLOCSPINLOCKS 0

#if (MALLOCDEBUG)
#include <assert.h>
#endif
#include "_malloc.h"

/*
 * TODO
 * ----
 * - test and fix; there's a bug about posix_memalign(), it seems
 *
 * IMPROVEMENTS
 * ------------
 * - "real" thread-local arenas to avoid locking on the fast paths
 * - much better readability thanks to dropping testing kludges and such +
 *   splitting things into a few more functions (might work a bit more on this)
 * - adding much more testing kludges to make things a bit messy again :)
 */

#if 0
#undef  __malloctrace
#define __malloctrace()
#endif
#undef  __mallocprnttrace
#undef  __malloctrace
#define __malloctrace()
#define __mallocprnttrace(str, sz, aln)                                 \
    do {                                                                \
        fprintf(stderr, "%s: %llu(%llu)\n", str, sz, aln);              \
    } while (0)

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
 * - Martin 'bluet' Stensg√•rd for an account on an AMD64 system for testing
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
 *        - magazine cache with allocation stack of pointers into the slab
 *        - LIFO to reuse freed blocks of virtual memory
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
#include <zero/asm.h>
#include <zero/unix.h>
#include <zero/trix.h>
#if defined(MALLOCPRIOLK)
#include <zero/priolk.h>
#elif defined(MALLOCTKTLK) && (MALLOCTKTLK)
#include <zero/tktlk.h>
#endif
#if (GNUTRACE) && (MALLOCTRACE)
#include <zero/gnu.h>
#endif
#if (MALLOCDEBUGHOOKS)
#include <zero/asm.h>
#endif
#if (MALLOCLFDEQ)
#define LFDEQ_VAL_T    struct mag *
#define LFDEQ_VAL_NONE NULL
#include <zero/tagptr.h>
#include <zero/lfdeq.h>
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
#if (MALLOCFREEMAP)
    fprintf(stderr, "\tfreemap\t%p\n", mag->freemap);
#endif
    fflush(stderr);

    return;
}
#endif

/* start of the allocator proper */

struct malloc                 g_malloc ALIGNED(PAGESIZE);
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
#if (MALLOCHASHSTAT)
struct hashstat {
    unsigned long long nput;
    unsigned long long nfind;
    unsigned long long nitem;
};
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

#if (MALLOCHASHSTAT)

void
mallochashstat(void)
{
    unsigned long long  nput = 0;
    unsigned long long  nfind = 0;
    unsigned long long  nitem = 0;
    unsigned long       nlist = 0;
    unsigned long       ndx;

    if (g_malloc.flg & MALLOCINIT) {
        for (ndx = 0 ; ndx < MALLOCNHASHITEM ; ndx++) {
            nput += g_malloc.hashstat[ndx].nput;
            nfind += g_malloc.hashstat[ndx].nfind;
            if (g_malloc.hashstat[ndx].nitem) {
                nitem += g_malloc.hashstat[ndx].nitem;
                nlist++;
            }
        }
        fprintf(stderr, "HASH STATS\n");
        fprintf(stderr, "%llu puts\n", nput);
        fprintf(stderr, "%llu finds\n", nfind);
        fprintf(stderr, "%llu items\n", nitem);
        fprintf(stderr, "%lu/%lu lists in use\n", nlist, MALLOCNHASHITEM);
        fprintf(stderr, "avg chain: %f\n", nitem >> MALLOCNHASHBIT);
    }

    return;
}

#endif /* MALLOCHASHSTAT */

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
    long           bktid = mag->bktid;
    struct magtab *bkt = &g_malloc.hdrbuf[bktid];

    mag->tab = bkt;
#if (MALLOCLFDEQ)
    magenqueue(mag, bkt);
#elif (MALLOCTAILQ)
    magqueue(mag, bkt, 1);
#else
    magpush(mag, bkt);
#endif
    
    return;
}

static struct mag *
maggethdr(long bktid)
{
    struct magtab *bkt = &g_malloc.hdrbuf[bktid];
    struct mag    *mag;
    struct mag    *first;
    struct mag    *cur;
    struct mag    *prev;
    long           n;
    long           lim;
    size_t         sz;
    uint8_t       *ptr;

#if (MALLOCLFDEQ)
    mag = magdequeue(bkt);
#else
    magpop(bkt, mag);
#endif
    if (!mag) {
        if (magembedtab(bktid)) {
            n = MALLOCNBUFHDR;
            sz = MALLOCHDRSIZE;
        } else {
            n = MALLOCHDRSIZE / maghdrsz();
            sz = maghdrsz();
        }
        ptr = mapanon(g_malloc.zerofd, rounduppow2(n * sz, PAGESIZE));
        if (ptr == MAP_FAILED) {
            
            return NULL;
        }
        mag = (struct mag *)ptr;
        lim = n;
        ptr += sz;
        mag->adr = mag;
        mag->prev = NULL;
        mag->next = NULL;
        cur = (struct mag *)ptr;
        lim--;
        cur->adr = cur;
        cur->prev = NULL;
        first = cur;
        prev = cur;
        while (--lim) {
            ptr += sz;
            cur = (struct mag *)ptr;
            prev->next = cur;
            cur->adr = cur;
            cur->prev = prev;
            prev = cur;
        }
//        cur->next = NULL;
        if (first) {
#if (MALLOCLFDEQ)
            lim = n;
            while (lim--) {
                magenqueue(hdr, bkt);
                hdr = tagptrgetadr(hdr->node.next);
            }
#elif (MALLOCTAILQ)
            magqueuemany(first, cur, bkt, n);
#else
            magpushmany(first, cur, bkt, n);
#endif
        }
    }
#if (MALLOCLFDEQ)
    tagptrinitadr(NULL, mag->node.prev);
    tagptrinitadr(NULL, mag->node.next);
#endif
    mag->bktid = bktid;
    
    return mag;
}

static void *
maginit(struct mag *mag, struct magtab *bkt, long *zeroret)
{
    if (!mag->stk) {
        maginittab(mag, mag->bktid);
    }
    if (!mag->base) {
        if (!maginitslab(mag, bkt, zeroret)) {
            
            return NULL;
        }
    }

    return mag;
}

static struct mag *
magget(long bktid, struct magtab *bkt, long *zeroret)
{
    struct mag *mag = maggethdr(bktid);
    
    if (mag) {
        if (!maginit(mag, bkt, zeroret)) {
            
            return NULL;
        }
        mag->prev = NULL;
        mag->next = NULL;
    }

    return mag;
}

#define magsetfree(mag)                                                 \
    do {                                                                \
        mag->cur = 0;                                                   \
        mag->lim = 0;                                                   \
        mag->nfree = 0;                                                 \
        mag->base = NULL;                                               \
        mag->adr = NULL;                                                \
        mag->size = 0;                                                  \
    } while (0)

#if (MALLOCLAZYUNMAP)

#define MALLOCUNMAPFREQ (1L << 16)

static void
mapfree(struct mag *mag)
{
    struct arn    *arn;
    struct magtab *bkt;
    long           bktid = mag->bktid;
    void          *adr;
    long           nfree;

    arn = &thrarn;
    bkt = &arn->freebuf[bktid];
    nfree = m_fetchadd(&mag->nfree, 1);
    if (nfree == MALLOCUNMAPFREQ - 1) {
        mag->nfree = 0;
#if (!MALLOCBUFMAPSTK)
        if ((mag->lim > 1) && !magembedtab(bktid)) {
            unmapanon(mag->stk, rounduppow2(magtabsz(bktid),
                                            PAGESIZE));
#if (MALLOCSTAT)
            ntabbyte -= magtabsz(bktid);
#endif
        }
#endif /* !MALLOCBUFMAPSTK */
        adr = (void *)mag->base;
        unmapanon(adr, mag->size);
#if (MALLOCSTAT)
        nmapbyte -= mag->size;
#endif
#if (MALLOCSTAT)
        nmapbyte -= mag->size;
#endif
        VALGRINDRMPOOL(adr);
        magsetfree(mag);
        if ((mag->lim > 1) && !magembedtab(bktid)) {
            mag->cur = 0;
            mag->lim = 0;
#if (!MALLOCBUFMAPSTK)
            mag->stk = NULL;
            mag->ptrtab = NULL;
#endif /* !MALLOCBUFMAPSTK */
            mag->base = NULL;
            mag->adr = NULL;
            mag->size = 0;
#if (MALLOCFREEMAP)
            mag->freemap = NULL;
#endif
            magputhdr(mag);
        }
    } else {
        bkt = &arn->magbuf[bktid];
        mag->tab = bkt;
#if (MALLOCTAILQ)
        magqueue(mag, bkt, 0);
#else
        magpush(mag, bkt);
#endif
    }

    return;
}

#else /* !MALLOCLAZYUNMAP */

static void
mapfree(struct mag *mag)
{
    long  bktid = mag->bktid;
    void *adr;
    long  nfree;
    
    if ((mag->lim > 1) && !magembedtab(bktid)) {
        unmapanon(mag->stk, rounduppow2(magtabsz(bktid),
                                        PAGESIZE));
#if (MALLOCSTAT)
        ntabbyte -= magtabsz(bktid);
#endif
    }
    adr = (void *)mag->base;
    unmapanon(adr, mag->size);
#if (MALLOCSTAT)
    nmapbyte -= mag->size;
#endif
    VALGRINDRMPOOL(adr);
    magsetfree(mag);
    if (!magembedtab(bktid)) {
        mag->stk = NULL;
#if (!MALLOCHDRBASE)
        mag->ptrtab = NULL;
#endif
#if (MALLOCFREEMAP)
        mag->freemap = NULL;
#endif
    }
    magputhdr(mag);
    
    return;
}

#endif /* MALLOCLFDEQ && MALLOCLAZYUNMAP */

static void
thrfreetls(void *arg)
{
    struct magtab *bkt;
    struct magtab *dbkt;
    struct arn    *arn = arg;
    struct mag    *mag;
    struct mag    *first;
    struct mag    *tail = NULL;
    long           bktid;
    long           n = 0;

    bkt = &arn->magbuf[0];
    dbkt = &g_malloc.magbuf[0];
    for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
        mag = bkt->ptr;
        if (mag) {
            mag->tab = dbkt;
#if (MALLOCBUFMAG)
            n++;
#endif
            first = mag;
            mag->arn = NULL;
#if (MALLOCLFDEQ)
            while (mag->next) {
                mag = mag->next;
#if (MALLOCBUFMAG)
                n++;
#endif
                mag->arn = NULL;
                mag->tab = dbkt;
                tail = mag;
            }
#else
            while (mag->next) {
                mag = mag->next;
#if (MALLOCBUFMAG)
                n++;
#endif
                mag->arn = NULL;
                mag->tab = dbkt;
                tail = mag;
            }
#endif /* MALLOCLFDEQ */
            mag = first;
            if (tail) {
#if (MALLOCLFDEQ)
                while (mag != tail) {
                    magenqueue(mag, dbkt);
                    mag = mag->next;
                }
                /* tail */
                magenqueue(mag, dbkt);
#elif (MALLOCTAILQ)
                magqueuemany(first, tail, dbkt, n);
#else
                magpushmany(first, tail, dbkt, n);
#endif
            } else {
#if (MALLOCLFDEQ)
                magenqueue(mag, dbkt);
#elif (MALLOCTAILQ)
                magqueue(mag, dbkt, 1);
#else
                magpush(mag, dbkt);
#endif
            }
        }
        bkt++;
        dbkt++;
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

//    fmtxlk(&g_malloc.priolk);
    val = g_malloc.prioval;
    g_malloc.prioval = (val++) & (sizeof(long) * CHAR_BIT - 1);
//    fmtxunlk(&g_malloc.priolk);

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
maginitslab(struct mag *mag, struct magtab *bkt, long *zeroret)
{
    long        bktid = mag->bktid;
    void      **stk;
    size_t      sz = 1UL << bktid;
    long        n = magnblk(bktid);
    uint8_t    *ptr;
    long        ndx;
    uintptr_t   magflg = 0;

    if (!mag->base) {
        ptr = SBRK_FAILED;
#if (!MALLOCNOSBRK)
        if (bktid <= MALLOCSLABLOG2 && !(g_malloc.flg & MALLOCNOHEAP)) {
            /* try to allocate slab from heap */
            __malloclk(&g_malloc.heaplk);
            ptr = growheap(rounduppow2(n * sz, PAGESIZE));
            __mallocunlk(&g_malloc.heaplk);
            if (ptr == SBRK_FAILED) {
                g_malloc.flg |= MALLOCNOHEAP;
            }
            *zeroret = 1;
        }
#endif
        if (ptr == SBRK_FAILED) {
            mag->nfree = 0;
            /* try to map slab */
            ptr = mapanon(g_malloc.zerofd,
                             rounduppow2(n * sz, PAGESIZE));
            if (ptr == MAP_FAILED) {
                
                return NULL;
            }
            *zeroret = 0;
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
        mag->adr = (void *)((uintptr_t)ptr | magflg);
        mag->size = rounduppow2(n * sz, PAGESIZE);
        mag->cur = 0;
        mag->lim = n;
        mag->bktid = bktid;
        mag->tab = bkt;
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
    void   *ret;
    long    n = magnblk(bktid);
    size_t  sz;

    if (!mag->stk) {
        if (n == 1) {
            mag->stk = NULL;
#if (!MALLOCHDRBASE)
            mag->ptrtab = NULL;
#endif
#if (MALLOCFREEMAP)
            mag->freemap = NULL;
#endif
        } else if (magembedtab(bktid)) {
            mag->stk = (void **)((uint8_t *)mag + maghdrsz());
#if (!MALLOCHDRBASE)
            mag->ptrtab = ((uint8_t *)mag->stk + n * sizeof(void *));
#endif
#if (MALLOCFREEMAP)
#if (MALLOCHDRBASE)
            mag->freemap = ((uint8_t *)mag->stk + n * sizeof(void *));
#else
            mag->freemap = ((uint8_t *)mag->stk + (n << 1) * sizeof(void *));
#endif
#endif
        } else {
            sz = magtabsz(bktid);
            ret = mapanon(g_malloc.zerofd, rounduppow2(sz, PAGESIZE));
            if (ret == MAP_FAILED) {
                
                return NULL;
            }
            mag->stk = ret;
#if (!MALLOCHDRBASE)
            mag->ptrtab = ((uint8_t *)ret + n * sizeof(void *));
#endif
#if (MALLOCFREEMAP)
#if (MALLOCHDRBASE)
            mag->freemap = ((uint8_t *)ret + n * sizeof(void *));
#else
            mag->freemap = ((uint8_t *)ret + (n << 1) * sizeof(void *));
#endif
#endif
#if (MALLOCSTAT)
            nmapbyte += rounduppow2(sz, magtabsz(bktid));
#endif
        }
    }

    return mag;
 }
    
static void
prefork(void)
{
    long   ndx;
#if (MALLOCATOMIC)
    long   res;
    void **bptr;
#endif

    __malloclk(&g_malloc.initlk);
    __malloclk(&g_malloc.heaplk);
    for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
#if (MALLOCATOMIC)
        bptr = &g_malloc.hdrbuf[ndx].ptr;
        do {
            res = m_cmpsetbit((volatile long *)bptr,
                              MALLOC_LK_BIT_POS);
        } while (res);
        bptr = &g_malloc.magbuf[ndx].ptr;
        do {
            res = m_cmpsetbit((volatile long *)bptr,
                              MALLOC_LK_BIT_POS);
        } while (res);
        bptr = &g_malloc.freebuf[ndx].ptr;
        do {
            res = m_cmpsetbit((volatile long *)bptr,
                              MALLOC_LK_BIT_POS);
        } while (res);
#else
        __malloclk(&g_malloc.hdrbuf[ndx].lk);
        __malloclk(&g_malloc.magbuf[ndx].lk);
        __malloclk(&g_malloc.freebuf[ndx].lk);
#endif
    }
    
    return;
}

static void
postfork(void)
{
    long   ndx;
#if (MALLOCATOMIC)
    void **bptr;
#endif

    for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
#if (MALLOCATOMIC)
        bptr = &g_malloc.freebuf[ndx].ptr;
        m_cmpclrbit((volatile long *)bptr,
                    MALLOC_LK_BIT_POS);
        bptr = &g_malloc.magbuf[ndx].ptr;
        m_cmpclrbit((volatile long *)bptr,
                    MALLOC_LK_BIT_POS);
        bptr = &g_malloc.hdrbuf[ndx].ptr;
        m_cmpclrbit((volatile long *)bptr,
                    MALLOC_LK_BIT_POS);
#else
        __malloclk(&g_malloc.freebuf[ndx].lk);
        __malloclk(&g_malloc.magbuf[ndx].lk);
        __malloclk(&g_malloc.hdrbuf[ndx].lk);
#endif
    }
    __mallocunlk(&g_malloc.heaplk);
    __mallocunlk(&g_malloc.initlk);
    
    return;
}

#if (MALLOCARRAYHASH)

/*
 * NOTES 
 * -----
 * - the hash table is based on knowing that pages from different magazines do
 *   not overlap
 */

#define MALLOC_HASH_MARK_POS 0
#define MALLOC_HASH_MARK_BIT (1L << 0)

/* allocate a hash table magazine chain item */
static struct hashblk *
hashgetblk(void)
{
    uintptr_t        upval;
    struct hashblk  *head;
    struct hashblk  *first;
    struct hashblk  *item;
    struct hashblk  *cur;
    struct hashblk  *prev;
    struct hashblk **hptr;
    long             res;
    size_t           n;

    /* obtain bit-lock on the chain head */
    hptr = &g_malloc.hashblkbuf;
//    head = g_malloc.hashbuf;
    res = m_cmpsetbit((volatile long *)hptr,
                      MALLOC_HASH_MARK_POS);
    while (res) {
        unsigned long nloop = 4096;
        
        do {
            res = m_cmpsetbit((volatile long *)hptr,
                              MALLOC_HASH_MARK_POS);
        } while (nloop--);
        if (res) {
            pthread_yield();
        }
    }
    head = g_malloc.hashblkbuf;
    upval = (uintptr_t)head;
    upval &= ~MALLOC_HASH_MARK_BIT;
    head = (struct hashblk *)upval;
    if (head) {
        m_syncwrite((volatile long *)hptr, head->next);

        return head;
    }
    /* allocate a page's worth of hash table magazine chain entries */
    n = PAGESIZE / sizeof(struct hashblk);
    item = mapanon(g_malloc.zerofd, PAGESIZE);
    if (item == MAP_FAILED) {
            abort();
    }
    /* chain entries */
    prev = item;
    cur = item;
    cur++;
    n--;
    first = cur;
    prev = first;
    while (--n) {
        cur++;
        prev->next = cur;
        prev = cur;
    }
    cur->next = NULL;
    /* add item->next to the head of queue; the head will be unlocked */
    m_syncwrite((volatile long *)hptr, first);

    return item;
}

/* buffer an allocated hash table magazine chain item */
static void
hashbufblk(struct hashblk *blk)
{
    uintptr_t        upval;
    struct hashblk  *head;
    struct hashblk **hptr;
    long             res;

    /* obtain bit-lock on the chain head */
    hptr = &g_malloc.hashblkbuf;
//    head = g_malloc.hashbuf;
    res = m_cmpsetbit((volatile long *)hptr,
                      MALLOC_HASH_MARK_POS);
    while (res) {
        unsigned long nloop = 4096;
        
        do {
            res = m_cmpsetbit((volatile long *)hptr,
                              MALLOC_HASH_MARK_POS);
        } while (nloop--);
        if (res) {
            pthread_yield();
        }
    }
    head = g_malloc.hashblkbuf;
    upval = (uintptr_t)head;
    upval &= ~MALLOC_HASH_MARK_BIT;
    head = (struct hashblk *)upval;
    /* add item to the head of queue; the head will be unlocked */
    blk->next = head;
    m_atomwrite((volatile long *)hptr,
                blk);

    return;
}

#define MALLOCHASHFIND 0
#define MALLOCHASHPUT  1
#define MALLOCHASHGET  2
/* find a hash table magazine chain item */
static struct mag *
hashfindmag(void *ptr, long act)
{
    uintptr_t        upval = (uintptr_t)ptr;
    struct mag      *mag;
    struct hashblk **hptr;
    struct hashmag  *blk;
    struct hashblk  *head;
    struct hashblk  *cur;
    struct hashblk  *prev;
    long             res;
    long             key;
    unsigned long    n;

#if (MALLOCNEWHASH) && 0
    key = hashq128uptr(upval, MALLOCNHASHBIT);
#elif (MALLOCRAZOHASH)
    key = razohash(upval, sizeof(uintptr_t), MALLOCNHASHBIT);
#if 0
    if (!bitset(g_malloc.hashmap, upval)) {

        return NULL;
    }
#endif
#else
    key = (upval * 127) & ((1UL << (MALLOCNHASHBIT)) - 1);
#endif
    /* obtain bit-lock on the chain head */
    hptr = &g_malloc.hashtab[key];
//    head = g_malloc.maghash[key];
#if (!MALLOCNEWHASH)
    do {
        res = m_cmpsetbit((volatile long *)hptr,
                          MALLOC_HASH_MARK_POS);
    } while (res);
#else
    res = m_cmpsetbit((volatile long *)hptr,
                      MALLOC_HASH_MARK_POS);
    while (res) {
        unsigned long nloop = 4096;
        
        do {
            res = m_cmpsetbit((volatile long *)hptr,
                              MALLOC_HASH_MARK_POS);
        } while (nloop--);
        if (res) {
            pthread_yield();
        }
    }
#endif
    head = *hptr;
    upval = (uintptr_t)head;
    upval &= ~MALLOC_HASH_MARK_BIT;
    head = (struct hashblk *)upval;
    cur = head;
    if (cur) {
        prev = NULL;
        mag = NULL;
        blk = NULL;
        while (cur) {
            n = hashgetitemcnt(cur);
            switch (n) {
                case 15:
                    if (cur->tab[14].ptr == ptr) {
                        blk = &cur->tab[14];

                        break;
                    }
                case 14:
                    if (cur->tab[13].ptr == ptr) {
                        blk = &cur->tab[13];

                        break;
                    }
                case 13:
                    if (cur->tab[12].ptr == ptr) {
                        blk = &cur->tab[12];

                        break;
                    }
                case 12:
                    if (cur->tab[11].ptr == ptr) {
                        blk = &cur->tab[11];

                        break;
                    }
                case 11:
                    if (cur->tab[10].ptr == ptr) {
                        blk = &cur->tab[10];

                        break;
                    }
                case 10:
                    if (cur->tab[9].ptr == ptr) {
                        blk = &cur->tab[9];

                        break;
                    }
                case 9:
                    if (cur->tab[8].ptr == ptr) {
                        blk = &cur->tab[8];

                        break;
                    }
                case 8:
                    if (cur->tab[7].ptr == ptr) {
                        blk = &cur->tab[7];

                        break;
                    }
                case 7:
                    if (cur->tab[6].ptr == ptr) {
                        blk = &cur->tab[6];

                        break;
                    }
                case 6:
                    if (cur->tab[5].ptr == ptr) {
                        blk = &cur->tab[5];

                        break;
                    }
                case 5:
                    if (cur->tab[4].ptr == ptr) {
                        blk = &cur->tab[4];

                        break;
                    }
                case 4:
                    if (cur->tab[3].ptr == ptr) {
                        blk = &cur->tab[3];

                        break;
                    }
                case 3:
                    if (cur->tab[2].ptr == ptr) {
                        blk = &cur->tab[2];

                        break;
                    }
                case 2:
                    if (cur->tab[1].ptr == ptr) {
                        blk = &cur->tab[1];

                        break;
                    }
                case 1:
                    if (cur->tab[0].ptr == ptr) {
                        blk = &cur->tab[0];

                        break;
                    }
                case 0:
                default:

                    break;
            }
            if (blk) {
                if (act == MALLOCHASHFIND) {
                    mag = blk->mag;
                } else if (act == MALLOCHASHPUT) {
                    blk->mag = mag;
                    blk->ptr = ptr;
                } else if (act == MALLOCHASHGET) {
                    mag = blk->mag;
                    blk->ptr = NULL;
                }
                if (prev) {
                    prev->next = cur->next;
                }
                cur->next = head;
                m_syncwrite(hptr, cur);

                return mag;
            }
            prev = cur;
            cur = cur->next;
        }
    }
    if (!cur && act == MALLOCHASHPUT) {
        cur = hashgetblk();
        cur->tab[0].mag = mag;
        cur->tab[0].ptr = ptr;
        hashsetitemcnt(cur, 1);
        cur->next = head;
        head = cur;
    }
    m_syncwrite(hptr, head);

    return NULL;
}

static struct mag *
hashputmag(void *ptr, struct mag *mag)
{
    if (mag) {
        mag = hashfindmag(ptr, MALLOCHASHPUT);
    } else {
        mag = hashfindmag(ptr, MALLOCHASHGET);
    }

    return mag;
}

#elif (MALLOCHASH)

/*
 * NOTES 
 * -----
 * - the hash table is based on knowing that pages from different magazines do
 *   not overlap
 */

#define MALLOC_HASH_MARK_POS 0
#define MALLOC_HASH_MARK_BIT (1L << 0)

/* allocate a hash table magazine chain item */
static struct hashmag *
hashgetitem(void)
{
    uintptr_t        upval;
    struct hashmag  *head;
    struct hashmag  *item;
    struct hashmag  *cur;
    struct hashmag  *prev;
    struct hashmag **hptr;
    long             res;
    size_t           n;

    /* obtain bit-lock on the chain head */
    hptr = &g_malloc.hashbuf;
//    head = g_malloc.hashbuf;
#if (!MALLOCNEWHASH)
    do {
        res = m_cmpsetbit((volatile long *)hptr,
                          MALLOC_HASH_MARK_POS);
    } while (res);
#else
    res = m_cmpsetbit((volatile long *)hptr,
                      MALLOC_HASH_MARK_POS);
    while (res) {
        unsigned long nloop = 4096;
        
        do {
            res = m_cmpsetbit((volatile long *)hptr,
                              MALLOC_HASH_MARK_POS);
        } while ((res) && nloop--);
        if (res) {
            pthread_yield();
        }
    }
#endif
    head = *hptr;
    upval = (uintptr_t)head;
    upval &= ~MALLOC_HASH_MARK_BIT;
    head = (struct hashmag *)upval;
    if (!head) {
        /* allocate a page's worth of hash table magazine chain entries */
        n = PAGESIZE / sizeof(struct hashmag);
        item = mapanon(g_malloc.zerofd, PAGESIZE);
        if (item == MAP_FAILED) {
            abort();
        }
        /* chain entries */
        cur = item;
        n--;
        cur++;
        head = cur;
        while (--n) {
            prev = cur;
            cur++;
            prev->next = cur;
        }
        cur->next = NULL;
    } else {
        item = head;
        head = head->next;
    }
    m_syncwrite((volatile long *)hptr, head);

    return item;
}

/* buffer an allocated hash table magazine chain item */
static void
hashbufitem(struct hashmag *item)
{
    uintptr_t        upval;
    struct hashmag  *head;
    struct hashmag **hptr;
    long             res;

    /* obtain bit-lock on the chain head */
    hptr = &g_malloc.hashbuf;
//    head = g_malloc.hashbuf;
#if (!MALLOCNEWHASH)
    do {
        res = m_cmpsetbit((volatile long *)hptr,
                          MALLOC_HASH_MARK_POS);
    } while (res);
#else
    res = m_cmpsetbit((volatile long *)hptr,
                      MALLOC_HASH_MARK_POS);
    while (res) {
        unsigned long nloop = 4096;
        
        do {
            res = m_cmpsetbit((volatile long *)hptr,
                              MALLOC_HASH_MARK_POS);
        } while (nloop--);
        if (res) {
            pthread_yield();
        }
    }
#endif
    head = *hptr;
    upval = (uintptr_t)head;
    upval &= ~MALLOC_HASH_MARK_BIT;
    head = (struct hashmag *)upval;
    item->nref = 0;
    item->ptr = NULL;
    item->mag = NULL;
    /* add item to the head of queue; the head will be unlocked */
    item->next = head;
    m_atomwrite((volatile long *)hptr,
                item);

    return;
}

/* find a hash table magazine chain item */
static struct mag *
hashfindmag(void *ptr)
{
    uintptr_t        upval = (uintptr_t)ptr;
    struct mag      *mag;
    struct hashmag **hptr;
    struct hashmag  *head;
    struct hashmag  *cur;
    struct hashmag  *prev = NULL;
    long             res;
    unsigned long    key;

#if (MALLOCNEWHASH) && 0
    key = hashq128uptr(upval, MALLOCNHASHBIT);
#else
    key = upval & ((1UL << (MALLOCNHASHBIT)) - 1);
#endif
    /* obtain bit-lock on the chain head */
    hptr = &g_malloc.maghash[key];
//    head = g_malloc.maghash[key];
#if (!MALLOCNEWHASH)
    do {
        res = m_cmpsetbit((volatile long *)hptr,
                          MALLOC_HASH_MARK_POS);
    } while (res);
#else
    res = m_cmpsetbit((volatile long *)hptr,
                      MALLOC_HASH_MARK_POS);
    while (res) {
        unsigned long nloop = 4096;
        
        do {
            res = m_cmpsetbit((volatile long *)hptr,
                              MALLOC_HASH_MARK_POS);
        } while (nloop--);
        if (res) {
            pthread_yield();
        }
    }
#endif
    head = *hptr;
    upval = (uintptr_t)head;
    upval &= ~MALLOC_HASH_MARK_BIT;
    head = (struct hashmag *)upval;
    cur = head;
    if (cur) {
        while (cur) {
            if (cur->ptr == ptr) {
                mag = cur->mag;
                if (prev) {
                    prev->next = cur->next;
                }
                mag->next = head;
                head = mag;
                m_syncwrite(hptr, head);
                    
                return mag;
            }
            prev = cur;
            cur = cur->next;
        }
    } else {
        m_cmpclrbit((volatile long *)hptr,
                    MALLOC_HASH_MARK_POS);
    }
    
    return NULL;
}

static struct mag *
hashsetmag(void *ptr, struct mag *mag)
{
    uintptr_t        upval = (uintptr_t)ptr;
    struct hashmag  *head;
    struct hashmag **hptr;
    struct hashmag  *cur = NULL;
    struct hashmag  *item;
    struct hashmag  *prev;
    long             res;
    unsigned long    key;

#if (MALLOCNEWHASH) && 0
    key = hashq128uptr(upval, MALLOCNHASHBIT);
#else
    key = upval & ((1UL << (MALLOCNHASHBIT)) - 1);
#endif
    /* obtain bit-lock on the chain head */
    hptr = &g_malloc.maghash[key];
//    head = g_malloc.maghash[key];
#if (!MALLOCNEWHASH)
    do {
        res = m_cmpsetbit((volatile long *)hptr,
                          MALLOC_HASH_MARK_POS);
    } while (res);
#else
    res = m_cmpsetbit((volatile long *)hptr,
                      MALLOC_HASH_MARK_POS);
    while (res) {
        unsigned long nloop = 4096;
        
        do {
            res = m_cmpsetbit((volatile long *)hptr,
                              MALLOC_HASH_MARK_POS);
        } while (nloop--);
        if (res) {
            pthread_yield();
        }
    }
#endif
    head = *hptr;
    upval = (uintptr_t)head;
    upval &= ~MALLOC_HASH_MARK_BIT;
    head = (struct hashmag *)upval;
    cur = head;
    if (cur) {
        prev = NULL;
        while (cur) {
            /* look for an existing item to update */
            if (cur->ptr == ptr) {
                if (mag) {
//                m_atominc(&cur->nref);
                    cur->nref++;
                    m_cmpclrbit((volatile long *)hptr,
                                MALLOC_HASH_MARK_POS);
                    
                    return mag;
                } else if (!--cur->nref) {
                    if (prev) {
                        prev->next = cur->next;
                        m_syncwrite((volatile long *)hptr, head);
                    } else {
                        m_syncwrite((volatile long *)hptr, cur->next);
                    }
                    hashbufitem(cur);
                    
                    return NULL;
                } else {
                    m_cmpclrbit((volatile long *)hptr,
                                MALLOC_HASH_MARK_POS);
                    
                    return mag;
                }
            }
            prev = cur;
            cur = cur->next;
        }
    }
    if (!cur && (mag)) {
        item = hashgetitem();
        if (!item) {
            abort();
        }
        item->nref = 1;
        item->ptr = ptr;
        item->mag = mag;
        /* add item to the head of queue; the head will be unlocked */
        item->next = head;
        m_atomwrite((volatile long *)hptr, item);

        return mag;
    } else {
        m_cmpclrbit((volatile long *)hptr,
                    MALLOC_HASH_MARK_POS);
    }

    return NULL;
}

#elif (!MALLOCHDRPREFIX)

static void
mtsetmag(void *ptr,
         struct mag *mag)
{
    void      *ptr1;
    void      *ptr2;
    uintptr_t  l1 = pagedirl1ndx(ptr);
    uintptr_t  l2 = pagedirl2ndx(ptr);
    uintptr_t  l3 = pagedirl3ndx(ptr);
    
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

#if (MALLOCDIAG)
static void
mallocdiag(void)
{
    struct arn *arn = thrarn;
    struct mag *mag1;
    struct mag *mag2;
    long        ndx;

    if (arn) {
        for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
            mag1 = arn->magbuf[ndx].ptr;
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
        __malloclk(&g_malloc.magbuf[ndx].lk);
        mag1 = g_malloc.magbuf[ndx].ptr;
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
        __mallocunlk(&g_malloc.magbuf[ndx].lk);
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

    __malloclk(&g_malloc.initlk);
    if (g_malloc.flg & MALLOCINIT) { 
        __mallocunlk(&g_malloc.initlk);
       
        return;
    }
#if defined(GNUMALLOC) && (GNUMALLOC)
    __malloc_hook = gnu_malloc_hook;
    __realloc_hook = gnu_realloc_hook;
    __memalign_hook = gnu_memalign_hook;
    __free_hook = gnu_free_hook;
#endif
#if (MALLOCSIG) && (!MALLOCSTAT)
    signal(SIGQUIT, mallquit);
    signal(SIGINT, mallquit);
    signal(SIGSEGV, mallquit);
    signal(SIGABRT, mallquit);
    signal(SIGTERM, mallquit);
#endif
#if (MALLOCSTAT)
    atexit(mallocstat);
#endif
#if (MMAP_DEV_ZERO)
    g_malloc.zerofd = open("/dev/zero", O_RDWR);
    if (g_malloc.zerofd < 0) {
        fprintf(stderr, "failed to open /dev/zero for mmap()\n");

        exit(1);
    }
#endif
    ndx = MALLOCNBKT;
    while (ndx--) {
#if (MALLOCLFDEQ)
        lfdeqinitqueue(&g_malloc.hdrbuf[ndx].lfdeq);
        lfdeqinitqueue(&g_malloc.freebuf[ndx].lfdeq);
        lfdeqinitqueue(&g_malloc.magbuf[ndx].lfdeq);
#endif
#if (MALLOCSPINLOCKS)
        __mallocinitspin(&g_malloc.hdrbuf[ndx].lk);
#else
        __mallocinitlk(&g_malloc.hdrbuf[ndx].lk);
#endif
        __mallocinitlk(&g_malloc.magbuf[ndx].lk);
        __mallocinitlk(&g_malloc.freebuf[ndx].lk);
    }
#if (!MALLOCNOSBRK)
    __malloclk(&g_malloc.heaplk);
    heap = growheap(0);
    ofs = (1UL << PAGESIZELOG2) - ((long)heap & (PAGESIZE - 1));
    if (ofs != PAGESIZE) {
        growheap(ofs);
    }
    __mallocunlk(&g_malloc.heaplk);
#endif /* !MALLOCNOSBRK */
#if (!MALLOCHASH)
#if (!MALLOCNEWMULTITAB)
#if (MALLOCMULTITAB)
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
#endif
#endif
#endif /* !MALLOCHASH */
#if (MALLOCARRAYHASH)
    g_malloc.hashtab = mapanon(g_malloc.zerofd,
                               MALLOCNHASHITEM * sizeof(struct hashblk *));
    if (g_malloc.hashtab == MAP_FAILED) {
        abort();
    }
#if 0
    g_malloc.hashmap = mapanon(g_malloc.zerofd,
                               (ADRBITS - PAGESIZELOG2) / CHAR_BIT);
    if (g_malloc.hashtab == MAP_FAILED) {
        abort();
    }
#endif
#if (MALLOCSTAT)
    ntabbyte += MALLOCNHASHITEM * sizeof(struct hashblk);
#endif
#elif (MALLOCHASH) && !(MALLOCMULTITAB) && !(MALLOCNEWMULTITAB)
    g_malloc.maghash = mapanon(g_malloc.zerofd,
                               MALLOCNHASHITEM * sizeof(struct hashmag *));
    if (g_malloc.maghash == MAP_FAILED) {
        abort();
    }
#elif (MALLOCFREETABS) && (PTRBITS > 32) && (MALLOCMULTITAB)
    g_malloc.pagedir = mapanon(g_malloc.zerofd,
                               PAGEDIRNL1KEY * sizeof(struct memtab));
    if (g_malloc.pagedir == MAP_FAILED) {
        abort();
    }
#if (MALLOCSTAT)
    ntabbyte += PAGEDIRNL1KEY * sizeof(struct memtab);
#endif
#elif (PTRBITS > 32) && !(MALLOCHDRPREFIX) /* !MALLOCHASH && !MALLOCFREETABS */
    g_malloc.pagedir = mapanon(g_malloc.zerofd,
                               PAGEDIRNL1KEY * sizeof(void *));
    if (g_malloc.pagedir == MAP_FAILED) {
        abort();
    }
#if (MALLOCSTAT)
    ntabbyte += PAGEDIRNL1KEY * sizeof(void *);
#endif
#endif
#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zmalloc_initialize_hook) {
        __zmalloc_initialize_hook();
    }
#endif
#if (MALLOCHASHSTAT)
    g_malloc.hashstat = mapanon(g_malloc.zerofd,
                                MALLOCNHASHITEM * sizeof(struct hashstat));
    if (g_malloc.hashstat == MAP_FAILED) {
        abort();
    }
#endif
    pthread_atfork(prefork, postfork, postfork);
    g_malloc.flg |= MALLOCINIT;
    __mallocunlk(&g_malloc.initlk);
    
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
    struct magtab *bkt;
    uintptr_t      upval;
    long           id;
    struct mag    *mag = NULL;
    struct arn    *arn;
    uint8_t       *adr;
    uint8_t       *ptr = NULL;
#if (MALLOCHDRPREFIX)
    size_t         sz = ((align <= MALLOCALIGNMENT)
                         ? (size + max(MEMHDRSIZE, MALLOCALIGNMENT))
                         : (size + max(MEMHDRSIZE, MALLOCALIGNMENT) + align));
#else
    size_t         sz = align <= MALLOCALIGNMENT ? size : size + align - 1;
#endif
    long           bktid = blkbktid(sz);
    long           lim;
#if (MALLOCHDRHACKS)
    uint8_t        nfo = bktid;
#endif
    long           zeroflg = 0;
    
    arn = &thrarn;
    if (!(thrflg & MALLOCINIT)) {
        thrinit();
        thrflg |= MALLOCINIT;
    }
    if (!(g_malloc.flg & MALLOCINIT)) {
        mallinit();
    }
//    align = max(align, MALLOCALIGNMENT);
    /* try to allocate from a partially used magazine */
    bkt = &arn->magbuf[bktid];
    do {
        ;
    } while (m_cmpsetbit((volatile long *)&bkt->ptr,
                         MALLOC_LK_BIT_POS));
    upval = (uintptr_t)bkt->ptr;
    mag = (void *)(upval & ~MALLOC_LK_BIT);
    if (mag) {
        lim = mag->lim;
        if (lim == 1) {
            ptr = mag->base;
            mag->cur = 1;
        } else {
            id = mag->cur++;
#if (MALLOCDEBUG)
            if (id >= mag->lim) {
                fprintf(stderr, "ID: %ld\n", id);
                magprint(mag);
            }
            assert(id < mag->lim);
#endif
            ptr = ((void **)mag->stk)[id];
        }
        if (mag->cur == lim) {
            magrmhead(bkt, mag);
            mag->arn = NULL;
            mag->prev = NULL;
            mag->next = NULL;
            mag->tab = NULL;
        } else {
            m_cmpclrbit(&bkt->ptr, MALLOC_LK_BIT_POS);
        }
    } else {
        m_syncwrite(&bkt->ptr, NULL);
        bkt = &g_malloc.magbuf[bktid];
        do {
            ;
        } while (m_cmpsetbit((volatile long *)&bkt->ptr,
                             MALLOC_LK_BIT_POS));
        upval = (uintptr_t)bkt->ptr;
        mag = (void *)(upval & ~MALLOC_LK_BIT);
        if (!mag) {
            m_syncwrite(&bkt->ptr, NULL);
        } else {
#if (MALLOCLFDEQ)
            mag = magdequeue(bkt);
#else
            magrmhead(bkt, mag);
#endif
        }
        if (!mag) {
            bkt = &g_malloc.freebuf[bktid];
            do {
                ;
            } while (m_cmpsetbit((volatile long *)&bkt->ptr,
                                 MALLOC_LK_BIT_POS));
            upval = (uintptr_t)bkt->ptr;
            mag = (void *)(upval & ~MALLOC_LK_BIT);
            if (!mag) {
                m_syncwrite(&bkt->ptr, NULL);
            } else {
#if (MALLOCLFDEQ)
                mag = magdequeue(bkt);
#else
                magrmhead(bkt, mag);
#endif
            }
            if (!mag) {
                bkt = &arn->magbuf[bktid];
                mag = magget(bktid, bkt, &zeroflg);
            }
        }
#if (MALLOCDEBUG)
        assert(mag != NULL);
#endif
        if (mag) {
            lim = mag->lim;
            if (lim == 1) {
                ptr = mag->base;
                mag->cur = 1;
            } else {
                id = mag->cur++;
#if (MALLOCDEBUG)
                if (mag->cur > mag->lim) {
                    magprint(mag);
                }
                assert(mag->cur <= mag->lim);
#endif
                ptr = ((void **)mag->stk)[id];
            }
            if (mag->cur < lim) {
                bkt = &arn->magbuf[bktid];
                mag->arn = arn;
                magpush(mag, bkt);
            } else {
                mag->arn = NULL;
                mag->prev = NULL;
                mag->next = NULL;
                mag->tab = NULL;
            }
        }
    }
#if (MALLOCDEBUG)
    if (!ptr && (mag)) {
        magprint(mag);
    }
    assert(ptr != NULL);
#endif
    if (ptr) {
        adr = clrptr(ptr);
        ptr = adr;
        sz = 1UL << bktid;
        if ((zero) && (zeroflg)) {
//            _memsetbk(ptr, 0, sz);
            memset(ptr, 0, sz);
        } else if (g_malloc.mallopt.flg & MALLOPT_PERTURB_BIT) {
            int perturb = g_malloc.mallopt.perturb;
            
            perturb = (~perturb) & 0xff;
//            _memsetbk(ptr, perturb, sz);
            memset(ptr, perturb, sz);
        }
#if (MALLOCHDRPREFIX)
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
#if (MALLOCHDRBASE)
        setbase(ptr, adr);
#endif
#else /* !MALLOCHDRPREFIX */
        if (align) {
            if ((uintptr_t)ptr & (align - 1)) {
                ptr = ptralign(ptr, align);
            }
        }
#if (MALLOCARRAYHASH)
        if (!((uintptr_t)adr & MAGUSED)) {
            hashputmag(ptr, mag);
        }
#elif (MALLOCHASH)
        hashsetmag(ptr, mag);
#elif (MALLOCMULTITAB)
        mtsetmag(ptr, mag);
#endif
#endif /* MALLOCHDRPREFIX */
#if 0
        if (align) {
            if ((uintptr_t)ptr & (align - 1)) {
                ptr = ptralign(ptr, align);
            }
        }
#if (MALLOCARRAYHASH)
        if (!((uintptr_t)adr & MAGUSED)) {
            hashputmag(ptr, mag);
        }
#elif (MALLOCHASH)
        hashsetmag(ptr, mag);
#elif (MALLOCMULTITAB)
        mtsetmag(ptr, mag);
#endif
#endif
#if !((MALLOCHDRPREFIX) && (MALLOCHDRBASE))
        magputptr(mag, ptr, adr);
#endif
#if (MALLOCHDRHACKS)
        setnfo(ptr, nfo);
#endif
#if (MALLOCFREEMAP)
        __malloclk(&mag->freelk);
        if (((mag->lim == 1) && (mag->freemap))
            || ((mag->lim > 1) && bitset(mag->freemap, magptrid(mag, ptr)))) {
#if (MALLOCDEBUG)
            magprint(mag);
#endif
            fprintf(stderr, "trying to reallocate block");
            fflush(stderr);
            
            abort();
        } else if (mag->lim == 1) {
            mag->freemap = (void *)(~(uintptr_t)NULL);
        } else {
            setbit(mag->freemap, magptrid(mag, adr));
        }
        __mallocunlk(&mag->freelk);
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
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif
    
    return ptr;
}


void
_free(void *ptr)
{
    struct magtab *bkt;
    struct arn    *arn;
    struct mag    *mag = NULL;
    void          *adr = NULL;
    uintptr_t      upval;
    long           bktid;
    long           lim;
#if (MALLOCLAZYUNMAP)
    static long    nfree = 0;
#endif
#if (MALLOCFREEMAP)
    long           pos;
#endif
#if (MALLOCHDRHACKS)
    uint8_t        nfo = 0;
#endif
    long           ndx;
    long           cur;
    uintptr_t      mapflg;
    
    if (!ptr) {
        
        return;
    }
    VALGRINDFREE(ptr);
    arn = &thrarn;
    if (!(thrflg & MALLOCINIT)) {
        thrinit();
        thrflg |= MALLOCINIT;
    }
#if (MALLOCHDRPREFIX)
    mag = getmag(ptr);
#if (MALLOCHDRBASE)
    adr = getbase(ptr);
#endif
#else /* !MALLOCHDRPREFIX */
#if (MALLOCARRAYHASH)
    mag = hashfindmag(ptr, MALLOCHASHGET);
#elif (MALLOCHASH)
    mag = hashfindmag(ptr);
#elif (MALLOCMULTITAB)
    mag = mtfindmag(ptr);
#endif
#endif /* MALLOCHDRPREFIX */
#if (MALLOCDEBUG)
    assert(mag != NULL);
#endif
    if (mag) {
#if (MALLOCPARANOIA)
        do {
            ;
        } while (m_cmpsetbit((volatile long *)&mag->lk,
                             MALLOC_LK_BIT_POS));
#endif
        arn = mag->arn;
#if (MALLOCHDRPREFIX)
        setmag(ptr, NULL);
#if (MALLOCHDRBASE)
        setbase(ptr, NULL);
#endif
#elif !(MALLOCARRAYHASH)
#if (MALLOCHASH)
        hashsetmag(ptr, NULL);
#endif
#endif
#if (MALLOCHDRHACKS)
        bktid = nfo & MEMHDRBKTMASK;
#else
        bktid = mag->bktid;
#endif
#if 0
        if (g_malloc.mallopt.flg & MALLOPT_PERTURB_BIT) {
            int perturb = g_malloc.mallopt.perturb;
            
            perturb &= 0xff;
            memset(adr, perturb, sz);
        }
#endif
        lim = mag->lim;
#if (!MALLOCHDRPREFIX)
        if (lim == 1) {
            ptr = mag->base;
        } else if (!adr) {
            ptr = maggetptr(mag, ptr);
        }
//        ptr = clrptr(adr);
#endif
        adr = markptr(adr);
#if (MALLOCDEBUG)
        assert(adr != NULL);
#endif
#if (MALLOCFREEMAP)
        /* FIXME: use m_cmpclrbit() */
        __malloclk(&mag->freelk);
        if (lim == 1 && (mag->freemap)
            || ((mag->lim > 1) && !bitset(mag->freemap, magptrid(mag, ptr)))) {
#if (MALLOCDEBUG)
            magprint(mag);
#endif
            fprintf(stderr, "trying to free an unused block\n");
            
            abort();
        } else if (mag->cur == 1) {
            mag->freemap = (void *)(~(uintptr_t)NULL);
        } else {
            clrbit(mag->freemap, magptrid(mag, ptr));
        }
        __mallocunlk(&mag->freelk);
#endif
        if (lim == 1) {
            ndx = 0;
            mag->base = clrptr(adr);
            mag->adr = adr;
            mag->cur = 0;
        } else {
            ndx = --mag->cur;
#if (MALLOCDEBUG)
            assert(ndx >= 0);
#endif
            ((void **)mag->stk)[ndx] = adr;
        }
        if (!ndx) {
            arn = &thrarn;
            mapflg = (uintptr_t)mag->adr & MAGMAP;
            if (lim > 1 || (mapflg)) {
                bkt = mag->tab;
                magrm(mag, bkt);
                mag->arn = NULL;
                mag->tab = NULL;
            }
            if ((arn) && arn->magbuf[bktid].n < magnarnbuf(bktid)) {
                bkt = &arn->magbuf[bktid];;
                mag->arn = arn;
                magpush(mag, bkt);
            } else if ((uintptr_t)mag->adr & MAGMAP) {
                /* unmap slab */
#if (MALLOCLFDEQ)
                mapfree(mag, &g_malloc.freebuf[bktid].lfdeq);
#else
                mapfree(mag);
#endif
            } else {
                bkt = &g_malloc.freebuf[bktid];
                mag->arn = NULL;
                magpush(mag, bkt);
            }
        } else if (lim > 1 && mag->cur == lim - 1) {
            bkt = &g_malloc.magbuf[bktid];
            /* queue an unqueued earlier fully allocated magazine */
            mag->tab = bkt;
#if (MALLOCLFDEQ)
            magenqueue(mag, bkt);
#elif (MALLOCTAILQ)
            magqueue(mag, bkt, 1);
#else
            magpush(mag, bkt);
#endif
        }
#if (MALLOCPARANOIA)
        m_cmpclrbit(&mag->lk, MALLOC_LK_BIT_POS);
#endif
    }
#if (MALLOCDEBUG)
    assert(mag != NULL);
#endif
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
    uint8_t    *retptr = NULL;
    size_t      sz = 1UL << blkbktid(size);
#if (MALLOCHDRPREFIX)
    struct mag *mag = getmag(ptr);
#elif (MALLOCARRAYHASH)
    struct mag *mag = ((ptr)
                       ? hashfindmag(ptr, MALLOCHASHFIND)
                       : NULL);
#elif (MALLOCHASH)
    struct mag *mag = ((ptr)
                       ? hashfindmag(ptr)
                       : NULL);
#elif (MALLOCHDRPREFIX) && 0
    struct mag *mag = (((uintptr_t)ptr & (PAGESIZE - 1))
                       ? mtfindmag(ptr)
                       : getmag(ptr));
#else
    struct mag *mag = ((ptr) ? mtfindmag(ptr) : NULL);
#endif
    long        bktid = blkbktid(sz);
    size_t      oldsz = (mag) ? 1UL << mag->bktid : 0;

    if (!ptr) {
        retptr = _malloc(sz, 0, 0);
    } else if ((mag) && mag->bktid < bktid) {
        retptr = _malloc(sz, 0, 0);
        if (retptr) {
            memcpy(retptr, ptr, oldsz);
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
    assert(retptr != NULL);
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

    if (!size) {
#if defined(_GNU_SOURCE)
        ptr = _malloc(MALLOCMINSIZE, 0, 0);
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
    ptr = _malloc(size, 0, 0);
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
    size_t  sz = n * size;
    void   *ptr = NULL;

    if (sz < n * size) {
        /* integer overflow occurred */

        return NULL;
    }
    if (!n || !size) {
#if defined(_GNU_SOURCE)
        ptr = _malloc(MALLOCMINSIZE, 0, 1);
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
    assert(ptr != NULL);
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
    assert(retptr != NULL);
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
#if defined(GNUMALLOC) && (GNUMALLOC)
zero_aligned_alloc(size_t align,
                   size_t size)
#else
aligned_alloc(size_t align,
              size_t size)
#endif
{
    void   *ptr = NULL;

#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zmemalign_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmemalign_hook(align, size, (const void *)caller);

        return ptr;
    }
#endif
    if (!powerof2(align) || (size & (align - 1))) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, align, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
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
#if defined(GNUMALLOC) && (GNUMALLOC)
zero_posix_memalign(void **ret,
                    size_t align,
                    size_t size)
#else
posix_memalign(void **ret,
               size_t align,
               size_t size)
#endif
{
    void   *ptr = NULL;

#if (MALLOCDEBUGHOOKS) || (defined(_ZERO_SOURCE) && (MALLOCHOOKS)) && 0
    if (__zmemalign_hook) {
        void *caller = NULL;
        
        m_getretadr(caller);
        ptr = __zmemalign_hook(align, size, (const void *)caller);
        *ret = ptr;
        if (ptr) {

            return 0;
        }

        return -1;
    }
#endif
    if (!powerof2(align) || (align & (sizeof(void *) - 1))) {
        errno = EINVAL;
        *ret = NULL;

        return -1;
    } else {
        ptr = _malloc(size, align, 0);
        if (!ptr) {
            *ret = NULL;
            
            return -1;
        }
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif
    *ret = ptr;
#if (MALLOCTRACE)
    __malloctrace();
    __mallocprnttrace("posix_memalign", size, align);
#endif

    return 0;
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
#if defined(GNUMALLOC) && (GNUMALLOC)
zero_valloc(size_t size)
#else
valloc(size_t size)
#endif
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
    assert(ptr != NULL);
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
        ptr = _malloc(size, align, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
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
    assert(retptr != NULL);
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
#if defined(GNUMALLOC) && (GNUMALLOC)
zero_pvalloc(size_t size)
#else
pvalloc(size_t size)
#endif
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
    assert(ptr != NULL);
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
#if defined(GNUMALLOC) && (GNUMALLOC)
zero__aligned_malloc(size_t size,
                     size_t align)
#else
_aligned_malloc(size_t size,
                size_t align)
#endif
{
    void   *ptr = NULL;

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
        ptr = _malloc(size, align, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif
#if (MALLOCTRACE)
    __malloctrace();
    __mallocprnttrace("__aligned_malloc", size, align);
#endif

    return ptr;
}

void
#if defined(GNUMALLOC) && (GNUMALLOC)
zero__aligned_free(void *ptr)
#else
_aligned_free(void *ptr)
#endif
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

#if defined(_INTEL_SOURCE) && !defined(__GNUC__)

void *
#if defined(__GNUC__)
__attribute__ ((alloc_align(2)))
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
__attribute__ ((malloc))
#endif
#if defined(GNUMALLOC) && (GNUMALLOC)
zero__mm_malloc(size_t size,
                size_t align)
#else
_mm_malloc(size_t size,
           size_t align)
#endif
{
    void   *ptr = NULL;

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
        ptr = _malloc(size, align, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif
#if (MALLOCTRACE)
    __malloctrace();
    __mallocprnttrace("_mm_malloc", size, align);
#endif

    return ptr;
}

void
#if defined(GNUMALLOC) && (GNUMALLOC)
zero__mm_free(void *ptr)
#else
_mm_free(void *ptr)
#endif
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

#endif /* _INTEL_SOURCE && !__GNUC__ */

void
#if defined(GNUMALLOC) && (GNUMALLOC)
zero_cfree(void *ptr)
#else
cfree(void *ptr)
#endif
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
#if (MALLOCARRAYHASH)
    struct mag *mag = ((ptr)
                       ? hashfindmag(ptr, MALLOCHASHFIND)
                       : NULL);
#elif (MALLOCHASH)
    struct mag *mag = ((ptr)
                       ? hashfindmag(ptr)
                       : NULL);
#elif (MALLOCMULTITAB) || (MALLOCNEWMULTITAB)
    struct mag *mag = mtfindmag(ptr);
#elif (MALLOCHDRPREFIX)
    struct mag *mag = getmag(ptr);
#elif (MALLOCHDRPREFIX) && 0
    struct mag *mag = (((uintptr_t)ptr & (PAGESIZE - 1))
                       ? mtfindmag(ptr)
                       : getmag(ptr));
#else
    struct mag *mag0 = mtfindmag(ptr, MALLOCSLABTAB);
    struct mag *mag = (mag0) ? mag0 : mtfindmag(ptr);
#endif
#if (MALLOCHDRPREFIX)
    size_t      sz = ((mag)
                      ? ((1UL << mag->bktid)
                         - (((mag->lim == 1) || (mag->bktid >= PAGESIZELOG2))
                            ? 0
                            : (ptrdiff(ptr, getbase(ptr)))))
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
#if (MALLOCARRAYHASH)
    struct mag     *mag = ((ptr)
                           ? hashfindmag(ptr, MALLOCHASHFIND)
                           : NULL);
#elif (MALLOCHASH)
    struct mag     *mag = ((ptr)
                           ? hashfindmag(ptr)
                           : NULL);
#elif (MALLOCMULTITAB) || (MALLOCNEWMULTITAB)
    struct mag     *mag = mtfindmag(ptr);
#elif (MALLOCHDRPREFIX)
    struct mag     *mag = getmag(ptr);
#elif (MALLOCHDRPREFIX) && 0
    struct mag     *mag = (((uintptr_t)ptr & (PAGESIZE - 1))
                           ? mtfindmag(ptr)
                           : getmag(ptr));
#elif (!MALLOCHDRHACKS)
    struct mag     *mag0 = mtfindmag(ptr, MALLOCSLABTAB);
    struct mag     *mag = (mag0) ? mag0 : mtfindmag(ptr);
#endif
    size_t          sz = (mag) ? (1UL << mag->bktid) : 0;
    
    return sz;
}

#if (GNUMALLOC)

static void
gnu_malloc_init(void)
{
    mallinit();
}

void *
xmalloc(size_t size)
{
    return malloc(size);
}

void
xfree(void *ptr)
{
    free(ptr);
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

