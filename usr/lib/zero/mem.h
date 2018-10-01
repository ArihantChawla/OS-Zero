#ifndef __ZERO_MEM_H__
#define __ZERO_MEM_H__

#include <zero/conf.h>
#include <stddef.h>
#include <mach/param.h>
#include <mach/asm.h>
#include <zero/hash.h>
#include <zero/fastudiv.h>
#include <zero/bits/mem.h>

#if defined(ZEROMEMTKTLK)
#include <mt/tktlk.h>
#define memlkmag(mp)   tktlk(&mag->lk)
#define memunlkmag(mp) tktunlk(&mag->lk)
#else
#include <mt/mtx.h>
#define memlkmag(mp)   fmtxlk(&mag->lk)
#define memunlkmag(mp) fmtxunlk(&mag->lk)
#endif

#define MEM_MAX_FAST_DIV     1023
#define memfastdiv(num, div)                                            \
    (((div) <= MEM_MAX_FAST_DIV                                         \
      ? fastu16divu16(num, div, fastu16divu16tab)                       \
      : ((num) / (div))))
/*
 * allocation anatomy
 * ------------------
 * - minimum allocation size is a cacheline (may be wasteful; see <bits/mem.h>)
 * - blocks are 1..PAGESIZE bytes in size
 *   - allocated from thread-local pools
 * - runs are PAGESIZE..MEM_RUN_MAX_PAGES * PAGESIZE (64 * PAGESIZE) bytes
 *   - allocated from thread-local pools
 * - middle-size blocks are multiples of PAGESIZE << MEM_MID_PAGE_SHIFT
 *   (4 * PAGE_SIZE) from 128 * PAGESIZE up to 256 * PAGESIZE bytes
 *   - allocated from global pools
 * - big blocks are allocated individually
 * - blocks are prefixed with allocation info structure struct memblk
 *   - other allocations have page-entries in g_mem.hashtab
 */
/* allocation types stored together with slab address */
#define MEM_BLK_MASK        (~(MEM_MIN_SIZE - 1))
#define MEM_BLK_SLAB        1
#define MEM_RUN_SLAB        2
#define MEM_MID_SLAB        3
#define MEM_TYPE_BITS       2 // # of bits used for slab type
#define MEM_TYPE_MASK       ((1U << MEM_SLAB_TYPE_BITS) - 1)
#define MEM_INFO_BITS       PAGESIZELOG2 // info, minimum 12
#define MEM_BLK_BITS        (MEM_SLAB_INFO_BITS - MEM_SLAB_TYPE_BITS) // ID bits
#define MEM_BLK_PAGE_BITS   (PTRBITS - MEM_INFO_BITS)
#define MEM_BLK_MAX_SIZE    (PAGESIZE / 2)
#define MEM_BLK_POOLS       PAGESIZELOG2
#define MEM_BLK_SLAB_SIZE   (8 * PAGESIZE)
#define MEM_RUN_MAX_PAGES   32
#define MEM_RUN_POOLS       MEM_RUN_MAX_PAGES
#define MEM_RUN_MAX_SIZE    (MEM_RUN_MAX_PAGES * PAGESIZE)
#define MEM_RUN_SLAB_SIZE   MEM_RUN_MAX_SIZE
#define MEM_MID_MIN_PAGES   (2 * MEM_RUN_MAX_PAGES)
#define MEM_MID_MAX_PAGES   512
#define MEM_MID_MIN_SIZE    (PAGESIZE * MEM_MID_MIN_PAGES)
#define MEM_MID_MAX_SIZE    (MEM_MID_MAX_PAGES * PAGESIZE)
#define MEM_MID_PAGE_SHIFT  2
#define MEM_MID_POOLS       ((MEM_MID_MAX_PAGES - MEM_MID_MIN_PAGES) \
                             >> MEM_MID_PAGE_SHIFT)

/* slab book-keeping info */
#define MEM_PAGE_MASK       (~(((uintptr_t)1 << PAGESIZELOG2) - 1))
#define memblkpage(adr)     ((uintptr_t)(ptr) & MEMPAGEMASK)
#define mempagenum(adr)     ((uintptr_t)(ptr) >> PAGESIZELOG2)
#define memblktype(adr)     ((uintptr_t)adr & MEM_BLK_TYPE_MASK)
#define memblkid(adr)       ((uintptr_t)adr & MEM_BLK_MASK)

/* per-pool # of allocation blocks */
#define memnumblk(pool)                                                 \
    (MEM_BLK_SLAB_SIZE >> (pool))
#define memnumrun(pool)                                                 \
    (memfastdiv(MEM_RUN_MAX_PAGES, (pool) + 1))
#define memnummid(pool)                                                 \
    (memfastdiv(MEM_MID_MAX_PAGES, ((pool) + 1) << MEM_MID_PAGE_SHIFT))

/* [per-pool] allocation sizes */
#define memblksize(pool) ((size_t)1 << (pool))
#define memrunsize(pool) ((size_t)PAGESIZE * ((pool) + 1))
#define memmidsize(pool) (MEM_MID_MIN_SIZE + (PAGESIZE << MEM_MID_PAGE_SHIFT) * pool)
#define membigsize(sz)   rounduppow2(sz, PAGESIZE)

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
    void           **stk;  // allocation pointer table, used in stack-fashion
    m_atomic_t       ndx;  // current index into allocation table
    uint8_t         *base; // slab/map base address
    struct memslab  *prev; // pointer to previous in list
    struct memslab  *next; // pointer to next in list
    size_t           pool; // allocation pool
    size_t           nblk; // number of total blocks
    size_t           bsz;  // block size in bytes
    uintptr_t        info; // allocation pointer + type
    void            *tab[VLA]; //
};
#define MEM_SLAB_TAB_ITEMS                                              \
    ((PAGESIZE - sizeof(struct memslab)) / sizeof(void *))

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

