#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/mem.h>

typedef struct membin * memallocbinfunc(struct mem *mem, long slot);
typedef void * meminitbinfunc(struct mem *mem, struct membin *bin);

static memallocbinfunc *membinallocfuncs[MEMBINTYPES] ALIGNED(CLSIZE);
static meminitbinfunc  *membininitfuncs[MEMBINTYPES];

static THREADLOCAL struct memarn     *tls_arn ALIGNED(CLSIZE);
#if (MEM_LK_TYPE == MEM_LK_PRIO)
static THREADLOCAL struct priolkdata  tls_priolkdata;
#endif
static THREADLOCAL pthread_once_t     tls_once;
static THREADLOCAL pthread_key_t      tls_key;
static THREADLOCAL long               tls_flg;

static struct membin *
mamallocsmallbin(struct mem *mem, long slot)
{
    MEMPTR_T      *adr = SBRK_FAILED;
    MEMWORD_T      binsz = memsmallbinsize(slot);
    MEMWORD_T      flg = 0;
    struct membin *bin;

    if (!(mem->flg & MEMNOHEAPBIT)) {
        /* try to allocate from heap (sbrk()) */
        memgetlk(&mem->heaplk);
        adr = growheap(binsz);
        if (adr == SBRK_FAILED) {
            memrellk(&mem->heaplk);
        } else {
            flg = MEMHEAPBIT;
        }
    }
    if (adr == SBRK_FAILED) {
        /* sbrk() failed, let's try mmap() */
        memrellk(&mem->heaplk);
        adr = mapanon(0, binsz);
        flg = MEMMAPBIT;
        if (adr == MAP_FAILED) {
#if defined(ENOMEM)
            errno = ENOMEM;
#endif

            return NULL;
        }
    }
    bin = adr;
    bin->flg = flg;             // possible MEMMAPBIT
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
    long           flg = bin->flg;
    MEMPTR_T       ptr = (MEMPTR_T)rounduppow2((MEMADR_T)adr
                                               + sizeof(struct membin),
                                               blksz);
    struct membin *bptr;

    membininitfree(bin);        // zero freemap, mark block #1 (0 is bin) in use
    if (!flg) {
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
    long           flg;

    /* mmap() blocks */
    adr = mapanon(0, mapsz);
    bin = adr;
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
    bin->flg = MEMMAPBIT;
    bin->base = adr;            // our newly allocated region
    bin->slot = slot;           // slot #

    return bin;
}

static void *
meminitpagebin(struct mem *mem, struct membin *bin)
{
    long       slot = bin->slot;
    MEMPTR_T   adr = bin->base;
    MEMWORD_T  blksz = slot * PAGESIZE;
//    long       flg = bin->flg;
    MEMPTR_T   ptr = (MEMPTR_T)rounduppow2((MEMADR_T)adr
                                           + sizeof(struct membin),
                                           blksz);

    membininitfree(bin);        // zero freemap, mark block #1 (0 is bin) in use

    return ptr;
}

static struct membin *
memallocbigbin(struct mem *mem, long slot)
{
    MEMWORD_T      mapsz = membigbinsize(slot);
    MEMPTR_T       adr;
    struct membin *bin;

    /* mmap() blocks */
    adr = mapanon(0, mapsz);
    bin = adr;
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
    bin->flg = MEMMAPBIT;
    bin->base = adr;            // our newly allocated region
    bin->slot = slot;           // slot #

    return adr;
}

static void *
meminitbigbin(struct mem *mem, struct membin *bin)
{
    long       slot = bin->slot;
    MEMPTR_T   adr = bin->base;
    MEMUWORD_T blksz = MEMWORD(1) << (slot);
//    long       flg = bin->flg;
    MEMPTR_T   ptr = (MEMPTR_T)rounduppow2((MEMADR_T)adr
                                           + sizeof(struct membin),
                                           blksz);

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
    long           flg;
    struct membin *bptr;
    MEMADR_T       upval;
    MEMWORD_T      binsz;

    if (!type) {
        bkt = &arn->small[slot]->list;
    } else if (type == 1) {
        bkt = &arn->page[slot]->list;
    } else {
        bkt = &arn->big[slot]->list;
    }

    memlkbit(&bkt->list);
    bin = membinallocfuncs[type](mem, slot);
    if (bin) {
        ptr = membininitfuncs[type](mem, bin);
        /* link bin to bucket */
        upval = bkt->list;
        upval &= ~MEMLKBIT;
        bptr = (struct membin *)upval;
        if (bptr) {
            bptr->prev = bin;
        }
        bin->prev = NULL;
        bin->next = bptr;
        bin->bkt = bkt;
        bin->atab = NULL;           // allocate on-demand
        if (flg & MEMHEAPBIT) {
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

/* find a bin or magazine address */
static void *
memfindbin(void *ptr)
{
    ;
}

