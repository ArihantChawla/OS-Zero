#ifndef __KERN_MEM_SLAB_H__
#define __KERN_MEM_SLAB_H__

#include <stdint.h>
#include <limits.h>
#include <zero/param.h>
#include <kern/types.h>
#include <kern/mem/mem.h>
#include <kern/mem/pool.h>
#if (!MEMNEWSLAB)
#if (PTRBITS <= 32)
#include <kern/mem/slab32.h>
#elif (PTRBITS <= 64)
#include <kern/mem/slab64.h>
#endif
#endif

#if (!MEMNEWSLAB)
#define MEMSLABMIN     (1UL << MEMSLABMINLOG2)
#define MEMSLABMINLOG2 16 // don't make this less than 16 for now :)
#endif

#if (MEMNEWSLAB)
#define memblkclrinfo(bp)                                               \
    ((bp)->info = ((m_ureg_t)0))
#define memblkclrbkt(bp)                                                \
    ((hp)->info &= ~MEMBKTMASK)
#define memblksetbkt(hp, bkt)                                           \
    (memblkclrbkt(hp), (hp)->info |= (bkt))
#define memblkgetbkt(hp)                                                \
    ((hp)->info & MEMBKTMASK)
#define memblkisfree(hp)                                                \
    (!((hp)->info) & MEMALLOCBIT)
#define memblksetfree(hp)                                               \
    ((hp)->info |= MEMFREE)
#define memblkclrfree(hp)                                               \
    ((hp)->info &= ~MEMFREE)
#define memblksetflg(hp, flg)                                           \
    ((hp)->info |= (flg))
#define memblkclrflg(hp)                                                \
    ((hp)->info &= ~MEMFLGBITS)
#else
#define memslabclrinfo(hp)                                              \
    ((hp)->info = 0UL)
#define memslabclrbkt(hp)                                               \
    ((hp)->info &= MEMFLGBITS)
#define memslabsetbkt(hp, bkt)                                          \
    (memslabclrbkt(hp), (hp)->info |= ((bkt) << MEMNFLGBIT))
#define memslabgetbkt(hp)                                               \
    ((hp)->info >> MEMNFLGBIT)
#define memslabisfree(hp)                                               \
    (((hp)->info) & MEMFREE)
#define memslabsetfree(hp)                                              \
    ((hp)->info |= MEMFREE)
#define memslabclrfree(hp)                                              \
    ((hp)->info &= ~MEMFREE)
#define memslabsetflg(hp, flg)                                          \
    ((hp)->info |= (flg))
#define memslabclrflg(hp)                                               \
    ((hp)->info &= ~MEMFLGBITS)
#endif

#if (MEMNEWSLAB)
#define __STRUCT_MEMQUEUE_SIZE                                          \
    (rounduppow2(offsetof(struct memblk, _pad), CLSIZE))
#define __STRUCT_MEMQUEUE_PAD                                           \
    (__STRUCT_MEMBLK_SIZE - offsetof(struct memblk, _pad))
struct memqueue {
    void          *adr;
    m_ureg_t       info;
    struct memblk *prev;
    struct memblk *next;
#if (__STRUCT_MEMBLK_PAD)
    uint8_t        _pad[__STRUCT_MEMBLK_PAD];
#endif
};
#else
struct memslab {
#if (PTRBITS <= 32)
    uint32_t        info;
    uint32_t        link;
#else
    unsigned long   info;
    struct slabhdr *prev;
    struct slabhdr *next;
#endif
};
#endif

#if (MEMNEWSLAB)
#define memgetblknum(ptr, pool)                                         \
    (((uintptr_t)(ptr) - (pool)->base) >> MEMMINLOG2)
#define memgetadr(hdr, pool)                                            \
    ((void *)((pool)->base + (memgetblknum(hdr, pool) << MEMMINLOG2)))
#define memgethdr(ptr, pool)                                            \
    ((struct memslab *)((uint8_t *)(pool)->blktab + memgetblknum(ptr, pool))
#else
#define memgetblknum(ptr, pool)                                         \
    (((uintptr_t)(ptr) - (pool)->base) >> MEMMINLOG2)
#define memgetadr(hdr, pool)                                            \
    ((void *)((pool)->base + (memgetblknum(hdr, pool) << MEMMINLOG2)))
#define memgethdr(ptr, pool)                                            \
    ((struct memslab *)((pool)->blktab) + memgetblknum(ptr, pool))
#endif

void * slaballoc(struct mempool *pool, unsigned long nb, unsigned long flg);
void   slabfree(struct mempool *pool, void *ptr);

#endif /* __KERN_MEM_SLAB_H__ */

