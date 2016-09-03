#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/mem.h>

typedef struct membin * memallocbinfunc(struct mem *mem, long slot);
typedef void * meminitbinfunc(struct mem *mem, struct membin *bin);

static THREADLOCAL struct memarn     *tls_arn ALIGNED(PAGESIZE);
#if (MEM_LK_TYPE == MEM_LK_PRIO)
static THREADLOCAL struct priolkdata  tls_priolkdata;
#endif
static THREADLOCAL pthread_once_t     tls_once;
static THREADLOCAL pthread_key_t      tls_key;
static THREADLOCAL long               tls_flg;
static struct                         mem g_mem ALIGNED(CLSIZE);

static struct membin *
memallocsmallbin(struct mem *mem, long slot)
{
    MEMPTR_T       adr = SBRK_FAILED;
    MEMWORD_T      binsz = memsmallbinsize(slot);
    MEMUWORD_T     info = 0;
    struct membin *bin;

    if (!(mem->flg & MEMNOHEAPBIT)) {
        /* try to allocate from heap (sbrk()) */
        memgetlk(&mem->heaplk);
        adr = growheap(binsz);
        if (adr == SBRK_FAILED) {
            memrellk(&mem->heaplk);
        } else {
            info = MEMHEAPBIT;
        }
    }
    if (adr == SBRK_FAILED) {
        /* sbrk() failed, let's try mmap() */
        memrellk(&mem->heaplk);
        adr = mapanon(0, binsz);
        if (adr == MAP_FAILED) {
#if defined(ENOMEM)
            errno = ENOMEM;
#endif

            return NULL;
        }
    }
    bin = (struct membin *)adr;
    bin->info = info;             // possible MEMMAPBIT
    bin->base = adr;            // our newly allocated region
    bin->slot = slot;           // slot #

    return bin;
}

static void *
meminitsmallbin(struct mem *mem, struct membin *bin)
{
    long           slot = bin->slot;
    MEMPTR_T       adr = bin->base;
    MEMWORD_T      blksz = MEMWORD(1) << (slot);
    MEMUWORD_T     info = bin->info;
    MEMPTR_T       ptr = (MEMPTR_T)rounduppow2((MEMADR_T)adr
                                               + sizeof(struct membin),
                                               blksz);
    struct membin *bptr;

    membininitfree(bin);        // zero freemap, mark block #1 (0 is bin) in use
    if (!info) {
        /* link block from sbrk() to global heap (put it on top) */
        bptr = mem->heap;
        bin->heap = bptr;
    }

    return ptr;
}

static struct membin *
memallocpagebin(struct mem *mem, long slot)
{
    MEMWORD_T      mapsz = mempagebinsize(slot);
    MEMPTR_T       adr;
    struct membin *bin;

    /* mmap() blocks */
    adr = mapanon(0, mapsz);
    bin = (struct membin *)adr;
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
    bin->base = adr;
    // our newly allocated region
    bin->slot = slot;           // slot #

    return bin;
}

static void *
meminitpagebin(struct mem *mem, struct membin *bin)
{
    long       slot = bin->slot;
    MEMPTR_T   adr = bin->base;
    MEMWORD_T  blksz = slot * PAGESIZE;
//    long       info = bin->info;
    MEMPTR_T   ptr = (MEMPTR_T)rounduppow2((MEMADR_T)adr
                                           + sizeof(struct membin),
                                           blksz);

    membininitfree(bin);        // zero freemap, mark block #1 (0 is bin) in use

    return ptr;
}

static struct membin *
memallocbigbin(struct mem *mem, long slot, MEMUWORD_T nblk)
{
    MEMWORD_T      mapsz = membigbinsize(slot, nblk);
    MEMPTR_T       adr;
    struct membin *bin;

    /* mmap() blocks */
    adr = mapanon(0, mapsz);
    bin = (struct membin *)adr;
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
    bin->base = adr;            // our newly allocated region
    bin->slot = slot;           // slot #

    return bin;
}

static void *
meminitbigbin(struct mem *mem, struct membin *bin, MEMUWORD_T nblk)
{
    long       slot = bin->slot;
    MEMPTR_T   adr = bin->base;
    MEMUWORD_T blksz = MEMWORD(1) << (slot);
//    long       info = bin->info;
    MEMPTR_T   ptr = (MEMPTR_T)rounduppow2((MEMADR_T)adr
                                           + sizeof(struct membin),
                                           blksz);

    memsetbinnblk(bin, nblk);
    membininitfree(bin);        // zero freemap, mark block #1 (0 is bin) in use

    return ptr;
}

static void *
memmkbin(struct mem *mem, long slot, long type)
{
    struct memarn *arn = tls_arn;
    MEMPTR_T       ptr = NULL;
    struct membkt *bkt;
    struct membin *bin;
    MEMUWORD_T     info;
    MEMUWORD_T     nblk = 1;
    struct membin *bptr;
    MEMADR_T       upval;

    if (!type) {
        bkt = &arn->small[slot];
        memlkbit(&bkt->list);
        bin = memallocsmallbin(mem, slot);
        if (bin) {
            meminitsmallbin(mem, bin);
        }
    } else if (type == 1) {
        bkt = &arn->page[slot];
        memlkbit(&bkt->list);
        bin = memallocpagebin(mem, slot);
        if (bin) {
            meminitpagebin(mem, bin);
        }
    } else {
        bkt = &arn->big[slot];
        memlkbit(&bkt->list);
        bin = memallocbigbin(mem, slot, nblk);
        if (bin) {
            meminitbigbin(mem, bin, nblk);
        }
    }
    if (bin) {
        /* link bin to bucket */
        upval = (MEMADR_T)bkt->list;
        upval &= ~MEMLKBIT;
        bptr = (struct membin *)upval;
        if (bptr) {
            bptr->prev = bin;
        }
        bin->prev = NULL;
        bin->next = bptr;
        bin->bkt = bkt;
        bin->atab = NULL;           // allocate on-demand
        if (info & MEMHEAPBIT) {
            /* this unlocks the global heap (low-bit becomes zero) */
            m_syncwrite(&mem->heap, bin);
        }
        /* this unlocks the arena slot (low-bit becomes zero) */
        m_syncwrite(&bkt->list, bin);
    } else {
        memrelbit(&bkt->list);
    }

    return ptr;                 // return pointer to first block
}

/* find a bin address; type encoded in the low 2 bits */
static void *
memputbin(void *ptr, long type)
{
    struct memitem *itab;
    struct memitem *item;
    long            k1;
    long            k2;
    long            k3;
    long            k4;
    void           *pstk[2] = { NULL };

    memgetkeybits(ptr, k1, k2, k3, k4);
    memgetlk(&g_mem.tab[k1].lk);
    itab = g_mem.tab[k1].tab;
    if (!itab) {
        itab = mapanon(0, MEMLVLITEMS * sizeof(struct memitem));
        if (itab == MAP_FAILED) {

            return NULL;
        }
        pstk[0] = itab;
        g_mem.tab[k1].tab = itab;
    }
    if (itab) {
        item = &itab[k2];
        itab = item->tab;
        if (!itab) {
            itab = mapanon(0, MEMLVLITEMS * sizeof(struct memitem));
            if (itab == MAP_FAILED) {
                unmapanon(pstk[0], MEMLVLITEMS * sizeof(struct memitem));
                
                return NULL;
            }
            pstk[1] = itab;
            item->tab = itab;
        }
        if (itab) {
            item = &itab[k3];
            itab = item->tab;
            if (!itab) {
                itab = mapanon(0, MEMLVLITEMS * sizeof(MEMADR_T));
                if (itab == MAP_FAILED) {
                    unmapanon(pstk[0], MEMLVLITEMS * sizeof(struct memitem));
                    unmapanon(pstk[1], MEMLVLITEMS * sizeof(struct memitem));
                    
                    return NULL;
                }
                pstk[2] = itab;
                item->tab = itab;
            }
            if (itab) {
                ((MEMADR_T *)itab)[k4] = (MEMADR_T)ptr | type;
            }
        }
    }
    memrellk(&g_mem.tab[k1].lk);

    return ptr;
}

static MEMADR_T
memfindbin(void *ptr)
{
    MEMADR_T        ret = 0;
    struct memitem *itab;
    struct memitem *item;
    long            k1;
    long            k2;
    long            k3;
    long            k4;

    memgetkeybits(ptr, k1, k2, k3, k4);
    memgetlk(&g_mem.tab[k1].lk);
    itab = g_mem.tab[k1].tab;
    if (itab) {
        item = &itab[k2];
        itab = item->tab;
        if (itab) {
            item = &itab[k3];
            itab = item->tab;
            if (itab) {
                ret = ((MEMADR_T *)itab)[k4];
            }
        }
    }
    memrellk(&g_mem.tab[k1].lk);

    return ret;
}

