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
 *   - allocated from thread-local pools, multiples of CLSIZE (cacheline)
 * - runs are PAGESIZE..MEM_RUN_MAX_PAGES * PAGESIZE (32 * PAGESIZE) bytes
 *   - allocated from thread-local pools, multiples of CLSIZE
 * - middle-size blocks are multiples of PAGESIZE << MEM_MID_PAGE_SHIFT
 *   (4 * PAGE_SIZE) from 64 * PAGESIZE up to 256 * PAGESIZE bytes
 *   - allocated from global pools, multiples of PAGESIZE >> MEM_MID_PAGE_SHIFT
 * - big blocks are allocated individually
 * - blocks are prefixed with allocation info structure struct memblk
 *   - other allocations have page-entries in g_mem.hashtab
 */

/* allocation types stored together with slab address */
#define MEM_BLK_MASK       (~(MEM_MIN_SIZE - 1))
#define MEM_BLK_SLAB       1 // small [TLS-buffered] allocations
#define MEM_RUN_SLAB       2 // [TLS-buffered] page-run allocations
#define MEM_MID_SLAB       3 // [global] mid-size allocations
#define MEM_BIG_SLAB       4 // [global] big-size allocations
#define MEM_TLS_SLAB_BIT   0x08
#define MEM_TYPE_BITS      4 // # of bits used for slab type
#define MEM_TYPE_MASK      ((1U << MEM_SLAB_TYPE_BITS) - 1)
#define MEM_INFO_BITS      PAGESIZELOG2 // info, minimum 12
/* small-size allocation blocks */
#define MEM_BLK_BITS       (MEM_INFO_BITS - MEM_TYPE_BITS) // ID bits
#define MEM_BLK_IDS        (1L << MEM_BLK_BITS)
#define MEM_BLK_TAB_SIZE                                                \
    rounduppow2(2 * MEM_BLK_MAX_IDS * sizeof(uintptr_t), PAGESIZE)
#define MEM_BLK_PAGE_BITS  (PTRBITS - MEM_INFO_BITS)
#define MEM_BLK_MAX_SIZE   (PAGESIZE / 2)
#define MEM_BLK_POOLS      PAGESIZELOG2
#define MEM_BLK_SLAB_SIZE  (4 * PAGESIZE)
#define MEM_BLK_MAX_IDS    (min(MEM_BLK_IDS, MEM_BLK_SLAB_SIZE / MEM_MIN_SIZE))
/* multiples-of-page run allocations */
#define MEM_RUN_MAX_PAGES  32
#define MEM_RUN_POOLS      MEM_RUN_MAX_PAGES
#define MEM_RUN_MAX_SIZE   (MEM_RUN_MAX_PAGES * PAGESIZE)
#define MEM_RUN_SLAB_SIZE  (2 * MEM_RUN_MAX_SIZE)
/* mid-size allocations */
#define MEM_MID_MIN_PAGES  (1L << MEM_MID_MIN_SHIFT)
#define MEM_MID_MIN_SHIFT  6 // minimum of 64 pages
#define MEM_MID_MAX_PAGES  512
#define MEM_MID_POOLS                                                   \
    ((MEM_MID_MAX_PAGES - MEM_MID_MIN_PAGES) >> MEM_MID_PAGE_SHIFT)
#define MEM_MID_MIN_SIZE   (PAGESIZE * MEM_MID_MIN_PAGES)
#define MEM_MID_MAX_SIZE   (MEM_MID_MAX_PAGES * PAGESIZE)
#define MEM_MID_PAGE_SHIFT 2
#define MEM_MID_POOLS      ((MEM_MID_MAX_PAGES - MEM_MID_MIN_PAGES) \
                            >> MEM_MID_PAGE_SHIFT)
#define MEM_MID_SLAB_PAGES (2 * MEM_MID_MAX_PAGES)
#define MEM_MID_SLAB_SIZE  (MEM_MID_SLAB_PAGES * PAGESIZE)
/* big allocations */
#define MEM_BIG_SLAB_SHIFT 23

/* slab book-keeping info */
#define MEM_PAGE_MASK      (~(((uintptr_t)1 << PAGESIZELOG2) - 1))
#define memblkpage(adr)    ((uintptr_t)(ptr) & MEM_PAGE_MASK)
#define mempagenum(adr)    ((uintptr_t)(ptr) >> PAGESIZELOG2)

/* per-pool # of allocation blocks */
#define memnumblk(pool) (MEM_BLK_SLAB_SIZE >> (3 + MEM_ALIGN_SHIFT + (pool)))
#define memnumrun(pool) (MEM_RUN_SLAB_SIZE / ((pool + 1) * PAGESIZE))
#define memnummid(pool) (MEM_MID_SLAB_PAGES / (MEM_MID_MIN_PAGES + (pool) * MEM_MID_UNIT_PAGES))
#define memnumbig(pool) (min(4, MEM_BIG_SLAB_SHIFT - (pool)))

/* [per-pool] allocation sizes */

#define memblksize(pool) ((size_t)1 << (MEM_ALIGN_SHIFT + (pool)))
#define memrunsize(pool) ((size_t)PAGESIZE * ((pool) + 1))
#define memmidsize(pool)                                                \
    (MEM_MID_MIN_SIZE + (pool) * (PAGESIZE << MEM_MID_PAGE_SHIFT))
#define membigsize(sz)   rounduppow2(sz, PAGESIZE)

/* allocation book-keeping */
/* in-slab allocation # */
#define memptrid(slab, ptr)  (((uint8_t *)(ptr) - (slab)->base) / (slab->bsz))
/* in-slab allocation address for block # */
#define memgetptr(slab, num) ((mem)->base + (num) * (slab)->bsz)

/* memory slab with buffer-stack */
#define MEM_SLAB_HDR_SIZE           (2 * PAGESIZE)
#define memblkid(slab, ptr, type, nblk)                                 \
    ((uintptr_t)ptr

#define memmkinfo(pool, type, nblk) (((type) << 24) | ((pool) << 16) | (nblk))
#define memgetnblk(slab)            ((slab)->info & 0xffff)
#define memgetpool(slab)            (((slab)->info >> 16) & 0xff)
#define memgettype(slab)            (((slab)->info >> 24) & 0xff)
#define memistls(type)              ((type) & MEM_TLS_SLAB_BIT)
struct memslab {
    struct memslab  *prev; // pointer to previous in list
    struct memslab  *next; // pointer to next in list
    uintptr_t        val; // # of slabs in pool or [TLS] pointer
    size_t           info; // allocation pool + type + # of blocks
    m_atomic_t       ndx;  // current index into allocation table
    void           **stk; // pointer stack + aligned pointers
    //    size_t           nblk; // number of total blocks
    uint8_t         *base; // slab/map base address
    size_t           bsz;  // block size in bytes
    void            *tab[VLA]; // embedded pointer tabs where present
};
#define MEM_SLAB_TAB_ITEMS                                              \
    ((MEM_SLAB_HDR_SIZE - offsetof(struct memslab, tab)) / sizeof(uintptr_t))

#if defined(ZEROMEMTKTLK)
struct mempool {
#if 0
    struct memslab    *slabtab[ZEROTKTBKTITEMS];
    struct zerotktbkt lkbkt;
#endif
    struct lfq        lfq;
};
#endif

#define MEM_TLS_SIZE PAGESIZE
struct memtls {
    struct memslab *blk[MEM_BLK_POOLS]; // tail is head->prev
    struct memslab *run[MEM_RUN_POOLS]; // tail is head->prev
};

struct memglob {
#if defined(ZEROMEMTKTLK)
    struct mempool mid[MEM_MID_POOLS];
#else
    struct lfq     midq[MEM_MID_POOLS];
#endif
};

struct membuf {
    struct memslab *hdrq;
    void           *tabq;
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
    m_atomic_t       lk;
    uintptr_t        flg;
    uintptr_t        nbused;
    uintptr_t        nbres;
    uintptr_t        nbfree;
};

static __inline__ void *
mempopblk(struct memslab *slab, struct memslab **headret)
{
    size_t  ndx = m_fetchadd(&slab->ndx, 1);
    //    intptr_t    n = slab->nblk;
    size_t  nblk = memgetnblk(slab);
    void   *ptr = NULL;

    if (ndx < nblk - 1) {
        ptr = slab->tab[ndx];
    } else if (ndx == nblk - 1) {
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
    //    intptr_t   n = slab->nblk;
    intptr_t   nblk = memgetnblk(slab);

    if (ndx > 0) {
        ndx--;
        slab->tab[ndx] = ptr;
        if (ndx == nblk - 1) {
            *tailret = slab;
        }
    } else {
        m_atominc(&slab->ndx);
    }

    return;
}

#endif /* __ZERO_MEM_H__ */

