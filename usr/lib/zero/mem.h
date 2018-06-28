#ifndef __ZERO_MEM_H__
#define __ZERO_MEM_H__

#include <zero/param.h>

/*
 * dynamic memory management
 * -------------------------
 * - minimum allocation size is a cacheline (may be wasteful; see <bits/mem.h>)
 * - blocks are 1..PAGESIZE / 2 bytes in size
 *   - allocated from thread-local pools
 * - runs are PAGESIZE..MEM_MAX_RUN_SIZE * PAGESIZE bytes in size
 *   - allocated from thread-local pools
 * - big blocks are allocated from global pools
 * - blocks are prefixed with allocation info structure struct memblk;
 *   - other allocations have page-entries in g_mem.hashtab
 */
#define MEM_SMALL_SLAB_SIZE (8 * PAGESIZE)
#define MEM_MAX_SMALL_SLOT  (PAGESIZELOG2 - CLSIZELOG2)
#define MEM_MAX_SMALL_SIZE  PAGESIZE
#define MEM_MAX_RUN_SHIFT   6
#define MEM_MAX_RUN_PAGES   (1 << MEM_MAX_RUN_SHIFT)
#define MEM_MAX_RUN_SLOT    (MEM_MAX_RUN_PAGES - 1)
#define MEM_MIN_RUN_SIZE    PAGESIZE
#define MEM_MAX_RUN_SIZE    (MEM_MAX_RUN_PAGES * PAGESIZE)

/* memory slab with buffer-stack */
struct memslab {
    struct memslab  *prev;      // pointer to previous in list
    struct memslab  *next;      // pointer to next in list
    size_t           bkt;       // allocation bucket
    size_t           nblk;      // number of total blocks
    size_t           ndx;       // current index into allocation stack
    void           **stk;       // allocation stack
};

/* per-thread memory arena */
struct memarn {
    struct memtls  *tls;
    struct memslab *queue;
};

#define MEM_BLK_ID_BITS     16
#define MEM_BLK_BKT_BITS    8
#define memgetblkid(nfo)                                                \
    ((nfo) & ((1L << PAGESIZELOG2) - 1))
#define memgetblkbkt(nfo)                                               \
    (((nfo) >> MEM_BLK_ID_BITS) & ((1L << MEM_BLK_ID_BITS) - 1))
#define memlkblk(blk)                                                   \
    (!m_cmpsetbit((m_atomic_t *)&((void *)(blk)[-1]), MEM_LK_BIT_POS))
#define memlkblk(blk)                                                   \
    (m_clrbit((m_atomic_t *)&((void *)(blk)[-1]), MEM_LK_BIT_POS))
#define memgetinfo(blk)                                                 \
    (*((uintptr_t *)(blk)[-2]))
struct memblk {
    uintptr_t  info;
    void      *ptr;
};

/* allocation hash entry */
struct memhash {
    void   *ptr;
    size_t  page;
};

/* allocation hash-table queue structure; cache-friendly */
struct memhashtab {
    size_t             n;
    struct memhashtab *next;
    struct memhash     queue[31];
};

/* global allocator data */
struct mem {
    uintptr_t          flg;
    uintptr_t          nbused;
    uintptr_t          nbres;
    uintptr_t          nbfree;
    struct memhashtab *bigtab[PTRBITS];
};

#endif /* __ZERO_MEM_H__ */

