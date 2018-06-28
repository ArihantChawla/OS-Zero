#ifndef __ZERO_MEM_H__
#define __ZERO_MEM_H__

#include <zero/param.h>
#include <zero/bits/mem.h>

/*
 * allocation anatomy
 * ------------------
 *
 */

/*
 * dynamic memory management
 * -------------------------
 * - minimum allocation size is a cacheline (may be wasteful; see <bits/mem.h>)
 * - blocks are 1..PAGESIZE / 2 bytes in size
 *   - allocated from thread-local pools
 * - runs are PAGESIZE..MEM_MAX_RUN_SIZE * PAGESIZE bytes in size
 *   - allocated from thread-local pools
 * - middle-size blocks are multiples of 8 * PAGESIZE up to 512 * PAGESIZE
 *   - allocated from global pools
 * - big blocks are allocated individually
 * - blocks are prefixed with allocation info structure struct memblk;
 *   - other allocations have page-entries in g_mem.hashtab
 */
#define MEM_SMALL_SLAB_SIZE (8 * PAGESIZE)
#define MEM_SMALL_POOLS     (PAGESIZELOG2 - CLSIZELOG2)
#define MEM_MAX_SMALL_POOL  (MEM_SMALL_POOLS - 1)
#define MEM_MAX_SMALL_SIZE  PAGESIZE
#define MEM_RUN_POOLS       (MEM_MAX_RUN_PAGES)
#define MEM_MAX_RUN_SHIFT   6
#define MEM_MAX_RUN_PAGES   (1 << MEM_MAX_RUN_SHIFT)
#define MEM_MAX_RUN_POOL    (MEM_MAX_RUN_PAGES - 1)
#define MEM_MIN_RUN_SIZE    PAGESIZE
#define MEM_MAX_RUN_SIZE    (MEM_MAX_RUN_PAGES * PAGESIZE)
#define MEM_MID_POOLS       32
#define MEM_MID_UNIT_SHIFT  3
#define MEM_MID_UNIT_PAGES  (1 << MEM_MID_UNIT_SHIFT)
#define MEM_MAX_MID_PAGES   (MEM_MID_UNIT_PAGES * MEM_MID_POOLS)
#define MEM_MIN_MID_SIZE    (MEM_MID_UNIT_PAGES * PAGESIZE)
#define MEM_MAX_MID_SIZE    (MEM_MAX_MID_PAGES * PAGESIZE)
#define MEM_BIG_POOLS       PTRBITS

/* memory slab with buffer-stack */
struct memslab {
    struct memslab  *prev;      // pointer to previous in list
    struct memslab  *next;      // pointer to next in list
    size_t           pool;      // allocation pool
    size_t           nblk;      // number of total blocks
    m_atomic_t       ndx;       // current index into allocation stack
    void           **stk;       // allocation stack
};

struct memtls {
    struct memslab *smalltab[MEM_SMALL_POOLS];
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
#define memlkblk(blk)                                                   \
    (m_clrbit((m_atomic_t *)&((void *)(blk)[-1]), MEM_ADR_LK_BIT_POS))
#define memgetinfo(blk)                                                 \
    (*((uintptr_t *)(blk)[-2]))
struct memblk {
    uintptr_t  info;
    void      *ptr;
};

#define THASH_VAL_NONE (~(uintptr_t)0)
#include <zero/thash.h>

/* global allocator data */
#define MEM_HASH_ITEMS 16384
struct mem {
    struct thashtab *hash[MEM_HASH_ITEMS];
    struct memslab  *midtab[MEM_MID_POOLS];
    struct memslab  *bigtab[MEM_BIG_POOLS];
    uintptr_t        flg;
    uintptr_t        nbused;
    uintptr_t        nbres;
    uintptr_t        nbfree;
};

#endif /* __ZERO_MEM_H__ */

