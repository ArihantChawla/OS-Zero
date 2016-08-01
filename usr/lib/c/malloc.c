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
 * - test and fix; there's a bug about posix_memalign(), it seems
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
#define MALLOCSIG         0
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
#include <zero/asm.h>
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
#if (MALLOCLFQ)
#define LFQ_VAL_T    struct mag *
#define LFQ_VAL_NONE NULL
#include <zero/tagptr.h>
#include <zero/lfq.h>
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
    long        bktid = mag->bktid;

#if (MALLOCLFQ)
    magenqueue(mag, &g_malloc.hdrbuf[bktid]);
#else
    magpush(mag, &g_malloc.hdrbuf[bktid], 1);
#endif
    
    return;
}

static struct mag *
maggethdr(long bktid)
{
    struct mag *mag;
    void       *ret;
    struct mag *hdr;
    struct mag *last;
    long        n;
    long        lim;
    size_t      sz;
    uint8_t    *ptr;

#if (MALLOCLFQ)
    mag = magdequeue(&g_malloc.hdrbuf[bktid]);
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
#if (MALLOCLFQ)
            tagptrinitadr(last, hdr->node.prev);
            if (last) {
                tagptrinitadr(hdr, last->node.next);
            } else {
                tagptrinitadr(hdr, mag->node.next);
                tagptrinitadr(mag, hdr->node.prev);
            }
#else
            hdr->prev = last;
            if (last) {
                last->next = hdr;
            } else {
                mag->next = hdr;
                hdr->prev = mag;
            }
#endif
            last = hdr;
        }
#if (MALLOCLFQ)
        hdr = tagptrgetadr(mag->node.next);
#else
        hdr = mag->next;
#endif
        if (hdr) {
#if (MALLOCLFQ)
            lim = n;
            while (lim--) {
                magenqueue(hdr, &g_malloc.hdrbuf[bktid]);
                hdr = tagptrgetadr(hdr->node.next);
            }
#else
            magpushmany(hdr, last, &g_malloc.hdrbuf[bktid], 1, n);
#endif
        }
    }
#if (MALLOCLFQ)
    tagptrinitadr(NULL, mag->node.prev);
    tagptrinitadr(NULL, mag->node.next);
#endif
    if (mag) {
        mag->prev = NULL;
        mag->next = NULL;
    }
    
    return mag;
}

static void *
maginit(struct mag *mag, long bktid, long *zeroret)
{
    if (!mag->stk) {
        maginittab(mag, bktid);
    }
    if (!mag->base) {
        if (!maginitslab(mag, bktid, zeroret)) {
            
            return NULL;
        }
    }

    return mag;
}

static struct mag *
magget(long bktid, long *zeroret)
{
    struct mag *mag = maggethdr(bktid);
    
    if (mag) {
        if (!maginit(mag, bktid, zeroret)) {
            
            return NULL;
        }
#if (MALLOCLFQ)
        tagptrinitadr(NULL, mag->node.prev);
        tagptrinitadr(NULL, mag->node.next);
#endif
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
        mag = arn->magbuf[bktid].ptr;
        if (mag) {
#if (MALLOCBUFMAG)
            n++;
#endif
            first = mag;
            mag->arn = NULL;
#if (MALLOCLFQ)
            while (mag->next) {
                mag = mag->next;
#if (MALLOCBUFMAG)
                n++;
#endif
                mag->arn = NULL;
                tail = mag;
            }
#else
            while (mag->next) {
                mag = mag->next;
#if (MALLOCBUFMAG)
                n++;
#endif
                mag->arn = NULL;
                tail = mag;
            }
#endif /* MALLOCLFQ */
            mag = first;
            if (tail) {
#if (MALLOCLFQ)
                while (mag != tail) {
                    magenqueue(mag, &g_malloc.magbuf[bktid]);
                    mag = mag->next;
                }
                /* tail */
                magenqueue(mag, &g_malloc.magbuf[bktid]);
#else
                magpushmany(first, tail, &g_malloc.magbuf[bktid], 1, n);
#endif
            } else {
#if (MALLOCLFQ)
                magenqueue(mag, &g_malloc.magbuf[bktid]);
#else
                magpush(mag, &g_malloc.magbuf[bktid], 1);
#endif
            }
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
maginitslab(struct mag *mag, long bktid, long *zeroret)
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
            __malloclk(&g_malloc.heaplk);
            ptr = growheap(rounduppow2(n * sz, PAGESIZE));
            __mallocunlk(&g_malloc.heaplk);
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
    
static void
prefork(void)
{
    long ndx;

    __malloclk(&g_malloc.initlk);
    __malloclk(&g_malloc.heaplk);
    for (ndx = 0 ; ndx < MALLOCNBKT ; ndx++) {
//        __malloclk(&g_malloc.freebuf[ndx].lk);
    }
#if (!MALLOCHASH) && !(MALLOCNEWMULTITAB)
#if (MALLOCMULTITAB)
    for (ndx = 0 ; ndx < PAGEDIRNL1KEY ; ndx++) {
#if (MALLOCSPINLOCKS)
        __malloclkspin(&g_malloc.pagedirlktab[ndx]);
//        __malloclkspin(&g_malloc.slabdirlktab[ndx]);
#else
        __malloclk(&g_malloc.pagedirlktab[ndx]);
//        __malloclk(&g_malloc.slabdirlktab[ndx]);
#endif
    }
#endif
#endif
    
    return;
}

static void
postfork(void)
{
    long ndx;

#if (!MALLOCHASH) && !(MALLOCNEWMULTITAB)
#if (MALLOCMULTITAB)
    for (ndx = 0 ; ndx < PAGEDIRNL1KEY ; ndx++) {
#if (MALLOCSPINLOCKS)
        __mallocunlkspin(&g_malloc.pagedirlktab[ndx]);
#else
        __mallocunlk(&g_malloc.pagedirlktab[ndx]);
#endif
    }
#if 0
    for (ndx = 0 ; ndx < SLABDIRNL1KEY ; ndx++) {
#if (MALLOCSPINLOCKS)
        __mallocunlkspin(&g_malloc.slabdirlktab[ndx]);
#else
        __mallocunlk(&g_malloc.slabdirlktab[ndx]);
#endif
    }
#endif /* 0 */
#endif
#endif
    __mallocunlk(&g_malloc.heaplk);
    __mallocunlk(&g_malloc.initlk);
    
    return;
}

#if (MALLOCHASH)

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
    if (!head) {
        /* allocate a page's worth of hash table magazine chain entries */
        n = PAGESIZE / sizeof(struct hashmag);
        item = mapanon(g_malloc.zerofd, PAGESIZE);
        if (item == MAP_FAILED) {
            abort();
        }
        /* chain entries */
        prev = item;
        cur = item;
        while (--n) {
            cur++;
            prev->next = cur;
            prev = cur;
        }
    } else {
        item = head;
    }
    /* add item->next to the head of queue; the head will be unlocked */
    m_atomwrite((volatile long *)hptr,
                item->next);

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
    item->upval = 0;
    item->adr = NULL;
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
    uintptr_t        upval;
    uintptr_t        upage = (uintptr_t)ptr >> PAGESIZELOG2;
    struct mag      *mag;
    struct hashmag  *orig;
    struct hashmag  *cur;
    struct hashmag  *prev;
    struct hashmag  *head;
    struct hashmag **hptr;
    long             res;
    unsigned long    key;

#if (MALLOCNEWHASH) && 0
    key = hashq128uptr(upval, MALLOCNHASHBIT);
#else
    key = upage & ((1UL << (MALLOCNHASHBIT)) - 1);
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
    while (cur) {
        if (cur->upval == upage) {
            mag = cur->adr;
            m_cmpclrbit((volatile long *)hptr,
                        MALLOC_HASH_MARK_POS);
            
            return mag;
        }
        prev = cur;
        cur = cur->next;
    }
    m_cmpclrbit((volatile long *)hptr,
                MALLOC_HASH_MARK_POS);
    
    return NULL;
}

static struct mag *
hashsetmag(void *ptr, struct mag *mag)
{
    uintptr_t        upval;
    uintptr_t        upage = (uintptr_t)ptr >> PAGESIZELOG2;
    struct hashmag  *head;
    struct hashmag **hptr;
    struct hashmag  *cur = NULL;
    struct hashmag  *item;
    struct hashmag  *orig;
    struct hashmag  *prev;
    long             res;
    unsigned long    key;

#if (MALLOCNEWHASH) && 0
    key = hashq128uptr(upval, MALLOCNHASHBIT);
#else
    key = upage & ((1UL << (MALLOCNHASHBIT)) - 1);
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
        /* look for an existing item to update */
        if (cur->upval == upage) {
            if (mag) {
//                m_atominc(&cur->nref);
                cur->nref++;
                m_cmpclrbit((volatile long *)hptr,
                            MALLOC_HASH_MARK_POS);
                
                return mag;
            } else if (!--cur->nref) {
                m_atomwrite((volatile long *)hptr, cur->next);
                hashbufitem(cur);
                
                return NULL;
            } else {
                m_cmpclrbit((volatile long *)hptr,
                            MALLOC_HASH_MARK_POS);
                
                return mag;
            }
        } else {
            prev = cur;
            cur = cur->next;
            while (cur) {
                if (cur->upval == upage) {
                    if (mag) {
//                m_atominc(&cur->nref);
                        cur->nref++;
                        m_cmpclrbit((volatile long *)hptr,
                                    MALLOC_HASH_MARK_POS);
                        
                        return mag;
                    } else if (!--cur->nref) {
                        prev->next = cur->next;
                        m_cmpclrbit((volatile long *)hptr,
                                    MALLOC_HASH_MARK_POS);
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
    }
    if (!cur && (mag)) {
        item = hashgetitem();
        if (!item) {
            abort();
        }
        item->nref = 1;
        item->upval = upage;
        item->adr = mag;
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

#elif defined(MALLOCNEWMULTITAB) && (MALLOCNEWMULTITAB)

static struct mag *
mtfindmag(void *ptr)
{
    uintptr_t      l1;
    uintptr_t      l2;
    uintptr_t      l3;
    uintptr_t      upval1;
    uintptr_t      upval2;
    uintptr_t      upval3;
    struct memtab *mptr1;
    struct memtab *mptr2;
    struct memtab *mptr3;
    struct mag    *mag = NULL;
    
    l1 = pagedirl1ndx(ptr);
    l2 = pagedirl2ndx(ptr);
    l3 = pagedirl3ndx(ptr);
    do {
        if (mttrylktab(&g_malloc.pagedir[l1])) {
            upval1 = (uintptr_t)g_malloc.pagedir[l1].ptr;
            upval1 &= MALLOC_TAB_LK_BIT;
            mptr1 = (void *)(upval1 & MALLOC_TAB_LK_BIT);
            if (mptr1) {
                if (mttrylktab(mptr1)) {
                    upval2 = (uintptr_t)mptr1->ptr;
                    upval2 &= ~MALLOC_TAB_LK_BIT;
                    mptr2 = (void *)upval2;
                    if (mptr2) {
                        if (mttrylktab(&mptr2[l2])) {
                            upval3 = (uintptr_t)(mptr2[l2].ptr);
                            upval3 &= ~MALLOC_TAB_LK_BIT;
                            mptr3 = (void *)upval3;
                            if (mptr3) {
                                mptr3 = mptr1->ptr;
                                if (mptr2) {
                                    mag = ((void **)mptr2)[l3];
                                }
                            }
                            mtunlktab(&mptr2[l2]);
                            mtunlktab(mptr1);
                            mtunlktab(&g_malloc.pagedir[l1]);

                            return mag;
                        } else {
                            mtunlktab(mptr1);
                            mtunlktab(&g_malloc.pagedir[l1]);
                            
                            return NULL;
                        }
                    } else {
                        mtunlktab(mptr1);
                        mtunlktab(&g_malloc.pagedir[l1]);

                        return NULL;
                    }
                } else {
                    mtunlktab(&g_malloc.pagedir[l1]);

                    return NULL;
                }
            } else {
                mtunlktab(mptr1);
            }
        } else {

            return NULL;
        }
    } while (1);

    return mag;
}
    
static void
mtsetmag(void *ptr,
         struct mag *mag)
{
    uintptr_t       l1;
    uintptr_t       l2;
    uintptr_t       l3;
    uintptr_t       upval1;
    uintptr_t       upval2;
    uintptr_t       upval3;
    long            fail = 0;
    struct memtab  *mptr1;
    struct memtab  *mptr2;
    struct memtab  *mptr3;
    struct memtab  *ptab[3] = { NULL, NULL, NULL };
    
    l1 = pagedirl1ndx(ptr);
    l2 = pagedirl2ndx(ptr);
    l3 = pagedirl3ndx(ptr);
    if (mag) {
        do {
            if (mttrylktab(&g_malloc.pagedir[l1])) {
                upval1 = (uintptr_t)g_malloc.pagedir[l1].ptr;
                upval1 &= ~MALLOC_TAB_LK_BIT;
                mptr1 = (void *)upval1;
                if (!mptr1) {
                    mptr1 = mapanon(g_malloc.zerofd,
                                    PAGEDIRNL2KEY * sizeof(struct memtab));
                    if (mptr1 != MAP_FAILED) {
                        upval2 = (uintptr_t)mptr1;
                        ptab[0] = mptr1;
                        upval2 |= MALLOC_TAB_LK_BIT;
                        mptr1->nref++;
                        g_malloc.pagedir[l1].ptr = (void *)upval2;
#if defined(MALLOCVALGRINDTABS)
                        VALGRINDALLOC(mptr1,
                                      PAGEDIRNL2KEY * sizeof(struct memtab), 1);
#endif
#if (MALLOCSTAT)
                        ntabbyte += PAGEDIRNL2KEY * sizeof(struct memtab);
#endif
                        mptr2 = mptr1->ptr;
                        if (!mptr2) {
                            mptr2 = mapanon(g_malloc.zerofd,
                                            PAGEDIRNL3KEY * sizeof(void *));
#if (MALLOCSTAT)
                            ntabbyte += PAGEDIRNL3KEY * sizeof(void *);
#endif
                        }
                        if (mptr2 != MAP_FAILED) {
                            upval1 = (uintptr_t)mptr2;
                            ptab[1] = mptr2;
                            upval1 |= MALLOC_TAB_LK_BIT;
                            mptr1->nref++;
                            mptr1->ptr = mptr2;
#if defined(MALLOCVALGRINDTABS)
                            VALGRINDALLOC(mptr2,
                                          PAGEDIRNL3KEY * sizeof(void *), 1);
#endif
#if (MALLOCSTAT)
                            ntabbyte += PAGEDIRNL3KEY * sizeof(void *);
#endif
                        } else {
                            fail = 1;
                        }
                    } else {
                        fail = 1;
                    }
                }
            }
        } while (1);
    }
    if (!mag && !fail) {
        mptr1 = g_malloc.pagedir[l1].ptr;
        upval1 = (uintptr_t)mptr1;
        upval1 &= ~MALLOC_TAB_LK_BIT;
        if (upval1) {
            mptr2 = (void *)upval1;
            if (mptr2) {
                mptr1 = ((void **)mptr2)[l2];
                upval2 = (uintptr_t)mptr1;
                mptr2 = (void *)upval2;
                upval2 &= ~MALLOC_TAB_LK_BIT;
                if (upval2) {
                    ptab[0] = mptr2;
                    upval3 = (uintptr_t)mptr2->ptr;
                    upval3 &= ~MALLOC_TAB_LK_BIT;
                    mptr3 = (void *)upval3;
                    if (upval3) {
                        if (mptr3) {
                            mptr1 = ((void **)mptr3)[l3];
                            if (mptr1) {
                                ptab[1] = mptr3;
                            }
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
            mtunlktab(mptr2);
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
            mtunlktab(mptr1);
            mtunlktab(&g_malloc.pagedir[l1]);
        }
    }

    return;
}

#elif (MALLOCMULTITAB)

#if defined(MALLOCFREETABS) && (MALLOCFREETABS)

static struct mag *
mtfindmag(void *ptr)
{
    uintptr_t      l1;
    uintptr_t      l2;
    uintptr_t      l3;
    struct memtab *mptr1;
    struct memtab *mptr2;
    struct mag    *mag = NULL;
    
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

    return mag;
}
    
static void
mtsetmag(void *ptr,
         struct mag *mag)
{
    uintptr_t       l1;
    uintptr_t       l2;
    uintptr_t       l3;
    long            fail = 0;
    struct memtab  *mptr1;
    struct memtab  *mptr2;
    struct memtab  *ptab[3] = { NULL, NULL, NULL };
    
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
#if defined(GNUMALLOC) && (GNUMALLOC) && 0
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
#if (!MALLOCTKTLK)
#if (MALLOCSPINLOCKS)
        __mallocinitspin(&g_malloc.hdrbuf[ndx].lk);
#else
        __mallocinitlk(&g_malloc.hdrbuf[ndx].lk);
#endif
        __mallocinitlk(&g_malloc.magbuf[ndx].lk);
#endif
//        __mallocinitlk(&g_malloc.freebuf[bktid].lk);
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
#if (MALLOCHASH) && !(MALLOCMULTITAB) && !(MALLOCNEWMULTITAB)
    g_malloc.maghash = mapanon(g_malloc.zerofd,
                               MALLOCNHASHITEM * sizeof(struct hashmag *));
    if (!g_malloc.maghash) {
        abort();
    }
#elif (MALLOCFREETABS)
    g_malloc.pagedir = mapanon(g_malloc.zerofd,
                               PAGEDIRNL1KEY * sizeof(struct memtab));
    if (g_malloc.pagedir == MAP_FAILED) {
        abort();
    }
#if (MALLOCSTAT)
    ntabbyte += PAGEDIRNL1KEY * sizeof(struct memtab);
#endif
#else /* !MALLOCHASH && !MALLOCFREETABS */
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
#if (MALLOCPTRNDX)
    PTRNDX      ndx = 0;
#endif
    struct mag *mag = NULL;
    struct arn *arn;
    uint8_t    *adr;
    uint8_t    *ptr = NULL;
    size_t      sz = align <= MALLOCALIGNMENT ? size : size + align - 1;
#if 0
    size_t      sz = ((size < PAGESIZE)
                      ? ((align)
                         ? (size + align)
                         : (size + MALLOCALIGNMENT))
                      : (size
                         + ((align <= PAGESIZE)
                            ? 0
                            : rounduppow2(size + align, PAGESIZE))));
#endif
    long        bktid = blkbktid(sz);
    long        lim;
#if (MALLOCHDRHACKS)
    uint8_t     nfo = bktid;
#endif
    
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
    if (arn) {
        mag = arn->magbuf[bktid].ptr;
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
                arn->magbuf[bktid].n--;
#endif
                magrmhead(&arn->magbuf[bktid], mag);
                mag->prev = NULL;
                mag->next = NULL;
            }
        }
    }
    if (!mag) {
#if (MALLOCLFQ)
        mag = magdequeue(&g_malloc.magbuf[bktid]);
#else
        magpop(&g_malloc.magbuf[bktid], mag, 1);
#endif
        if (!mag) {
            mag = magdequeue(&g_malloc.magbuf[bktid]);
        }
        if (!mag) {
            mag = magget(bktid, &zero);
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
                arn->magbuf[bktid].n++;
#endif
                magpush(mag, &arn->magbuf[bktid], 0);
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
#if (MALLOCHASH)
        if ((align) && ((uintptr_t)ptr & (align - 1))) {
            ptr = ptralign(ptr, align);
        }
        hashsetmag(ptr, mag);
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
            mtsetmag(ptr, mag);
        }
#else /* !MALLOCHDRPREFIX */
        /* store unaligned source pointer and mag address */
        if (align) {
            if ((uintptr_t)ptr & (align - 1)) {
                ptr = ptralign(ptr, align);
            }
        }
        mtsetmag(ptr, mag);
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
        __malloclk(&mag->freelk);
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
    
    return ptr;
}

#define MALLOCUNMAPFREQ (1L << 20)

void
_free(void *ptr)
{
    struct arn     *arn;
    struct mag     *mag = NULL;
    void           *adr = NULL;
    long            bktid;
    long            lim;
#if (MALLOCLAZYUNMAP)
    static long     nfree = 0;
#endif
#if (MALLOCFREEMAP)
    long            ndx;
#endif
#if (MALLOCHDRHACKS)
    uint8_t         nfo = 0;
#endif
#if (MALLOCPTRNDX)
    PTRNDX          ndx;
#endif
    
    if (!ptr) {

        return;
    }
//    arn = &thrarn;
    if (!(thrflg & MALLOCINIT)) {
        thrinit();
        thrflg |= MALLOCINIT;
    }
#if (MALLOCHASH)
    mag = hashfindmag(ptr);
#elif (MALLOCHDRPREFIX)
    if ((uintptr_t)ptr & (PAGESIZE - 1)) {
        mag = getmag(ptr);
    } else {
        mag = mtfindmag(ptr);
    }
#if (MALLOCHDRHACKS)
    nfo = getnfo(ptr);
#endif
#endif
    if (mag) {
        arn = mag->arn;
#if (MALLOCHASH)
        hashsetmag(ptr, NULL);
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
        __malloclk(&mag->freelk);
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
        __mallocunlk(&mag->freelk);
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
            mag->stk[--mag->cur] = ndx;
#else
            ((void **)mag->stk)[--mag->cur] = adr;
#endif
        }
        lim = mag->lim;
        if (!mag->cur) {
            if (lim > 1) {
                magrm(mag, &arn->magbuf[bktid], 1);
            }
#if (MALLOCLFQ) && 0
            if ((uintptr_t)mag->adr & MAGMAP) {
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
//                mag->arn = arn;
                magpush(mag, &arn->magbuf[bktid], 0);
            }
#else /* !MALLOCLFQ */
            if (arn->magbuf[bktid].n < magnarnbuf(bktid)) {
//                mag->arn = arn;
                magpush(mag, &arn->magbuf[bktid], 0);
            } else if ((uintptr_t)mag->adr & MAGMAP) {
                /* unmap slab */
                adr = (void *)mag->base;
                VALGRINDRMPOOL(adr);
#if (MALLOCLAZYUNMAP)
                nfree = m_fetchadd(&nfree, 1);
                if (nfree == MALLOCUNMAPFREQ - 1) {
                    nfree = 0;
                    do {
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
                        mag = magdequeue(&g_malloc.mapbuf[bktid]);
                    } while (mag);
                } else {
                    magenqueue(mag, &g_malloc.mapbuf[bktid]);
                }
#else
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
                magpush(mag, &arn->magbuf[bktid], 0);
            }
#endif /* MALLOCLFQ */
        } else if (mag->cur == lim - 1) {
                /* queue an unqueued earlier fully allocated magazine */
#if (MALLOCLFQ)
            magenqueue(mag, &g_malloc.magbuf[bktid]);
#else
            magpush(mag, &g_malloc.magbuf[bktid], 1);
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
    uint8_t    *retptr = NULL;
    size_t      sz = 1UL << blkbktid(size);
#if (MALLOCHASH)
    struct mag *mag = ((ptr)
                       ? hashfindmag(ptr)
                       : NULL);
#elif (MALLOCHDRPREFIX)
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
    _assert(ptr != NULL);
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
    _assert(ptr != NULL);
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

#if defined(_INTEL_SOURCE)

void *
#if defined(__GNUC__)
__attribute__ ((alloc_align(2)))
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
__attribute__ ((malloc))
#endif
#if defined(GNUMALLOC) && (GNUMALLOC)
zero__mm_malloc(int size,
                int align)
#else
_mm_malloc(int size,
           int align)
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
    _assert(ptr != NULL);
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

#endif /* _INTEL_SOURCE */

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
#if (MALLOCHASH)
    struct mag *mag = ((ptr)
                       ? hashfindmag(ptr)
                       : NULL);
#elif (MALLOCMULTITAB) || (MALLOCNEWMULTITAB)
    struct mag *mag = mtfindmag(ptr);
#elif (MALLOCHDRPREFIX)
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
#if (MALLOCHASH)
    struct mag     *mag = ((ptr)
                           ? hashfindmag(ptr)
                           : NULL);
#elif (MALLOCMULTITAB) || (MALLOCNEWMULTITAB)
    struct mag     *mag = mtfindmag(ptr);
#elif (MALLOCHDRPREFIX)
    struct mag     *mag = (((uintptr_t)ptr & (PAGESIZE - 1))
                           ? mtfindmag(ptr)
                           : getmag(ptr));
#elif (!MALLOCHDRHACKS)
    struct mag     *mag0 = mtfindmag(ptr, MALLOCSLABTAB);
    struct mag     *mag = (mag0) ? mag0 : mtfindmag(ptr);
#endif
#if (MALLOCHDRHACKS)
    size_t          sz = 1UL << (getnfo(ptr) & MEMHDRBKTMASK);
#else
    size_t          sz = (mag) ? (1UL << mag->bktid) : 0;
#endif
    
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

