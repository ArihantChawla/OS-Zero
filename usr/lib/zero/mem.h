/*
 * dynamic memory management
 * -------------------------
 * - blocks are 1..PAGESIZE / 2 bytes in size
 *   - allocated from thread-local pools
 * - runs are PAGESIZE..MEM_MAX_RUN_SIZE * PAGESIZE bytes in size
 *   - allocated from thread-local pools
 * - big blocks are allocated from global pools
 * - blocks are prefixed with allocation info structure struct memblk;
 *   - other allocations have page-entries in g_mem.hashtab
 */

#define MEM_SMALL_SLAB_SIZE (8 * PAGESIZELOG2)
#define MEM_MIN_SMALL_SLOT  6
#define MEM_MAX_SMALL_SLOT  (PAGESIZELOG2 - 1)
#define MEM_MAX_SMALL_SIZE  (PAGESIZE / 2)
#define MEM_MAX_RUN_PAGE    32
#define MEM_MIN_RUN_SLOT    (PAGESIZELOG2)
#define MEM_MAX_SMALL_SLOT  (PAGESIZELOG2 + MEM_MAX_RUN_PAGES - 1)
#define MEM_MIN_RUN_SIZE    PAGESIZE
#define MEM_MAX_RUN_SIZE    (MEM_MAX_RUN_PAGES * PAGESIZE)
#define MEM_MIN_BIG_SLOT    (PAGESIZELOG2 + 6)

struct memslab {
    struct memslab  *prev;
    struct memslab  *next;
    size_t           bkt;
    size_t           nblk;
    size_t           ndx;
    void           **stk;
};

struct memarn {
    struct memtls  *tls;
    struct memslab *queue;
};

#define MEM_BLK_ID_BITS     PAGESIZELOG2
#define MEM_BLK_BKT_BITS    8
#define memgetblkid(nfo)                                                \
    ((nfo) & ((1L << PAGESIZELOG2) - 1))
#define memgetblkbkt(nfo)                                               \
    (((nfo) >> MEM_BLK_ID_BITS) & ((1L << MEM_BLK_ID_BITS) - 1))
#define memlkblk(blk)                                                   \
    (!m_cmpsetbit((m_atomic_t *)&blk->ptr, MEM_LK_BIT_POS))
#define memunlkblk(blk)                                                 \
    (m_clrbit((m_atomic_t *)&blk->ptr, MEM_LK_BIT_POS)))
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

struct mem {
    uintptr_t          flg;
    uintptr_t          nbused;
    uintptr_t          nbres;
    uintptr_t          nbfree;
    struct memhashtab *bigtab[PTRBITS];
};

