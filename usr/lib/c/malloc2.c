/*r
 * Zero Malloc Revision 2
 *
 * Copyright Tuomo Petteri Venäläinen 2014
 */

#define MALLOCVARSIZEBUF 0

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
 *
 *        map
 *        ---
 *        - process map segment
 *          - mmap() interface; thread-safe
 *
 *        headers
 *        -------
 *        - mapped internal book-keeping for magazines
 *          - pointer stacks
 *          - table to map allocation pointers to magazine pointers
 *            - may differ because of alignments etc.
 *          - optionally, a bitmap to denote unallocated slices in magazines
 */

#define GNUMALLOCHOOKS 1

#include <assert.h>

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
#define MALLOCNARN           4
#if (MALLOCVARSIZEBUF)
#define MALLOCSLABLOG2       18
#define MALLOCSMALLSLABLOG2  16
#define MALLOCTINYSLABLOG2   13
#define MALLOCTEENYSLABLOG2  10
#define MALLOCSMALLMAPLOG2   22
#define MALLOCMIDSIZEMAPLOG2 24
#define MALLOCBIGMAPLOG2     25
#else
#define MALLOCSLABLOG2       20
#endif
#define MALLOCMINSIZE        (1UL << MALLOCMINLOG2)
#define MALLOCMINLOG2        CLSIZELOG2
#define MALLOCNBKT           PTRBITS

/*
 * magazines for bucket bktid have 1 << magnblklog2(bktid) blocks of
 * 1 << bktid bytes
 */
#if (MALLOCVARSIZEBUF)
#define magnbytelog2(bktid)                                             \
    ((((bktid) <= MALLOCSLABLOG2)                                       \
      ? (((bktid <= MALLOCTEENYSLABLOG2)                                \
          ? MALLOCTINYSLABLOG2                                          \
          : (((bktid <= MALLOCTINYSLABLOG2)                             \
              ? MALLOCSMALLSLABLOG2                                     \
              : MALLOCSLABLOG2))))                                      \
      : (((bktid <= MALLOCSMALLMAPLOG2)                                 \
          ? MALLOCMIDSIZEMAPLOG2                                        \
          : (((bktid) <= MALLOCMIDSIZEMAPLOG2)                          \
             ? MALLOCBIGMAPLOG2                                         \
             : (bktid))))))
#define magnblklog2(bktid)                                              \
    (magnbytelog2(bktid) - (bktid))
#else
#define magnblklog2(bktid)                                              \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? (MALLOCSLABLOG2 - (bktid))                                       \
     : 0)
#endif

#define MAGMAP         0x01
#define MAGFLGMASK     MAGMAP
#define MALLOCMAGSIZE  PAGESIZE
#define MAGGLOBAL      0x0001
/* magazines for larger/fewer allocations embed the tables in the structure */
#define magembedstk(bktid) (nbstk(bktid) <= MALLOCMAGSIZE - offsetof(struct mag, data))
/* magazine header structure */
struct mag {
    void        *adr;
    long         cur;
    long         max;
    long         arnid;
    long         bktid;
    struct mag  *prev;
    struct mag  *next;
#if (MAGFREEMAP)
    uint8_t     *freemap;
#endif
    void       **stk;
    void       **ptrtab;
    uint8_t      data[EMPTY];
};

/* magazine list header structure */
struct maglist {
    MUTEX       lk;
    long        n;
    struct mag *head;
    struct mag *tail;
};

#define MALLOCARNSIZE      rounduppow2(sizeof(struct arn), PAGESIZE)
/* arena structure */
struct arn {
    struct maglist magtab[MALLOCNBKT];  // partially allocated magazines
    struct maglist freetab[MALLOCNBKT]; // totally unallocated magazines
    struct maglist hdrtab[MALLOCNBKT];  // header cache
    long           nref;                // number of threads using the arena
    MUTEX          nreflk;              // lock for updating nref
};

/* malloc global structure */
#define MALLOCINIT 0x00000001L
struct malloc {
    struct maglist  magtab[MALLOCNBKT]; // partially allocated magazines
    struct maglist  freetab[MALLOCNBKT]; // totally unallocated magazines
    struct arn    **arntab;             // arena structures
    void          **mdir;               // allocation header lookup structure
    MUTEX           initlk;             // initialization lock
    MUTEX           heaplk;             // lock for sbrk()
    /* FIXME: should this key be per-thread? */
    pthread_key_t   arnkey;             // for reclaiming arenas to global pool
    long            narn;               // number of arenas in action
    long            flags;              // allocator flags
    int             zerofd;             // file descriptor for mmap()
};

static struct malloc g_malloc ALIGNED(PAGESIZE);
__thread long        _arnid = -1;
MUTEX                _arnlk;
long                 curarn;

/* allocation pointer tag bits */
#define BLKDIRTY    0x01
#define BLKFLGMASK  (MALLOCMINSIZE - 1)
/* clear tag bits at allocation time */
#define clrptr(ptr) ((void *)((uintptr_t)ptr & ~BLKFLGMASK))

#define nbstk(bktid) ((1UL << (magnblklog2(bktid) + 1)) * sizeof(void *))
#if (MALLOCVARSIZEBUF)
#define magnbyte(bktid) (1UL << magnbytelog2(bktid))
#else
#define magnbyte(bktid) (1UL << ((bktid) + magnblklog2(bktid)))
#endif

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

#define mdirl1ndx(ptr) (((uintptr_t)ptr >> MDIRL1NDX) & ((1 << MDIRNL1BIT) - 1))
#define mdirl2ndx(ptr) (((uintptr_t)ptr >> MDIRL2NDX) & ((1 << MDIRNL2BIT) - 1))
#define mdirl3ndx(ptr) (((uintptr_t)ptr >> MDIRL3NDX) & ((1 << MDIRNL3BIT) - 1))
#define mdirl4ndx(ptr) (((uintptr_t)ptr >> MDIRL4NDX) & ((1 << MDIRNL4BIT) - 1))

#define MDIRNL1BIT     12
#if (PTRBITS == 32)
#define MDIRNL2BIT     (32 - MDIRNL1BIT - MALLOCMINLOG2)
#else
#define MDIRNL2BIT     12
#endif
#if (ADRBITS >= 48)
#define MDIRNL3BIT     12
#define MDIRNL4BIT     (ADRBITS - MDIRNL1BIT - MDIRNL2BIT - MDIRNL3BIT - MALLOCMINLOG2)
#else
#define MDIRNL3BIT     (ADRBITS - MDIRNL1BIT - MDIRNL2BIT - MALLOCMINLOG2)
#define MDIRNL4BIT     0
#endif
#define MDIRNL1KEY     (1UL << MDIRNL1BIT)
#define MDIRNL2KEY     (1UL << MDIRNL2BIT)
#define MDIRNL3KEY     (1UL << MDIRNL3BIT)
#define MDIRL1NDX      (MDIRL2NDX + MDIRNL2BIT)
#define MDIRL2NDX      (MDIRL3NDX + MDIRNL3BIT)
#if (MDIRNL4BIT)
#define MDIRNL4KEY     (1UL << MDIRNL3BIT)
#define MDIRL3NDX      (MDIRL4NDX + MDIRNL4BIT)
#define MDIRL4NDX      MALLOCMINLOG2
#else
#define MDIRL3NDX      MALLOCMINLOG2
#endif

#if 0
#define MDIRNL1BIT     16
#if (PTRBITS == 32)
#define MDIRNL2BIT     (32 - MDIRNL1BIT - MALLOCMINLOG2)
#else
#define MDIRNL2BIT     16
#endif
#define MDIRNL3BIT    (ADRBITS - MDIRNL1BIT - MDIRNL2BIT - MALLOCMINLOG2)
#endif

long
thrarnid(void)
{
    struct arn *arn;
    
    if (_arnid >= 0) {

        return _arnid;
    }
    mtxlk(&_arnlk);
    _arnid = curarn++;
    arn = g_malloc.arntab[_arnid];
    mtxlk(&arn->nreflk);
    arn->nref++;
    mtxunlk(&arn->nreflk);
    curarn &= (MALLOCNARN - 1);
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
    uintptr_t   l1 = mdirl1ndx(ptr);
    uintptr_t   l2 = mdirl2ndx(ptr);
    uintptr_t   l3 = mdirl3ndx(ptr);
#if (MDIRNL4BIT)
    uintptr_t   l4 = mdirl4ndx(ptr);
#endif
    void       *ptr1;
    void       *ptr2;
    struct mag *mag = NULL;

    ptr1 = g_malloc.mdir[l1];
    if (ptr1) {
        ptr2 = ((void **)ptr1)[l2];
#if (MDIRNL4BIT)
        if (ptr2) {
            ptr1 = ((struct mag **)ptr2)[l3];
            if (ptr1) {
                mag = ((struct mag **)ptr1)[l4];
            }
        }
#else
        if (ptr2) {
            mag = ((struct mag **)ptr2)[l3];
        }
#endif
    }

    return mag;
}

static void
setmag(void *ptr,
       struct mag *mag)
{
    uintptr_t    l1 = mdirl1ndx(ptr);
    uintptr_t    l2 = mdirl2ndx(ptr);
    uintptr_t    l3 = mdirl3ndx(ptr);
#if (MDIRNL4BIT)
    uintptr_t    l4 = mdirl4ndx(ptr);
#endif
    void        *ptr1;
    void        *ptr2;
    void       **pptr;
    struct mag **item;

    ptr1 = g_malloc.mdir[l1];
    if (!ptr1) {
        g_malloc.mdir[l1] = ptr1 = mapanon(g_malloc.zerofd,
                                           MDIRNL2KEY * sizeof(void *));
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
                                  MDIRNL3KEY * sizeof(struct mag *));
        if (ptr2 == MAP_FAILED) {
#ifdef ENOMEM
            errno = ENOMEM;
#endif

            exit(1);
        }
    }
#if (MDIRNL4BIT)
    pptr = ptr2;
    ptr1 = pptr[l3];
    if (!ptr1) {
        pptr[l3] = ptr1 = mapanon(g_malloc.zerofd,
                                  MDIRNL4KEY * sizeof(struct mag *));
        if (ptr2 == MAP_FAILED) {
#ifdef ENOMEM
            errno = ENOMEM;
#endif

            exit(1);
        }
    }
    item = &((struct mag **)ptr1)[l4];
#else
    item = &((struct mag **)ptr2)[l3];
#endif
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
    g_malloc.mdir = mapanon(g_malloc.zerofd, MDIRNL1KEY * sizeof(void *));
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
        mtxunlk(&arn->magtab[bktid].lk);
        mtxlk(&arn->freetab[bktid].lk);
        mag = arn->freetab[bktid].head;
        if (mag) {
            /* remove magazine from list of totally unallocated slabs */
            ptrval = mag->stk[mag->cur++];
            if (mag->next) {
                mag->next->prev = NULL;
            }
            arn->freetab[bktid].head = mag->next;
            mtxunlk(&arn->freetab[bktid].lk);
            mag->prev = NULL;
            mag->next = NULL;
            if (gtpow2(mag->max, 1)) {
                /* queue magazine to partially allocated list */
                mtxlk(&arn->magtab[bktid].lk);
                mag->next = arn->magtab[bktid].head;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                arn->magtab[bktid].head = mag;
                mtxunlk(&arn->magtab[bktid].lk);
            }
        } else {
            mtxunlk(&arn->freetab[bktid].lk);
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
                mtxunlk(&g_malloc.magtab[bktid].lk);
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
                            mag->ptrtab = &mag->stk[1UL << magnblklog2(bktid)];
                        } else {
                            /* map new allocation stack */
                            stk = mapanon(g_malloc.zerofd, nbstk(bktid));
                            if (stk == MAP_FAILED) {
                                unmapanon(mag, MALLOCMAGSIZE);
                                
                                return NULL;
                            }
                            mag->stk = stk;
                            mag->ptrtab = &stk[1UL << magnblklog2(bktid)];
                        }
                    }
                    ptr = SBRK_FAILED;
                    if (bktid <= MALLOCSLABLOG2) {
                        /* try to allocate slab from heap */
                        mtxlk(&g_malloc.heaplk);
                        ptr = growheap(magnbyte(bktid));
                        mtxunlk(&g_malloc.heaplk);
                    }
                    if (ptr == SBRK_FAILED) {
                        /* try to map slab */
                        ptr = mapanon(g_malloc.zerofd, magnbyte(bktid));
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
                    max = 1UL << magnblklog2(bktid);
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
                    mag->ptrtab = &stk[1UL << magnblklog2(bktid)];
                }
            }
        }
    }
    ptr = clrptr(ptrval);
    if (ptr) {
        /* TODO: unlock magtab earlier */
        if (zero && (((uintptr_t)ptrval & BLKDIRTY))) {
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
            unmapanon(mag->adr, magnbyte(bktid));
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

