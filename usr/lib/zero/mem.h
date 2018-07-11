#ifndef __ZERO_MEM_H__
#define __ZERO_MEM_H__

#include <mach/param.h>
#include <mach/asm.h>
#include <zero/bits/mem.h>
#include <zero/mtx.h>
#include <zero/hash.h>
#include <zero/fastudiv.h>

#define MEM_MAX_FAST_DIV     1023
#define memfastdiv(num, div)                                            \
    (((div) <= MEM_MAX_FAST_DIV                                         \
      ? fastu16divu16(num, div, fastu16divu16tab)                       \
      : ((num) / (div))))
/*
 * allocation anatomy
 * ------------------
 */
#define MEM_BLK_MASK         (~(MEM_MIN_SIZE - 1))
#define memblkid(ptr)        ((uintptr_t)(ptr) & MEM_BLK_MASK)

/* stored together with slab address to denote slab type */
#define MEM_BLK_SLAB         0x1
#define MEM_RUN_SLAB         0x2
#define MEM_MID_SLAB         0x3
#define MEM_SLAB_TYPE_BITS   2 // # of bits used for slab type
#define MEM_SLAB_LOW_BITS    PAGESIZELOG2 // info
#define MEM_SLAB_BLK_BITS    (MEM_SLAB_LOW_BITS - MEM_SLAB_TYPE_BITS) // ID bits
#define MEM_SLAB_BLK_IDS     (1 << MEM_SLAB_BLK_BITS) // max # of IDs
#define MEM_SLAB_BLK_MASK    (~(((uintptr_t)1 << MEM_SLAB_LOW_BITS) - 1)) // ID

/* slab book-keeping info */
#define mempageid(ptr)       ((uintptr_t)(ptr) & ((uintptr_t)1 << PAGESIZELOG2))

/* per-pool # of allocation blocks */
#define memnumblk(pool)                                               \
    ((uintptr_t)1 << (PAGESIZELOG2 + MEM_BLK_SLAB_SHIFT - (pool)))
#define memnumrun(pool)                                                 \
    (memfastdiv(MEM_MAX_RUN_PAGES, (pool) + 1))
#define memnummid(pool)                                                 \
    (memfastdiv(MEM_MAX_MID_PAGES, ((pool) + 1) << MEM_MID_UNIT_SHIFT))

#define memblksize(pool)     ((size_t)1 << (pool))
#define memrunsize(pool)     (PAGESIZE * ((pool) + 1))
#define memmidsize(pool)     ((PAGESIZE << MEM_MID_UNIT_SHIFT) * ((pool) + 1))
#define membigsize(sz)       rounduppow2(sz, PAGESIZE)

/*
 * dynamic memory management
 * -------------------------
 * - minimum allocation size is a cacheline (may be wasteful; see <bits/mem.h>)
 * - blocks are 1..PAGESIZE / 2 bytes in size
 *   - allocated from thread-local pools
 * - runs are PAGESIZE..MEM_MAX_RUN_PAGES * PAGESIZE bytes in size
 *   - allocated from thread-local pools
 * - middle-size blocks are multiples of 8 * PAGESIZE up to 512 * PAGESIZE
 *   - allocated from global pools
 * - big blocks are allocated individually
 * - blocks are prefixed with allocation info structure struct memblk;
 *   - other allocations have page-entries in g_mem.hashtab
 */
#define MEM_SLAB_PAGE_SHIFT 3
#define MEM_BLK_SLAB_SIZE   (PAGESIZE << MEM_SLAB_PAGE_SHIFT)
#define MEM_BLK_POOLS       (PAGESIZELOG2 - CLSIZELOG2)
#define MEM_MAX_BLK_POOL    (MEM_BLK_POOLS - 1)
#define MEM_MAX_BLK_SIZE    PAGESIZE
#define MEM_RUN_POOLS       (MEM_MAX_RUN_PAGES)
#define MEM_MAX_RUN_SHIFT   6
#define MEM_MAX_RUN_PAGES   (1 << MEM_MAX_RUN_SHIFT)
#define MEM_MAX_RUN_POOL    (MEM_MAX_RUN_PAGES - 1)
#define MEM_MIN_RUN_SIZE    PAGESIZE
#define MEM_MAX_RUN_SIZE    (MEM_MAX_RUN_PAGES * PAGESIZE)
#define MEM_MID_POOLS       32
#define MEM_MID_UNIT_SHIFT  2
#define MEM_MID_UNIT_PAGES  (1 << MEM_MID_UNIT_SHIFT)
#define MEM_MAX_MID_PAGES   (MEM_MID_UNIT_PAGES * MEM_MID_POOLS)
#define MEM_MIN_MID_SIZE    (MEM_MID_UNIT_PAGES * PAGESIZE)
#define MEM_MAX_MID_SIZE    (MEM_MAX_MID_PAGES * PAGESIZE)

/* allocation book-keeping */
/* in-slab allocation # */
#define memptrid(slab, ptr)  (((uint8_t *)(ptr) - (slab)->base) / (slab->bsz))
/* in-slab allocation address for block # */
#define memgetptr(slab, num) ((mem)->base + (num) * (slab)->bsz)

/* lowest-order bit lock for pointers */
#define memtrylkptr(ptr)                                                \
    (!m_cmpsetbit((m_atomic_t *)(ptr), MEM_ADR_LK_BIT_POS))

static __inline__ void
memlkptr(m_atomicptr_t *ptr)
{
    do {
        while (*(m_atomic_t *)ptr & MEM_ADR_LK_BIT) {
            m_waitspin();
        }
        if (memtrylkptr(ptr)) {

            break;
        }
    } while (1);

    return;
}

/* memory slab with buffer-stack */
#define MEM_SLAB_SIZE (2 * PAGESIZE)
#define MEM_SLAB_BLKS (PAGESIZE / sizeof(void *))
struct memslab {
    struct memslab  *prev; // pointer to previous in list
    struct memslab  *next; // pointer to next in list
    uint8_t         *base; // slab/map base address
    size_t           pool; // allocation pool
    size_t           nblk; // number of total blocks
    size_t           bsz; // block size in bytes
    m_atomic_t       ndx; // current index into allocation table
    uint8_t          type;
    void           **tab; // allocation pointer table, used in stack-fashion
};

#define MEM_TLS_SIZE PAGESIZE
struct memtls {
    struct memslab *blktab[MEM_BLK_POOLS];
    struct memslab *runtab[MEM_RUN_POOLS];
};

/* per-thread memory arena */
struct memtlsqueue {
    struct memtls  *tls;
    struct memslab *queue;
};

#define MEM_BLK_ID_BITS     16
#define MEM_BLK_POOL_BITS   8
#define memgetblkid(nfo)                                                \
    ((nfo) & ((1L << PAGESIZELOG2) - 1))
#define memgetblkpool(nfo)                                              \
    (((nfo) >> MEM_BLK_ID_BITS) & ((1L << MEM_BLK_ID_BITS) - 1))
#define memlkblk(blk)                                                   \
    (!m_cmpsetbit((m_atomic_t *)&((void *)(blk)[-1]), MEM_ADR_LK_BIT_POS))
#define memunlkblk(blk)                                                 \
    (m_clrbit((m_atomic_t *)&((void *)(blk)[-1]), MEM_ADR_LK_BIT_POS))
#define memgetinfo(blk)                                                 \
    (*((uintptr_t *)(blk)[-2]))
struct memblk {
    uintptr_t  info;
    void      *ptr;
};

#define MEM_INIT_BIT (1 << 0)
/* global allocator data */
#define MEM_HASH_ITEMS 16384
struct mem {
    struct thashtab *hash[MEM_HASH_ITEMS];
    struct memslab  *midtab[MEM_MID_POOLS];
    m_atomic_t       lk;
    uintptr_t        flg;
    uintptr_t        nbused;
    uintptr_t        nbres;
    uintptr_t        nbfree;
};

static __inline__ void *
mempopblk(struct memslab *slab, struct memslab **headret)
{
    m_atomic_t  ndx = m_fetchadd(&slab->ndx, 1);
    intptr_t    n = slab->nblk;
    void       *ptr = NULL;

    if (ndx < n - 1) {
        ptr = slab->tab[ndx];
    } else if (ndx == n - 1) {
        *headret = slab->next;
    } else {
        m_atomdec((m_atomic_t *)slab->ndx);
    }

    return ptr;
}

static __inline__ void
mempushblk(struct memslab *slab, void *ptr, struct memslab **tailret)
{
    m_atomic_t ndx = m_fetchadd(&slab->ndx, -1);
    intptr_t   n = slab->nblk;

    if (ndx > 0) {
        ndx--;
        slab->tab[ndx] = ptr;
        if (ndx == n - 1) {
            *tailret = slab;
        }
    } else {
        m_atominc(&slab->ndx);
    }

    return;
}

#endif /* __ZERO_MEM_H__ */

