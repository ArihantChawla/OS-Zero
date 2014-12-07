/*
 * Zero Malloc Revision 2
 *
 * Copyright Tuomo Petteri Venäläinen 2014
 */

#define MALLOCWIDELOCK 0

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
 *          - supports both heap and mapped regions
 *
 *        heap
 *        ----
 *        - process heap segment
 *          - sbrk() interface; needs global lock
 *
 *        map
 *        ---
 *        - process map segment
 *          - mmap() interface; thread-safe
 *
 *        headers
 *        -------
 *        - mapped internal book-keeping for magazines, e.g. pointer stacks
 */

#define GNUMALLOCHOOKS 1

//#include <assert.h>

#include <features.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <malloc.h>

//#include <sys/sysinfo.h>

#define ZEROMTX 1
#if defined(PTHREAD) && (PTHREAD)
#include <pthread.h>
#endif
#if defined(ZEROMTX) && (ZEROMTX)
#define MUTEX volatile long
#include <zero/mtx.h>
#elif (PTHREAD)
#define MUTEX pthread_mutex_t
#endif
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/trix.h>

//#define MALLOCNARN     (2 * get_nprocs_conf())
//#define MALLOCNARN     (2 * sysconf(_SC_NPROCESSORS_CONF))
#define MALLOCNARN     16
#define MALLOCSLABLOG2 20
#define MALLOCMINSIZE  (1UL << MALLOCMINLOG2)
#define MALLOCMINLOG2  CLSIZELOG2
#define MALLOCNBKT     PTRBITS

#define MAGMAP         0x01
#define MAGFLGMASK     MAGMAP
#define MALLOCMAGSIZE  PAGESIZE
#define MAGGLOBAL      0x0001
#define magembedstk(bktid) (nbstk(bktid) <= MALLOCMAGSIZE - offsetof(struct mag, data))
struct mag {
    void        *adr;           // slab base address
    long         cur;           // current pointer stack index
    long         max;           // # of items on pointer stack
    long         arnid;         // arena ID
    long         bktid;         // bucket ID
    struct mag  *prev;          // previous in chain
    struct mag  *next;          // next in chain
#if (MAGFREEMAP)
    uint8_t     *freemap;
#endif
    void       **stk;           // pointer stack
    void       **ptrtab;        // original pointers for allocation (align etc.)
    uint8_t      data[EMPTY];   // internal pointer stack where present
};

struct maglist {
    MUTEX       lk;             // mutual exclusion lock
#if (MALLOCLISTCNT)
    long        n;              // number of magasines on list
#endif
    struct mag *head;           // first magasine on list
#if (MALLOCTAIL)
    struct mag *tail;           // last magasine on list
#endif
};

#define MALLOCARNSIZE      rounduppow2(sizeof(struct arn), PAGESIZE)
struct arn {
    struct maglist magtab[MALLOCNBKT];  // partially allocated magasines
    struct maglist freetab[MALLOCNBKT]; // totally unallocated magasines
    struct maglist hdrtab[MALLOCNBKT];  // cached magasine headers
    long        nref;                   // number of threads using the arena
    MUTEX       nreflk;                 // lock for updating nref
};

#define MALLOCINIT 0x00000001L
struct malloc {
    struct maglist  magtab[MALLOCNBKT];
    struct maglist  freetab[MALLOCNBKT];
    struct arn    **arntab;
    void          **mdir;
    MUTEX           initlk;
    MUTEX           heaplk;
    pthread_key_t   arnkey;
    long            narn;
    long            flags;
    int             zerofd;
};

static struct malloc g_malloc ALIGNED(PAGESIZE);
__thread long        _arnid = -1;
MUTEX                _arnlk;
volatile long        nextarnid;

#define BLKDIRTY    0x01
#define BLKFLGMASK  (MALLOCMINSIZE - 1)
#define clrptr(ptr) ((void *)((uintptr_t)ptr & ~BLKFLGMASK))

#define nblklog2(bktid)                                                 \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? (MALLOCSLABLOG2 - (bktid))                                       \
     : 0)

#define nbstk(bktid) ((1UL << (nblklog2(bktid) + 1)) * sizeof(void *))
#define nbmag(bktid) (1UL << ((bktid) + nblklog2(bktid)))

#define ptralign(ptr, pow2)                                             \
    (!((uintptr_t)ptr & (align - 1))                                    \
     ? ptr                                                              \
     : ((void *)rounduppow2((uintptr_t)ptr, align)))
#define blkalignsz(sz, aln)                                             \
    (((aln) <= MALLOCMINSIZE)                                           \
     ? max(sz, aln)                                                     \
     : (sz) + (aln))

#define magptrid(mag, ptr)                                              \
    (((uintptr_t)(ptr) - ((uintptr_t)(mag)->adr & ~MAGFLGMASK)) >> (mag)->bktid)
#define magputptr(mag, ptr1, ptr2)                                      \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr1)] = (ptr2))
#define maggetptr(mag, ptr)                                             \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr)])

#define l1ndx(ptr) (((uintptr_t)ptr >> L1NDX) & ((1 << NL1BIT) - 1))
#define l2ndx(ptr) (((uintptr_t)ptr >> L2NDX) & ((1 << NL2BIT) - 1))
#define l3ndx(ptr) (((uintptr_t)ptr >> L3NDX) & ((1 << NL3BIT) - 1))

#define NL1KEY     (1UL << NL1BIT)
#define NL2KEY     (1UL << NL2BIT)
#define NL3KEY     (1UL << NL3BIT)
#define L1NDX      (L2NDX + NL2BIT)
#define L2NDX      (L3NDX + NL3BIT)
#define L3NDX      MALLOCMINLOG2
#define NL1BIT     16
#if (PTRBITS == 32)
#define NL2BIT     (32 - NL1BIT - MALLOCMINLOG2)
#else
#define NL2BIT     16
#endif
#define NL3BIT    (ADRBITS - NL1BIT - NL2BIT - MALLOCMINLOG2)

long
thrarnid(void)
{
    struct arn *arn;
    
    if (_arnid >= 0) {

        return _arnid;
    }
    mtxlk(&_arnlk);
    _arnid = nextarnid++;
    arn = g_malloc.arntab[_arnid];
    mtxlk(&arn->nreflk);
    arn->nref++;
    mtxunlk(&arn->nreflk);
    nextarnid &= (MALLOCNARN - 1);
    pthread_setspecific(g_malloc.arnkey, g_malloc.arntab[_arnid]);
    mtxunlk(&_arnlk);

    return _arnid;
}

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
findmag(void *ptr)
{
    uintptr_t  l1 = l1ndx(ptr);
    uintptr_t  l2 = l2ndx(ptr);
    uintptr_t  l3 = l3ndx(ptr);
    void       *ptr1;
    void       *ptr2;
    struct mag *mag = NULL;

    ptr1 = g_malloc.mdir[l1];
    if (ptr1) {
        ptr2 = ((void **)ptr1)[l2];
        if (ptr2) {
            mag = ((struct mag **)ptr2)[l3];
        }
    }

    return mag;
}

static void
setmag(void *ptr,
       struct mag *mag)
{
    uintptr_t    l1 = l1ndx(ptr);
    uintptr_t    l2 = l2ndx(ptr);
    uintptr_t    l3 = l3ndx(ptr);
    void        *ptr1;
    void        *ptr2;
    void       **pptr;
    struct mag **item;

    ptr1 = g_malloc.mdir[l1];
    if (!ptr1) {
        g_malloc.mdir[l1] = ptr1 = mapanon(g_malloc.zerofd,
                                           NL2KEY * sizeof(void *));
        if (ptr1 == MAP_FAILED) {
#ifdef ENOMEM
            errno = ENOMEM;
#endif

            exit(1);
        }
    }
    pptr = ptr1;
    ptr2 = pptr[l2];
    if (!ptr2) {
        pptr[l2] = ptr2 = mapanon(g_malloc.zerofd,
                                  NL3KEY * sizeof(struct mag *));
        if (ptr2 == MAP_FAILED) {
#ifdef ENOMEM
            errno = ENOMEM;
#endif

            exit(1);
        }
    }
    item = &((struct mag **)ptr2)[l3];
    *item = mag;

    return;
}

static void
prefork(void)
{
    struct arn *arn;
    long        arnid;
    long        bktid;

    mtxlk(&g_malloc.initlk);
    mtxlk(&g_malloc.heaplk);
    for (arnid = 0 ; arnid < MALLOCNARN ; arnid++) {
        arn = g_malloc.arntab[arnid];
        for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
            mtxlk(&arn->magtab[bktid].lk);
            mtxlk(&arn->freetab[bktid].lk);
            mtxlk(&arn->hdrtab[bktid].lk);
        }
    }
    for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
        mtxlk(&g_malloc.magtab[bktid].lk);
        mtxlk(&g_malloc.freetab[bktid].lk);
    }
    
    return;
}

static void
postfork(void)
{
    struct arn *arn;
    long        arnid;
    long        bktid;

    for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
        mtxunlk(&g_malloc.freetab[bktid].lk);
        mtxunlk(&g_malloc.magtab[bktid].lk);
    }
    for (arnid = 0 ; arnid < MALLOCNARN ; arnid++) {
        arn = g_malloc.arntab[arnid];
        for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
            mtxunlk(&arn->hdrtab[bktid].lk);
            mtxunlk(&arn->freetab[bktid].lk);
            mtxunlk(&arn->magtab[bktid].lk);
        }
    }
    mtxunlk(&g_malloc.heaplk);
    mtxunlk(&g_malloc.initlk);
    
    return;
}

static void
freearn(void *arg)
{
    struct arn *arn = arg;
    struct mag *mag;
    struct mag *head;
    long        bktid;

    mtxlk(&arn->nreflk);
    arn->nref--;
    if (!arn->nref) {
        for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
            mtxlk(&arn->magtab[bktid].lk);
            head = arn->magtab[bktid].head;
            if (head) {
                mag = head;
                while (mag->next) {
                    mag = mag->next;
                }
                mtxlk(&g_malloc.magtab[bktid].lk);
                mag->next = g_malloc.magtab[bktid].head;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                g_malloc.magtab[bktid].head = head;
                mtxunlk(&g_malloc.magtab[bktid].lk);
            }
            arn->magtab[bktid].head = NULL;
            mtxunlk(&arn->magtab[bktid].lk);
            mtxlk(&arn->freetab[bktid].lk);
            head = arn->freetab[bktid].head;
            if (head) {
                mag = head;
                while (mag->next) {
                    mag = mag->next;
                }
                mtxlk(&g_malloc.freetab[bktid].lk);
                mag->next = g_malloc.freetab[bktid].head;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                g_malloc.freetab[bktid].head = head;
                mtxunlk(&g_malloc.freetab[bktid].lk);
            }
            arn->freetab[bktid].head = NULL;
            mtxunlk(&arn->freetab[bktid].lk);
        }
    }
    mtxunlk(&arn->nreflk);

    return;
}

static void
mallinit(void)
{
    long        narn;
    long        arnid;
    long        bktid;
    long        ofs;
    uint8_t    *ptr;

    mtxlk(&g_malloc.initlk);
    if (g_malloc.flags & MALLOCINIT) {
        mtxunlk(&g_malloc.initlk);
        
        return;
    }
    narn = MALLOCNARN;
#if (MMAP_DEV_ZERO)
    g_malloc.zerofd = open("/dev/zero", O_RDWR);
#endif
    g_malloc.arntab = mapanon(g_malloc.zerofd,
                              narn * sizeof(struct arn **));
    ptr = mapanon(g_malloc.zerofd, narn * MALLOCARNSIZE);
    arnid = narn;
    while (arnid--) {
        g_malloc.arntab[arnid] = (struct arn *)ptr;
        ptr += MALLOCARNSIZE;
    }
#if (ARNREFCNT)
    g_malloc.arnreftab = mapanon(_mapfd, NARN * sizeof(unsigned long));
    g_malloc.arnreflktab = mapanon(_mapfd, NARN * sizeof(MUTEX));
#endif
    arnid = narn;
    while (arnid--) {
        for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
            mtxinit(&g_malloc.arntab[arnid]->magtab[bktid].lk);
            mtxinit(&g_malloc.arntab[arnid]->freetab[bktid].lk);
            mtxinit(&g_malloc.arntab[arnid]->hdrtab[bktid].lk);
        }
    }
    g_malloc.narn = narn;
    bktid = MALLOCNBKT;
    while (bktid--) {
        mtxinit(&g_malloc.freetab[bktid].lk);
        mtxinit(&g_malloc.magtab[bktid].lk);
    }
    mtxlk(&g_malloc.heaplk);
    ofs = PAGESIZE - ((long)growheap(0) & (PAGESIZE - 1));
    if (ofs != PAGESIZE) {
        growheap(ofs);
    }
    mtxunlk(&g_malloc.heaplk);
    g_malloc.mdir = mapanon(g_malloc.zerofd, NL1KEY * sizeof(void *));
#if defined(_GNU_SOURCE) && (GNUMALLOCHOOKS)
    if (__malloc_initialize_hook) {
        __malloc_initialize_hook();
    }
#endif
    pthread_key_create(&g_malloc.arnkey, freearn);
    pthread_atfork(prefork, postfork, postfork);
    g_malloc.flags |= MALLOCINIT;
    mtxunlk(&g_malloc.initlk);

    return;
}

void *
_malloc(size_t size,
        size_t align,
        long zero)
{
    struct arn  *arn;
    struct mag  *mag;
    uint8_t     *ptr;
    uint8_t     *ptrval;
    void       **stk = NULL;
    long         arnid;
    long         sz = max(blkalignsz(size, align), MALLOCMINSIZE);
    long         bktid = blkbktid(sz);
    long         mapped = 0;
    long         max;
    long         n;
    size_t       incr;

    if (!(g_malloc.flags & MALLOCINIT)) {
        mallinit();
    }
    arnid = thrarnid();
    arn = g_malloc.arntab[arnid];
    /* try to allocate from a partially used magazine */
    mtxlk(&arn->magtab[bktid].lk);
    mag = arn->magtab[bktid].head;
    if (mag) {
        ptrval = mag->stk[mag->cur++];
        if (mag->cur == mag->max) {
            /* remove fully allocated magazine from partially allocated list */
            if (mag->next) {
                mag->next->prev = NULL;
            }
            arn->magtab[bktid].head = mag->next;
            mag->prev = NULL;
            mag->next = NULL;
        }
        mtxunlk(&arn->magtab[bktid].lk);
    } else {
        /* try to allocate from list of free magazines with no allocations */
#if !(MALLOCWIDELOCK)
        mtxunlk(&arn->magtab[bktid].lk);
#endif
        mtxlk(&arn->freetab[bktid].lk);
        mag = arn->freetab[bktid].head;
        if (mag) {
            /* remove magazine from list of totally unallocated slabs */
            ptrval = mag->stk[mag->cur++];
            if (mag->next) {
                mag->next->prev = NULL;
            }
            arn->freetab[bktid].head = mag->next;
#if !(MALLOCWIDELOCK)
            mtxunlk(&arn->freetab[bktid].lk);
#endif
            mag->prev = NULL;
            mag->next = NULL;
            if (gtpow2(mag->max, 1)) {
                /* queue magazine to partially allocated list */
#if !(MALLOCWIDELOCK)
                mtxlk(&arn->magtab[bktid].lk);
#endif
                mag->next = arn->magtab[bktid].head;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                arn->magtab[bktid].head = mag;
#if !(MALLOCWIDELOCK)
                mtxunlk(&arn->magtab[bktid].lk);
#endif
            }
        } else {
#if !(MALLOCWIDELOCK)
            mtxunlk(&arn->freetab[bktid].lk);
#endif
            mtxlk(&g_malloc.magtab[bktid].lk);
            mag = g_malloc.magtab[bktid].head;
            if (mag) {
                ptrval = mag->stk[mag->cur++];
                if (mag->cur == mag->max) {
                    if (mag->next) {
                        mag->next->prev = NULL;
                    }
                    g_malloc.magtab[bktid].head = mag->next;
                    mtxunlk(&g_malloc.magtab[bktid].lk);
                    mag->prev = NULL;
                    mag->next = NULL;
                }
            } else {
#if !(MALLOCWIDELOCK)
                mtxunlk(&g_malloc.magtab[bktid].lk);
#endif
                mtxlk(&g_malloc.freetab[bktid].lk);
                mag = g_malloc.freetab[bktid].head;
                if (mag) {
                    ptrval = mag->stk[mag->cur++];
                    if (mag->next) {
                        mag->next->prev = NULL;
                    }
                    g_malloc.freetab[bktid].head = mag->next;
                    mtxunlk(&g_malloc.freetab[bktid].lk);
                    mag->prev = NULL;
                    mag->next = NULL;
                    if (gtpow2(mag->max, 1)) {
                        /* queue magazine to partially allocated list */
                        mtxlk(&g_malloc.magtab[bktid].lk);
                        mag->next = g_malloc.magtab[bktid].head;
                        if (mag->next) {
                            mag->next->prev = mag;
                        }
                        g_malloc.magtab[bktid].head = mag;
                        mtxunlk(&g_malloc.magtab[bktid].lk);
                    }
                } else {
                    /* create new magazine */
                    mtxunlk(&g_malloc.freetab[bktid].lk);
                    mtxlk(&arn->hdrtab[bktid].lk);
                    /* try to use a cached magazine header */
                    mag = arn->hdrtab[bktid].head;
                    if (mag) {
                        if (mag->next) {
                            mag->next->prev = NULL;
                        }
                        arn->hdrtab[bktid].head = mag->next;
                        mtxunlk(&arn->hdrtab[bktid].lk);
                        mag->prev = NULL;
                        mag->next = NULL;
                    } else {
                        mtxunlk(&arn->hdrtab[bktid].lk);
                        /* map new magazine header */
                        mag = mapanon(g_malloc.zerofd, MALLOCMAGSIZE);
                        if (mag == MAP_FAILED) {
                            
                            return NULL;
                        }
                        if (magembedstk(bktid)) {
                            /* use magazine headers data-field for allocation stack */
                            mag->stk = (void **)mag->data;
                            mag->ptrtab = &mag->stk[1UL << nblklog2(bktid)];
                        } else {
                            /* map new allocation stack */
                            stk = mapanon(g_malloc.zerofd, nbstk(bktid));
                            if (stk == MAP_FAILED) {
                                unmapanon(mag, MALLOCMAGSIZE);
                                
                                return NULL;
                            }
                            mag->stk = stk;
                            mag->ptrtab = &stk[1UL << nblklog2(bktid)];
                        }
                    }
                    ptr = SBRK_FAILED;
                    if (bktid <= MALLOCSLABLOG2) {
                        /* try to allocate slab from heap */
                        mtxlk(&g_malloc.heaplk);
                        ptr = growheap(nbmag(bktid));
                        mtxunlk(&g_malloc.heaplk);
                    }
                    if (ptr == SBRK_FAILED) {
                        /* try to map slab */
                        ptr = mapanon(g_malloc.zerofd, nbmag(bktid));
                        if (ptr == MAP_FAILED) {
                            unmapanon(mag, MALLOCMAGSIZE);
                            if (!magembedstk(bktid)) {
                                unmapanon(stk, nbstk(bktid));
                            }
                            
                            return NULL;
                        }
                        mapped = 1;
                    }
                    ptrval = ptr;
                    /* initialise magazine header */
                    max = 1UL << nblklog2(bktid);
                    if (mapped) {
                        mag->adr = (void *)((uintptr_t)ptr | MAGMAP);
                    } else {
                        mag->adr = ptr;
                    }
                    mag->cur = 1;
                    mag->max = max;
                    mag->arnid = arnid;
                    mag->bktid = bktid;
                    mag->prev = NULL;
                    mag->next = NULL;
                    stk = mag->stk;
                    /* initialise allocation stack */
                    incr = 1UL << bktid;
                    for (n = 0 ; n < max ; n++) {
                        ptr += incr;
                        stk[n] = ptr;
                    }
                    if (gtpow2(max, 1)) {
                        /* queue slab with an active allocation */
                        mag->next = arn->magtab[bktid].head;
                        if (mag->next) {
                            mag->next->prev = mag;
                        }
                        arn->magtab[bktid].head = mag;
                    }
                    mag->stk = stk;
                    mag->ptrtab = &stk[1UL << nblklog2(bktid)];
                }
            }
        }
#if (MALLOCWIDELOCK)
        mtxunlk(&g_malloc.magtab[bktid].lk);
        mtxunlk(&arn->freetab[bktid].lk);
#endif
    }
#if (MALLOCWIDELOCK)
    mtxunlk(&arn->magtab[bktid].lk);
#endif
//    assert(mag != NULL);
    ptr = clrptr(ptrval);
    if (ptr) {
        /* TODO: unlock magtab earlier */
        if ((zero) && (((uintptr_t)ptrval & BLKDIRTY))) {
            memset(ptr, 0, 1UL << (bktid));
        }
        if (align) {
            ptr = ptralign(ptr, align);
        }
        /* store unaligned source pointer */
        magputptr(mag, ptr, clrptr(ptrval));
        /* add magazine to lookup structure using retptr as key */
        setmag(ptr, mag);
#if defined(ENOMEM)
    } else {
        errno = ENOMEM;
#endif
    }
//    assert(ptr != NULL);

    return ptr;
}

void
_free(void *ptr)
{
    struct arn *arn;
    struct mag *mag;
    long        arnid;
    long        max;
    long        bktid;
    long        freemap = 0;

    mag = findmag(ptr);
    if (mag) {
        setmag(ptr, NULL);
        arnid = mag->arnid;
        bktid = mag->bktid;
        arn = g_malloc.arntab[arnid];
        mtxlk(&arn->magtab[bktid].lk);
        /* remove pointer from allocation lookup structure */
        setmag(ptr, NULL);
        ptr = maggetptr(mag, ptr);
        arn = g_malloc.arntab[arnid];
        max = mag->max;
        bktid = mag->bktid;
        mag->stk[--mag->cur] = (uint8_t *)((uintptr_t)ptr | BLKDIRTY);
        if (!mag->cur) {
        /* TODO: unlock magtab here? */
            if (gtpow2(max, 1)) {
                /* remove magazine from partially allocated list */
                if (mag->prev) {
                    mag->prev->next = mag->next;
                } else {
                    arn->magtab[bktid].head = mag->next;
                }
                if (mag->next) {
                    mag->next->prev = mag->prev;
                }
                mag->prev = NULL;
                mag->next = NULL;
            }
            if ((uintptr_t)mag->adr & MAGMAP) {
                /* indicate magazine was mapped */
                freemap = 1;
            } else {
                /* queue map to list of totally unallocated ones */
                mtxlk(&arn->freetab[bktid].lk);
                mag->next = arn->freetab[bktid].head;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                arn->freetab[bktid].head = mag;
                mtxunlk(&arn->freetab[bktid].lk);
            }
        } else {
            /* allocate from list of partially allocated magazines */
            if (mag->cur == max - 1) {
                mag->prev = NULL;
                /* remove [fully allocated] magazine from partial list */
                mag->next = arn->magtab[bktid].head;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                arn->magtab[bktid].head = mag;
            }
        }
        mtxunlk(&arn->magtab[bktid].lk);
        if (freemap) {
            /* unmap slab */
            unmapanon(mag->adr, nbmag(bktid));
            mag->adr = NULL;
            mag->prev = NULL;
            /* add magazine header to header cache */
            mtxlk(&arn->hdrtab[bktid].lk);
            mag->next = arn->hdrtab[bktid].head;
            if (mag->next) {
                mag->next->prev = mag;
            }
            arn->hdrtab[bktid].head = mag;
            mtxunlk(&arn->hdrtab[bktid].lk);
        }
    }

    return;
}

void *
malloc(size_t size)
{
    void   *ptr;

    if (!size) {

        return NULL;
    }
#if defined(_GNU_SOURCE) && (GNUMALLOCHOOKS)
    if (__malloc_hook) {
        void *caller;

        caller = m_getretadr();
        __malloc_hook(size, (const void *)caller);;
    }
#endif
    ptr = _malloc(size, 0, 0);

    return ptr;
}

void *
calloc(size_t n, size_t size)
{
    size_t sz = max(n * size, MALLOCMINSIZE);
    void *ptr = _malloc(sz, 0, 1);

    if (!sz) {
        return NULL;
    }

    return ptr;
}

void *
_realloc(void *ptr,
         size_t size,
         long rel)
{
    void       *retptr = ptr;
    long        sz = max(size, MALLOCMINSIZE);
    struct mag *mag = (ptr) ? findmag(ptr) : NULL;
    long        bktid = blkbktid(sz);
    uintptr_t   bsz = (mag) ? 1UL << bktid : 0;

    if (!ptr) {
        retptr = _malloc(size, 0, 0);
    } else if ((mag) && mag->bktid != bktid) {
        retptr = _malloc(size, 0, 0);
        if (retptr) {
            memcpy(retptr, ptr, min(sz, bsz));
            _free(ptr);
            ptr = NULL;
        }
    }
    if ((rel) && (ptr)) {
        _free(ptr);
    }

    return retptr;
}

void *
realloc(void *ptr,
        size_t size)
{
    void *retptr = NULL;

#if defined(_GNU_SOURCE) && (GNUMALLOCHOOKS)
    if (__realloc_hook) {
        void *caller;

        caller = m_getretadr();
        __realloc_hook(ptr, size, (const void *)caller);
    }
#endif
    if (!size && (ptr)) {
        _free(ptr);
    } else {
        retptr = _realloc(ptr, size, 0);
    }

    return retptr;
}

void
free(void *ptr)
{
#if (_BNU_SOURCE)
    if (__free_hook) {
        void *caller;

        caller = m_getretadr();
        __free_hook(ptr, (const void *)caller);
    }
#endif
    if (ptr) {
        _free(ptr);
    }

    return;
}

#if (_ISOC11_SOURCE)
void *
aligned_alloc(size_t align,
              size_t size)
{
    void *ptr = NULL;

#if defined(_GNU_SOURCE) && (GNUMALLOCHOOKS)
    if (__memalign_hook) {
        void *caller;
        
        caller = m_getretadr();
        __memalign_hook(align, size, (const void *)caller);
    }
#endif
    if (!powerof2(align) || (size & (align - 1))) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, align, 0);
    }

    return ptr;
}

#endif

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
int
posix_memalign(void **ret,
               size_t align,
               size_t size)
{
    void *ptr = _malloc(size, align, 0);
    int   retval = -1;

#if defined(_GNU_SOURCE) && (GNUMALLOCHOOKS)
    if (__memalign_hook) {
        void *caller;
        
        caller = m_getretadr();
        __memalign_hook(align, size, (const void *)caller);
    }
#endif
    if (!powerof2(align) || (size & (sizeof(void *) - 1))) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, align, 0);
        if (ptr) {
            retval ^= retval;
        }
    }
    *ret = ptr;

    return retval;
}
#endif

/* STD: UNIX */

#if ((_BSD_SOURCE)                                                      \
    || (_XOPEN_SOURCE >= 500 || ((_XOPEN_SOURCE) && (_XOPEN_SOURCE_EXTENDED))) \
    && !(_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600))
void *
valloc(size_t size)
{
    void *ptr;

#if defined(_GNU_SOURCE) && (GNUMALLOCHOOKS)
    if (__memalign_hook) {
        void *caller;
        
        caller = m_getretadr();
        __memalign_hook(PAGESIZE, size, (const void *)caller);
    }
#endif
    ptr = _malloc(size, PAGESIZE, 0);
    
    return ptr;
}
#endif

void *
memalign(size_t align,
         size_t size)
{
    void *ptr = NULL;

#if defined(_GNU_SOURCE) && (GNUMALLOCHOOKS)
    if (__memalign_hook) {
        void *caller;
        
        caller = m_getretadr();
        __memalign_hook(align, size, (const void *)caller);
    }
#endif
    if (!powerof2(align)) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, align, 0);
    }

    return ptr;
}

#if (_BSD_SOURCE)
void *
reallocf(void *ptr,
         size_t size)
{
    void *retptr = _realloc(ptr, size, 1);

    return retptr;
}
#endif

#if defined(_GNU_SOURCE)
void *
pvalloc(size_t size)
{
    size_t  sz = rounduppow2(size, PAGESIZE);
    void   *ptr = _malloc(sz, PAGESIZE, 0);

    return ptr;
}
#endif

void
cfree(void *ptr)
{
    if (ptr) {
        _free(ptr);
    }

    return;
}

size_t
malloc_usable_size(void *ptr)
{
    struct mag *mag = findmag(ptr);
    size_t      sz = (mag) ? 1UL << mag->bktid : 0;

    return sz;
}

size_t
malloc_good_size(size_t size)
{
    size_t sz = 1UL << blkbktid(size);

    return sz;
}

size_t
malloc_size(void *ptr)
{
    struct mag *mag = findmag(ptr);
    size_t      sz = (mag) ? 1UL << mag->bktid : 0;

    return sz;
}

