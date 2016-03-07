#ifndef __KERN_MEM_SLAB_H__
#define __KERN_MEM_SLAB_H__

#include <stdint.h>
#include <limits.h>
#include <zero/param.h>
#include <kern/types.h>
#include <kern/mem/mem.h>
#include <kern/mem/pool.h>
#if 0
#if (!MEMNEWSLAB)
#if (PTRBITS <= 32) 
#include <kern/mem/slab32.h>
#elif (PTRBITS <= 64)
#include <kern/mem/slab64.h>
#endif
#endif
#endif

#if !(MEMNEWSLAB)
#define memslabclrinfo(sp)                                              \
    ((sp)->info = 0UL)
#define memslabclrbkt(sp)                                               \
    ((sp)->info &= MEMFLGBITS)
#define memslabsetbkt(sp, bkt)                                          \
    (memslabclrbkt(sp), (sp)->info |= ((bkt) << MEMNFLGBIT))
#define memslabgetbkt(sp)                                               \
    ((sp)->info >> MEMNFLGBIT)
#define memslabisfree(sp)                                               \
    (((sp)->info) & MEMFREE)
#define memslabsetfree(sp)                                              \
    ((sp)->info |= MEMFREE)
#define memslabclrfree(sp)                                              \
    ((sp)->info &= ~MEMFREE)
#define memslabsetflg(sp, flg)                                          \
    ((sp)->info |= (flg))
#define memslabclrflg(sp)                                               \
    ((sp)->info &= ~MEMFLGBITS)
#endif

#if (MEMNEWSLAB)
#define __STRUCT_MEMSLAB_PAD                                            \
    (CLSIZE - 3 * PTRSIZE - WORDSIZE)
struct memslab {
    void           *adr;
    m_ureg_t        info;
    struct memslab *prev;
    struct memslab *next;
#if (__STRUCT_MEMSLAB_PAD)
    uint8_t         _pad[__STRUCT_MEMSLAB_PAD];
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

#define memgetblknum(ptr, pool)                                         \
    (((uintptr_t)(ptr) - (pool)->base) >> MEMSLABSHIFT)
#define memgetadr(hdr, pool)                                            \
    ((void *)((uint8_t *)(pool)->base                                   \
              + (memgetblknum(hdr, pool) << MEMSLABSHIFT)))
#define memgetslab(ptr, pool)                                           \
    ((struct memslab *)(pool)->hdrtab + memgetblknum(ptr, pool))
#define memgetmag(ptr, pool)                                            \
    ((struct memmag *)(pool)->hdrtab + memgetblknum(ptr, pool))

void * slaballoc(struct mempool *pool, unsigned long nb, unsigned long flg);
void   slabfree(struct mempool *pool, void *ptr);

#endif /* __KERN_MEM_SLAB_H__ */

