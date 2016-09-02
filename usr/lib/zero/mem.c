#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/mem.h>

static THREADLOCAL struct memarn     *tls_arn ALIGNED(CLSIZE);
#if (MEM_LK_TYPE == MEM_LK_PRIO)
static THREADLOCAL struct priolkdata  tls_priolkdata;
#endif
static THREADLOCAL pthread_once_t     tls_once;
static THREADLOCAL pthread_key_t      tls_key;
static THREADLOCAL long               tls_flg;
/*
 * allocate a bin with MEM_BIN_MAX_BLOCKS blocks of 1 << slot bytes
 * - try sbrk() for a heap allocation first, use mmap() if that should fail
 */
static void *
meminitbin(struct mem *mem, long slot)
{
    struct memarn    *arn = tls_arn;
    MEMUWORD_T        binsz = membinsize(slot);
    MEMUWORD_T        blksz = MEMUWORD(1) << (slot);
    MEMWORD_T         flg = 0;
    MEMPTR_T         *adr = SBRK_FAILED;
    struct membinbkt *bkt = &arn->qtab[slot]->list;
    struct membin    *bin;
    MEMPTR_T          ptr;
    struct membin    *bptr;
    MEMADR_T          upval;

    if (!(mem->flg & MEMNOHEAPBIT)) {
        /* try to allocate from heap (sbrk()) */
        memgetlk(&mem->heaplk);
        adr = growheap(binsz);
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
    bin = (struct membin *)adr;
    ptr = (MEMPTR_T)rounduppow2((MEMADR_T)adr
                                 + sizeof(struct membin),
                                 blksz);
    bin->flg = flg;
    bin->base = adr;            // our newly allocated region
    bin->bkt = bkt;             // parent bucket
    bin->slot = slot;           // slot #
    membininitfree(bin);        // zero freemap, mark block #1 (0 is bin) in use
    if (!flg) {
        /* link block from sbrk() to global heap (put it on top) */
        bptr = mem->heap;
        bin->heap = bptr;
    }
    /* link bin to bucket */
    memlkbit(&bkt->list);
    upval = bkt->list;
    upval &= ~MEMLKBIT;
    bptr = (struct membin *)upval;
    if (bptr) {
        bptr->prev = bin;
    }
    bin->prev = NULL;
    bin->next = bptr;
    if (!flg) {
        /* this unlocks the global heap (low-bit becomes zero) */
        m_syncwrite(&mem->heap, bin);
    }
    /* this unlocks the arena slot (low-bit becomes zero) */
    m_syncwrite(&bkt->list, bin);

    return ptr;                 // return pointer to first block
}

/*
 * allocate a bin with MEM_BIN_MAX_BLOCKS blocks of 1 << slot bytes
 * - the first block is the header
 * - try sbrk() for a heap allocation first, use mmap() if that should fail
 */
static void *
meminitmap(struct mem *mem, long slot)
{
    struct memarn    *arn = tls_arn;
    MEMUWORD_T        mapsz = memmapsize(slot);
    MEMUWORD_T        blksz = MEMUWORD(1) << (slot);
    MEMWORD_T         flg = 0;
    struct membinbkt *bkt = &arn->mtab[slot]->list;
    MEMPTR_T          adr;
    struct membin    *bin;
    MEMPTR_T          ptr;
    struct membin    *bptr;
    MEMADR_T          upval;

    /* mmap() blocks */
    memrellk(&mem->heaplk);
    adr = mapanon(0, mapsz);
    flg = MEMMAPBIT;
    if (adr == MAP_FAILED) {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
        
        return NULL;
    }
    bin = (struct membin *)adr;
    ptr = (MEMPTR_T)rounduppow2((MEMADR_T)adr
                                 + sizeof(struct membin),
                                 PAGESIZE);
    bin->flg = flg;             // MAMMAPBIT
    bin->base = adr;            // our newly allocated region
    bin->bkt = bkt;             // parent bucket
    bin->slot = slot;           // allocations 1 << slot, PAGESIZE * slot
    membininitfree(bin);        // zero freemap, mark block #1 (0 is bin) in use
    bin->atab = NULL;           // allocate on-demand
    if (!flg) {
        /* link block from sbrk() to global heap (put it on top) */
        bptr = mem->heap;
        bin->heap = bptr;
    }
    /* link bin to bucket */
    memlkbit(&bkt->list);
    upval = bkt->list;
    upval &= ~MEMLKBIT;
    bptr = (struct membin *)upval;
    if (bptr) {
        bptr->prev = bin;
    }
    bin->prev = NULL;
    bin->next = bptr;
    if (!flg) {
        /* this unlocks the global heap (low-bit becomes zero) */
        m_syncwrite(&mem->heap, bin);
    }
    /* this unlocks the arena slot (low-bit becomes zero) */
    m_syncwrite(&bkt->list, bin);

    return ptr;                 // return pointer to first block
}

/* find a bin or magazine address */
static void *
memfindpool(void *ptr)
{
    ;
}

