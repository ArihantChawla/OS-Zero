/*
 * Zero Malloc Revision 2
 *
 * Copyright Tuomo Petteri Venäläinen 2014
 */

#define MALLOCDEBUG      1
#define MALLOCFREEMDIR   0  // under construction
#define MALLOCSTKNDX     0
#define MALLOCFREEMAP    0  // use free block bitmaps
#define MALLOCHACKS      0  // enable experimental features
#define MALLOCBUFMAP     0  // buffer mapped slabs to global pool
#define MALLOCVARSIZEBUF 0  // use variable-size slabs; FIXME
#define MALLOCHASH       0
#define MALLOCNHASHBIT   20
#define MALLOCNHASH      (1UL << MALLOCNHASHBIT)

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

 /*
  * TODO
  * ----
  * - free inactive subtables from mdir
  * - fix MALLOCVARSIZEBUF
  */

#define GNUMALLOCHOOKS 1

#if (MALLOCDEBUG)
#include <assert.h>
#endif
#include <features.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <malloc.h>
#if (MALLOCFREEMAP)
#include <limits.h>
#endif

//#include <sys/sysinfo.h>

#define ZEROMTX 1
#if defined(PTHREAD) && (PTHREAD)
#include <pthread.h>
#define MUTEX pthread_mutex_t
#define mtxinit(mp) pthread_mutex_init(mp, NULL)
#define mtxlk(mp) pthread_mutex_lock(mp)
#define mtxunlk(mp) pthread_mutex_unlock(mp)
#endif
#if defined(ZEROMTX) && (ZEROMTX)
//#define MUTEX volatile long
//#include <zero/mtx.h>
#elif (PTHREAD)
#define MUTEX pthread_mutex_t
#endif
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/trix.h>
#if (MALLOCHASH)
#include <zero/hash.h>
#endif

//#define MALLOCNARN     (2 * get_nprocs_conf())
//#define MALLOCNARN     (2 * sysconf(_SC_NPROCESSORS_CONF))
#define MALLOCNARN           16
#if (MALLOCVARSIZEBUF)
#define MALLOCSLABLOG2       18
#define MALLOCSMALLSLABLOG2  16
#define MALLOCTINYSLABLOG2   13
#define MALLOCTEENYSLABLOG2  10
#else
#define MALLOCSLABLOG2       18
#endif
#if (MALLOCVARSIZEBUF) || (MALLOCBUFMAP)
#define MALLOCSMALLMAPLOG2   21
#define MALLOCMIDSIZEMAPLOG2 23
#define MALLOCBIGMAPLOG2     25
#endif
#define MALLOCMINSIZE        (1UL << MALLOCMINLOG2)
#define MALLOCMINLOG2        CLSIZELOG2
#define MALLOCNBKT           PTRBITS
#if (MALLOCSTKNDX)
#define MAGPTRNDX            uint32_t
#endif

/*
 * magazines for bucket bktid have 1 << magnblklog2(bktid) blocks of
 * 1 << bktid bytes
 */
#if (MALLOCBUFMAP)
#define magnbufmaplog2(bktid)                                           \
    (((bktid) <= MALLOCSMALLMAPLOG2)                                    \
     ? 3                                                                \
     : (((bktid) <= MALLOCMIDSIZEMAPLOG2)                               \
        ? 2                                                             \
        : (((bktid) <= MALLOCBIGMAPLOG2                                 \
            ? 1                                                         \
            : 0))))
#define magnbufmap(bktid)                                               \
    (1UL << magnbufmaplog2(bktid))
#endif /* MALLOCBUFMAP */
#if (MALLOCSTKNDX)
#define magnbytelog2(bktid)                                             \
    (1UL << (bktid))
#define magnblklog2(bktid)                                              \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? (MALLOCSLABLOG2 - (bktid))                                       \
     : 0)
#elif (MALLOCVARSIZEBUF)
#define magnbytelog2(bktid)                                             \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? (((bktid) <= MALLOCTEENYSLABLOG2)                                \
        ? MALLOCTINYSLABLOG2                                            \
        : (((bktid) <= MALLOCTINYSLABLOG2)                              \
           ? MALLOCSMALLSLABLOG2                                        \
           : MALLOCSLABLOG2))                                           \
     : (((bktid) <= MALLOCSMALLMAPLOG2)                                 \
        ? MALLOCMIDSIZEMAPLOG2                                          \
        : (((bktid) <= MALLOCMIDSIZEMAPLOG2)                            \
           ? MALLOCBIGMAPLOG2                                           \
           : (bktid))))
#define magnblklog2(bktid)                                              \
    (magnbytelog2(bktid) - (bktid))
#else
#define magnbytelog2(bktid) (bktid)
#define magnblklog2(bktid)                                              \
    (((bktid) <= MALLOCSLABLOG2)                                        \
     ? (MALLOCSLABLOG2 - (bktid))                                       \
     : 0)
#endif

#define MAGMAP         0x01
#define MAGGLOB        0x02
#define MAGFLGMASK     (MAGMAP | MAGGLOB)
#define MALLOCMAGSIZE  (8 * PAGESIZE)
/* magazines for larger/fewer allocations embed the tables in the structure */
#define magembedstk(bktid) \
    (magnbytetab(bktid) <= MALLOCMAGSIZE - offsetof(struct mag, data))
/* magazine header structure */
struct mag {
    void        *adr;
    long         cur;
    long         max;
    long         arnid;
    long         bktid;
    struct mag  *prev;
    struct mag  *next;
#if (MALLOCFREEMAP)
    uint8_t     *freemap;
#endif
#if (MALLOCSTKNDX)
    MAGPTRNDX   *stk;
    MAGPTRNDX   *ptrtab;
#elif (MALLOCHACKS)
    uintptr_t   *stk;
    uintptr_t   *ptrtab;
#else
    void       **stk;
    void       **ptrtab;
#endif
    uint8_t      data[EMPTY];
};

#if (MALLOCDEBUG)
void
magprint(struct mag *mag)
{
    fprintf(stderr, "MAG %p\n", mag);
    fprintf(stderr, "\tadr\t%p\n", mag->adr);
    fprintf(stderr, "\tcur\t%ld\n", mag->cur);
    fprintf(stderr, "\tmax\t%ld\n", mag->max);

    return;
}
#endif

/* magazine list header structure */
struct maglist {
    MUTEX       lk;
    long        n;
    struct mag *head;
    struct mag *tail;
};

#if (MALLOCHASH)
struct hashitem {
    void            *ptr;
    struct mag      *mag;
    struct hashitem *next;
};

struct hashlist {
    MUTEX            lk;
    long             n;
    struct hashitem *head;
    struct hashitem *tail;
};
#endif /* MALLOCHASH */

#if (MALLOCFREEMDIR)
struct magitem {
    struct mag *mag;
    long        nref;
};
#endif

#define MALLOCARNSIZE      rounduppow2(sizeof(struct arn), PAGESIZE)
/* arena structure */
struct arn {
    struct maglist magtab[MALLOCNBKT];  // partially allocated magazines
    struct maglist hdrtab[MALLOCNBKT];  // header cache
#if 0
    long           nref;                // number of threads using the arena
    MUTEX          nreflk;              // lock for updating nref
#endif
};

/* malloc global structure */
#define MALLOCINIT 0x00000001L
struct malloc {
    struct maglist    magtab[MALLOCNBKT]; // partially allocated magazines
    struct maglist    freetab[MALLOCNBKT]; // totally unallocated magazines
#if (MALLOCHASH)
    struct hashlist  *hashtab;
    struct hashitem  *hashbuf;
#endif
    struct arn      **arntab;           // arena structures
    void            **mdir;             // allocation header lookup structure
    MUTEX             initlk;           // initialization lock
    MUTEX             heaplk;           // lock for sbrk()
    /* FIXME: should this key be per-thread? */
    pthread_key_t     arnkey;           // for reclaiming arenas to global pool
    long              narn;             // number of arenas in action
    long              flags;            // allocator flags
    int               zerofd;           // file descriptor for mmap()
};

static struct malloc g_malloc ALIGNED(PAGESIZE);
__thread long        _arnid = -1;
MUTEX                _arnlk;
long                 curarn;

#if defined(_GNU_SOURCE) && (GNUMALLOCHOOKS)
void *(*__malloc_hook)(size_t size, const void *caller);
void *(*__realloc_hook)(void *ptr, size_t size, const void *caller);
void *(*__memalign_hook)(size_t align, size_t size, const void *caller);
void  (*__free_hook)(void *ptr, const void *caller);
void *(*__malloc_initialize_hook)(void);
void  (*__after_morecore_hook)(void);
#endif

/* allocation pointer flag bits */
#define BLKDIRTY    0x01
#define BLKFLGMASK  (MALLOCMINSIZE - 1)
/* clear flag bits at allocation time */
#define clrptr(ptr) ((void *)((uintptr_t)ptr & ~BLKFLGMASK))

#if (MALLOCFREEMAP)
#define magptrndx(mag, ptr)                                             \
    (((uintptr_t)ptr - (uintptr_t)mag->adr) >> mag->bktid)
#endif
#if (MALLOCSTKNDX)
#if (MALLOCFREEMAP)
#define magnbytetab(bktid)                                              \
    (((1UL << (magnblklog2(bktid) + 1)) * sizeof(MAGPTRNDX))            \
     + rounduppow2((1UL << magnblklog2(bktid)) / CHAR_BIT, PAGESIZE))
#else /* !MALLOCFREEMAP */
#define magnbytetab(bktid)                                              \
    ((1UL << (magnblklog2(bktid) + 1)) * sizeof(MAGPTRNDX))
#endif /* MALLOCFREEMAP */
#elif (MALLOCHACKS)
#if (MALLOCFREEMAP)
#define magnbytetab(bktid)                                             \
    ((1UL << (magnblklog2((bktid) + 1))) * sizeof(MAGPTRNDX)           \
     + rounduppow2((1UL << magnblklog2(bktid)) / CHAR_BIT, PAGESIZE))
#else
#define magnbytetab(bktid) ((1UL << (magnblklog2(bktid) + 1)) * sizeof(void *))
#endif
#else
#define magnbytetab(bktid) ((1UL << (magnblklog2(bktid) + 1)) * sizeof(void *))
#endif
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

#if (MALLOCSTKNDX)
#define magputptr(mag, ptr1, ptr2)                                      \
    ((mag)->ptrtab[magptr2ndx(mag, ptr1)] = magptr2ndx(mag, ptr2))
#define magptr2ndx(mag, ptr)                                            \
    ((MAGPTRNDX)(((uintptr_t)ptr \
        - ((uintptr_t)(mag)->adr & ~(MAGFLGMASK))) \
     >> (bktid)))
#define magndx2ptr(mag, ndx)                                            \
    ((void *)((uintptr_t)(mag)->adr &~MAGFLGMASK) + ((ndx) << (mag)->bktid))
#define maggetptr(mag, ptr)                                             \
    (magndx2ptr(mag, magptr2ndx(mag, ptr)))
#else /* !MALLOCSTKNDX */
#define magptrid(mag, ptr)                                              \
    (((uintptr_t)(ptr) - ((uintptr_t)(mag)->adr & ~MAGFLGMASK)) >> (mag)->bktid)
#define magputptr(mag, ptr1, ptr2)                                      \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr1)] = (ptr2))
#define maggetptr(mag, ptr)                                             \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr)])
#endif /* MALLOCSTKNDX */

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
#if (ADRBITS > 48)
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
//    mtxlk(&arn->nreflk);
//    arn->nref++;
    curarn &= (MALLOCNARN - 1);
    pthread_setspecific(g_malloc.arnkey, g_malloc.arntab[_arnid]);
//    mtxunlk(&arn->nreflk);
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

#if (MALLOCHASH)

static struct hashitem *
hashgetitem(void)
{
    struct hashitem *item = g_malloc.hashbuf;

    if (item) {
        g_malloc.hashbuf = item->next;
    } else {
        struct hashitem *tmp;
        unsigned long    n = PAGESIZE / sizeof(struct hashitem);
        
        item = mapanon(g_malloc.zerofd, PAGESIZE / sizeof(struct hashitem));
        if (item) {
            tmp = item;
            while (--n) {
                tmp->next = ++item;
                tmp = item;
            }
            tmp->next = NULL;
        }
    }

    return item;
}

static struct mag *
findmag(void *ptr)
{
    unsigned long    key = hashq128(&ptr, sizeof(void *), MALLOCNHASHBIT);
    struct hashitem *item;

    item = g_malloc.hashtab[key].head;
    while (item) {
        if (item->ptr == ptr) {
            
            return item->mag;
        }
        item = item->next;
    }

    return NULL;
}

static void
setmag(void *ptr,
       struct mag *mag)
{
    unsigned long    key = hashq128(&ptr, sizeof(void *), MALLOCNHASHBIT);
    struct hashitem *prev = NULL;
    struct hashitem *item;

    if (!mag) {
        item = g_malloc.hashtab[key].head;
        while (item) {
            if (item->ptr == ptr) {
                if (prev) {
                    prev->next = item->next;
                }
                if (!item->next && !prev) {
                    g_malloc.hashtab[key].head = NULL;
                    g_malloc.hashtab[key].tail = NULL;
                }

                return;
            }
            prev = item;
            item = item->next;
        }
    } else {
        item = hashgetitem();
        item->ptr = ptr;
        item->mag = mag;
        item->next = g_malloc.hashtab[key].head;
        if (!item->next) {
            g_malloc.hashtab[key].tail = item;
        }
        g_malloc.hashtab[key].head = item;
    }
}

#else /* !MALLOCHASH */

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
#if (MALLOCFREEMDIR)
            if (ptr1) {
                mag = ((struct magitem **)ptr1)[l4]->mag;
            }
        }
#endif
            if (ptr1) {
                mag = ((struct mag **)ptr1)[l4];
            }
        }
#else
        if (ptr2) {
#if (MALLOCFREEMDIR)
            mag = ((struct magitem **)ptr2)[l3]->mag;
#endif
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
    uintptr_t        l1 = mdirl1ndx(ptr);
    uintptr_t        l2 = mdirl2ndx(ptr);
    uintptr_t        l3 = mdirl3ndx(ptr);
#if (MDIRNL4BIT)
    uintptr_t        l4 = mdirl4ndx(ptr);
#endif
#if (MALLOCFREEMDIR)
    struct magitem  *ptr1;
    struct magitem  *ptr2;
#else
    void            *ptr1;
    void            *ptr2;
#endif
    void           **pptr;
#if (MALLOCFREEMDIR)
    struct magitem **item;
#else
    struct mag     **item;
#endif
#if (MALLOCFREEMDIR)
    void            *tab[3] = { NULL, NULL, NULL };
    long             nref;
#endif
    
    ptr1 = g_malloc.mdir[l1];
    if (!ptr1) {
#if (MALLOCFREEMDIR)
        if (!ptr) {
            
            return;
        }
        g_malloc.mdir[l1] = ptr1 = mapanon(g_malloc.zerofd,
                                           MDIRNL2KEY * sizeof(struct magitem));
#else
        g_malloc.mdir[l1] = ptr1 = mapanon(g_malloc.zerofd,
                                           MDIRNL2KEY * sizeof(void *));
#endif
        if (ptr1 == MAP_FAILED) {
#ifdef ENOMEM
            errno = ENOMEM;
#endif
            
            exit(1);
#if (MALLOCFREEMDIR)
        } else {
            ptr1->nref++;
#endif
        }
#if (MALLOCFREEMDIR)
    } else if (!mag) {
        nref = --ptr1->nref;
        if (!nref) {
            tab[0] = ptr1;
        }
    } else {
        ptr1->nref++;
#endif /* MALLOCFREEMDIR */
    }
    pptr = (void **)ptr1;
    ptr2 = pptr[l2];
    if (!ptr2) {
#if (MALLOCFREEMDIR)
        if (!mag) {
            nref = --ptr1->nref;
            if (!nref) {
                tab[1] = ptr1;
            }
            ptr1 = tab[0];
            if (ptr1) {
                unmapanon(ptr1,
                          MDIRNL2KEY * sizeof(struct magitem));
                
                return;
            }
        } else {
            pptr[l2] = ptr2 = mapanon(g_malloc.zerofd,
                                      MDIRNL3KEY * sizeof(struct magitem));
        }
#else /* !MALLOCFREEMDIR */
        pptr[l2] = ptr2 = mapanon(g_malloc.zerofd,
                                  MDIRNL3KEY * sizeof(struct mag *));
#endif /* MALLOCFREEMDIR */
        if (ptr2 == MAP_FAILED) {
#ifdef ENOMEM
            errno = ENOMEM;
#endif
            
            exit(1);
        }
#if (MALLOCFREEMDIR)
    } else if (!mag) {
        nref = --ptr1->nref;
        if (!nref) {
            tab[1] = ptr;
        }
        if (ptr2) {
            ptr2->nref++;
        }
#endif
    }
#if (MDIRNL4BIT)
    pptr = ptr2;
    ptr1 = pptr[l3];
    if (!ptr1) {
#if (MALLOCFREEMDIR)
        if (!mag) {
            ptr1 = tab[0];
            if (ptr1) {
                unmapanon(ptr,
                          MDIRNL2KEY * sizeof(struct magitem));
            }
            ptr1 = tab[1];
            if (ptr1) {
                unmapanon(ptr,
                          MDIRNL3KEY * sizeof(struct magitem));
            }
            
            return;
        } else {
            pptr[l3] = ptr1 = mapanon(g_malloc.zerofd,
                                      MDIRNL4KEY * sizeof(struct magitem));
        }
#else /* !MALLOCFREEMDIR */
        pptr[l3] = ptr1 = mapanon(g_malloc.zerofd,
                                  MDIRNL4KEY * sizeof(struct mag *));
#endif /* MALLOCFREEMDIR */
        if (ptr1 == MAP_FAILED) {
#if defined(ENOMEM)
            errno = ENOMEM;
#endif /* ENOMEM */
#if (MALLOCFREEMDIR)
            if (!mag) {
                ptr1 = tab[0];
                if (ptr1) {
                    unmapanon(ptr,
                              MDIRNL2KEY * sizeof(struct magitem));
                }   
                ptr1 = tab[1];
                if (ptr1) {
                    unmapanon(ptr1,
                              MDIRNL3KEY * sizeof(struct magitem));
                }
                
                return;
            } else {
                ptr1->nref++;
            }
            
            exit(1);
        } else if (!mag) {
            nref = --ptr1->nref;
            if (!nref) {
                tab[2] = ptr1;
            }
            ptr1 = tab[0];
            if (ptr1) {
                unmapanon(ptr1,
                          MDIRNL2KEY * sizeof(struct magitem));
            }   
            ptr1 = tab[1];
            if (ptr1) {
                unmapanon(ptr1,
                          MDIRNL3KEY * sizeof(struct magitem));
            }
            ptr1 = tab[2];
            if (ptr1) {
                unmapanon(ptr1,
                          MDIRNL4KEY * sizeof(struct magitem));
            }
            
            return;
        }
    } else if (ptr) {
        ptr1->nref++;
    }
    item = &((struct magitem **)ptr1)->ptr;
#else /* !MALLOCFREEMDIR */
    item = &((struct mag **)ptr1)[l4];
#endif /* MALLOCFREEMDIR */
#else /* !MDIRNL4BIT */
    item = &((struct mag **)ptr2)[l3];
#endif
    *item = mag;
    
    return;
}

#endif /* MALLOCHASH */

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
//        mtxlk(&arn->nreflk);
        for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
            mtxlk(&arn->magtab[bktid].lk);
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
            mtxunlk(&arn->magtab[bktid].lk);
        }
//        mtxunlk(&arn->nreflk);
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
#if (MALLOCBUFMAP)
    long        n = 0;
#endif
    
//    mtxlk(&arn->nreflk);
//    arn->nref--;
//    if (!arn->nref) {
        for (bktid = 0 ; bktid < MALLOCNBKT ; bktid++) {
            mtxlk(&arn->magtab[bktid].lk);
            head = arn->magtab[bktid].head;
            if (head) {
                mag = head;
                mag->adr = (void *)((uintptr_t)mag->adr | MAGGLOB);
                while (mag->next) {
                    mag = mag->next;
                    mag->adr = (void *)((uintptr_t)mag->adr | MAGGLOB);
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
        }
//    }
//    mtxunlk(&arn->nreflk);
    
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
#if (MMAP_DEV_ZERO)
    g_malloc.zerofd = open("/dev/zero", O_RDWR);
#endif
#if (MALLOCHASH)
    g_malloc.hashtab = mapanon(g_malloc.zerofd,
                               MALLOCNHASH * sizeof(struct hashlist));
#endif
    narn = MALLOCNARN;
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
    uint8_t     *ptrval = NULL;
#if (MALLOCSTKNDX)
    MAGPTRNDX   *stk;
    MAGPTRNDX   *tab;
#elif (MALLOCHACKS)
    uintptr_t   *stk = NULL;
    uintptr_t   *tab = NULL;
#else
    void       **stk = NULL;
    void       **tab = NULL;
#endif
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
#if (MALLOCDEBUG) && 0
        fprintf(stderr, "1\n");
#endif
#if (MALLOCSTKNDX)
        ptrval = magndx2ptr(mag, mag->stk[mag->cur]);
        mag->cur++;
#elif (MALLOCHACKS)
        ptrval = (void *)mag->stk[mag->cur++];
#else
        ptrval = mag->stk[mag->cur++];
#endif
#if (MALLOCDEBUG)
        assert(mag->cur <= mag->max);
#endif
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
        /* try to allocate from list of free magazines with active allocations */
#if (MALLOCDEBUG) && 0
        fprintf(stderr, "2\n");
#endif
        mtxunlk(&arn->magtab[bktid].lk);
        mtxlk(&g_malloc.magtab[bktid].lk);
        mag = g_malloc.magtab[bktid].head;
        if (mag) {
#if (MALLOCSTKNDX)
            ptrval = magndx2ptr(mag, mag->stk[mag->cur]);
            mag->cur++;
#elif (MALLOCHACKS)
            ptrval = (void *)mag->stk[mag->cur++];
#else
            ptrval = mag->stk[mag->cur++];
#endif
#if (MALLOCDEBUG)
            assert(mag->cur <= mag->max);
#endif
            if (mag->cur == mag->max) {
                if (mag->next) {
                    mag->next->prev = NULL;
                }
                g_malloc.magtab[bktid].head = mag->next;
                mag->adr = (void *)((uintptr_t)mag->adr & ~MAGGLOB);
                mag->prev = NULL;
                mag->next = NULL;
            }
            mtxunlk(&g_malloc.magtab[bktid].lk);
        } else {
#if (MALLOCDEBUG) && 0
            fprintf(stderr, "3\n");
#endif
            mtxunlk(&g_malloc.magtab[bktid].lk);
            mtxlk(&g_malloc.freetab[bktid].lk);
            mag = g_malloc.freetab[bktid].head;
            if (mag) {
#if (MALLOCDEBUG)
                if (mag->cur) {
                    fprintf(stderr, "mag->cur is %ld/%ld (not 0)\n", mag->cur, mag->max);
                    magprint(mag);
                }
                assert(mag->cur == 0);
#endif
#if (MALLOCSTKNDX)
                ptrval = magndx2ptr(mag, mag->stk[mag->cur]);
                mag->cur++;
#elif (MALLOCHACKS)
                ptrval = (void *)mag->stk[mag->cur++];
#else
                ptrval = mag->stk[mag->cur++];
#endif
                if (mag->next) {
                    mag->next->prev = NULL;
                }
                g_malloc.freetab[bktid].head = mag->next;
#if (MALLOCBUFMAP)
                g_malloc.freetab[bktid].n--;
#endif
                mtxunlk(&g_malloc.freetab[bktid].lk);
                mag->prev = NULL;
                mag->next = NULL;
                if (gtpow2(mag->max, 1)) {
                    /* queue magazine to partially allocated list */
                    mag->adr = (void *)((uintptr_t)mag->adr | MAGGLOB);
                    mtxlk(&g_malloc.magtab[bktid].lk);
                    mag->next = g_malloc.magtab[bktid].head;
                    if (mag->next) {
                        mag->next->prev = mag;
                    }
                    g_malloc.magtab[bktid].head = mag;
                    mtxunlk(&g_malloc.magtab[bktid].lk);
                }
                mtxunlk(&g_malloc.freetab[bktid].lk);
            } else {
                /* create new magazine */
#if (MALLOCDEBUG) && 0
                fprintf(stderr, "4\n");
#endif
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
                        /* use magazine header's data-field for allocation stack */
#if (MALLOCSTKNDX)
                        mag->stk = (MAGPTRNDX *)mag->data;
                        mag->ptrtab = &mag->stk[1UL << magnblklog2(bktid)];
#elif (MALLOCHACKS)
                        mag->stk = (uintptr_t *)mag->data;
                        mag->ptrtab = (uintptr_t *)&mag->stk[1UL << magnblklog2(bktid)];
#else
                        mag->stk = (void **)mag->data;
                        mag->ptrtab = &mag->stk[1UL << magnblklog2(bktid)];
#endif
#if (MALLOCFREEMAP)
                        mag->freemap = (uint8_t *)&mag->stk[(1UL << (magnblklog2(bktid) + 1))];
#endif
                    } else {
                        /* map new allocation stack */
                        stk = mapanon(g_malloc.zerofd, magnbytetab(bktid));
                        if (stk == MAP_FAILED) {
                            unmapanon(mag, MALLOCMAGSIZE);
                            
                            return NULL;
                        }
#if (MALLOCSTKNDX)
                        mag->stk = (MAGPTRNDX *)stk;
                        mag->ptrtab = &stk[1UL << magnblklog2(bktid)];
#else
                        mag->stk = stk;
                        mag->ptrtab = &stk[1UL << magnblklog2(bktid)];
#endif
#if (MALLOCFREEMAP)
                        mag->freemap = (uint8_t *)&mag->stk[(1UL << (magnblklog2(bktid) + 1))];
#endif
                    }
                    ptr = SBRK_FAILED;
                    if (bktid <= MALLOCSLABLOG2) {
                        /* try to allocate slab from heap */
                        mtxlk(&g_malloc.heaplk);
                        ptr = growheap(magnbyte(bktid));
                        mtxunlk(&g_malloc.heaplk);
                    }
                    if (ptr == SBRK_FAILED) {
#if 0
                        /* try to map slab */
                        ptr = mapanon(g_malloc.zerofd, magnbyte(bktid));
                        if (ptr == MAP_FAILED) {
                            unmapanon(mag, MALLOCMAGSIZE);
                            if (!magembedstk(bktid)) {
                                unmapanon(stk, magnbytetab(bktid));
                            }
                            
                            return NULL;
                        }
                        mapped = 1;
#endif
                        abort();
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
                    tab = mag->ptrtab;
                    /* initialise allocation stack */
                    incr = 1UL << bktid;
                    for (n = 0 ; n < max ; n++) {
#if (MALLOCSTKNDX)
                        stk[n] = (MAGPTRNDX)n;
                        tab[n] = (MAGPTRNDX)0;
#elif (MALLOCHACKS)
                        stk[n] = (uintptr_t)ptr;
                        tab[n] = (uintptr_t)0;
#else
                        stk[n] = ptr;
                        tab[n] = NULL;
#endif
                        ptr += incr;
                    }
                    if (gtpow2(max, 1)) {
                        /* queue slab with an active allocation */
                        mtxlk(&arn->magtab[bktid].lk);
                        mag->next = arn->magtab[bktid].head;
                        if (mag->next) {
                            mag->next->prev = mag;
                        }
                        arn->magtab[bktid].head = mag;
                        mtxunlk(&arn->magtab[bktid].lk);
                    }
//                        mag->stk = stk;
//                        mag->ptrtab = &stk[1UL << magnblklog2(bktid)];
                }
            }
        }
    }
    ptr = clrptr(ptrval);
    if (ptr) {
        /* TODO: unlock magtab earlier */
        if ((zero) && (((uintptr_t)ptrval & BLKDIRTY))) {
            memset(ptr, 0, 1UL << (bktid));
        }
        if ((align) && ((uintptr_t)ptr & (align - 1))) {
            ptr = ptralign(ptr, align);
        }
        /* store unaligned source pointer */
        magputptr(mag, ptr, clrptr(ptrval));
#if (MALLOCFREEMAP)
        if (bitset(mag->freemap, magptrndx(mag, ptr))) {
            fprintf(stderr, "trying to reallocate block");
            
            abort();
        }
        setbit(mag->freemap, magptrndx(mag, ptr));
#endif
        /* add magazine to lookup structure using retptr as key */
        setmag(ptr, mag);
#if defined(ENOMEM)
    } else {
        errno = ENOMEM;
#endif
    }
#if (MALLOCDEBUG)
    if (!ptr) {
        magprint(mag);
    }
    assert(mag != NULL);
    assert(ptr != NULL);
#endif
    
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
        mtxlk(&g_malloc.magtab[bktid].lk);
        /* remove pointer from allocation lookup structure */
        setmag(ptr, NULL);
#if (MALLOCFREEMAP)
        if (!bitset(mag->freemap, magptrndx(mag, ptr))) {
            fprintf(stderr, "trying free an unused block\n");

            abort();
        }
        clrbit(mag->freemap, magptrndx(mag, ptr));
#endif
        ptr = maggetptr(mag, ptr);
//        arn = g_malloc.arntab[arnid];
        max = mag->max;
        bktid = mag->bktid;
#if (MALLOCSTKNDX)
        mag->stk[--mag->cur] = magptr2ndx(mag, ptr);
#elif (MALLOCHACKS)
        mag->stk[--mag->cur] = (uintptr_t)ptr | BLKDIRTY;
#else
        mag->stk[--mag->cur] = (void *)((uintptr_t)ptr | BLKDIRTY);
#endif
        if (!mag->cur) {
            if (gtpow2(max, 1)) {
                /* remove magazine from partially allocated list */
                if (mag->prev) {
                    mag->prev->next = mag->next;
                } else if ((uintptr_t)mag->adr & MAGGLOB) {
                    g_malloc.magtab[bktid].head = mag->next;
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
                mtxlk(&g_malloc.freetab[bktid].lk);
                mag->next = g_malloc.freetab[bktid].head;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                g_malloc.freetab[bktid].head = mag;
                mtxunlk(&g_malloc.freetab[bktid].lk);
            }
            /* allocate from list of partially allocated magazines */
        } else if (mag->cur == max - 1) {
//            mtxlk(&arn->magtab[bktid].lk);
            mag->prev = NULL;
            /* remove [fully allocated] magazine from partial list */
            mag->next = arn->magtab[bktid].head;
            if (mag->next) {
                mag->next->prev = mag;
            }
            arn->magtab[bktid].head = mag;
//            mtxunlk(&arn->magtab[bktid].lk);
        }
        mtxunlk(&g_malloc.magtab[bktid].lk);
        mtxunlk(&arn->magtab[bktid].lk);
#if !(MALLOCBUFMAP)
//        mtxunlk(&arn->magtab[bktid].lk);
#endif
        if (freemap) {
#if (MALLOCBUFMAP)
            if (bktid > MALLOCSLABLOG2) {
                mtxlk(&g_malloc.freetab[bktid].lk);
                if (g_malloc.freetab[bktid].n < magnbufmap(bktid)) {
                    mag->prev = NULL;
                    mag->next = g_malloc.freetab[bktid].head;
                    if (mag->next) {
                        mag->next->prev = mag;
                    }
                    g_malloc.freetab[bktid].head = mag;
                    g_malloc.freetab[bktid].n++;
                    freemap = 0;
                }
                mtxunlk(&g_malloc.freetab[bktid].lk);
            }
            if (freemap) {
                mtxunlk(&g_malloc.freetab[bktid].lk);
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
#else
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
#endif
        }
#if (MALLOCBUFMAP)
//        mtxunlk(&arn->magtab[bktid].lk);
#endif
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
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif

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
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif

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
#if (MALLOCDEBUG)
    assert(retptr != NULL);
#endif

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
    void   *ptr = NULL;
    size_t  aln = max(align, MALLOCMINSIZE);

#if defined(_GNU_SOURCE) && (GNUMALLOCHOOKS)
    if (__memalign_hook) {
        void *caller;
        
        caller = m_getretadr();
        __memalign_hook(align, size, (const void *)caller);
    }
#endif
    if (!powerof2(aln) || (size & (aln - 1))) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, aln, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif

    return ptr;
}

#endif

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
int
posix_memalign(void **ret,
               size_t align,
               size_t size)
{
    void   *ptr = NULL;
    size_t  aln = max(align, MALLOCMINSIZE);
    int     retval = -1;

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
        ptr = _malloc(size, aln, 0);
        if (ptr) {
            retval = 0;
        }
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif
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
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif
    
    return ptr;
}
#endif

void *
memalign(size_t align,
         size_t size)
{
    void   *ptr = NULL;
    size_t  aln = max(align, MALLOCMINSIZE);

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
        ptr = _malloc(size, aln, 0);
    }
#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif

    return ptr;
}

#if (_BSD_SOURCE)
void *
reallocf(void *ptr,
         size_t size)
{
    void *retptr = _realloc(ptr, size, 1);

#if (MALLOCDEBUG)
    assert(retptr != NULL);
#endif

    return retptr;
}
#endif

#if defined(_GNU_SOURCE)
void *
pvalloc(size_t size)
{
    size_t  sz = rounduppow2(size, PAGESIZE);
    void   *ptr = _malloc(sz, PAGESIZE, 0);

#if (MALLOCDEBUG)
    assert(ptr != NULL);
#endif

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

