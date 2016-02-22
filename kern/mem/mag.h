#ifndef __KERN_MEM_MAG_H__
#define __KERN_MEM_MAG_H__

#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <kern/mem/mem.h>

#define mempop(mp)       ((mp)->ptab[((mp)->ndx)++])
#define mempush(mp, ptr) ((mp)->ptab[--((mp)->ndx)] = (ptr))
#define memmagempty(mp)  ((mp)->ndx == (mp)->n)
#define memmagfull(mp)   (!(mp)->ndx)

#define __STRUCT_MEMMAG_PTAB_SIZE                                       \
    ((1UL << (MEMSLABMINLOG2 - MEMMINLOG2)) * sizeof(void *))
#if defined(MEMPARANOIA)
#if (MEMSLABMINLOG2 - MEMMINLOG2 < (LONGSIZELOG2 + 3))
#define __STRUCT_MEMMAG_BMAP_SIZE sizeof(long)
#else
#define
#define __STRUCT_MEMMAG_BMAP_SIZE                                       \
    (1UL << (MEMSLABMINLOG2 - MEMMINLOG2 - 3))
#endif
#else
#define __STRUCT_MEMMAG_BMAP_SIZE 0
#endif /* defined(MEMPARANOIA) */
#define __STRUCT_MEMMAG_SIZE (rounduppow2(offsetof(struct memmag, _pad), \
                                          CLSIZE))
#define __STRUCT_MEMMAG_PAD                                             \
    (__STRUCT_MEMMAG_SIZE - offsetof(struct memmag, _pad))
struct memmag {
    uintptr_t      base;
    volatile long  bkt;
    struct memmag *prev;
    struct memmag *next;
#if defined(MEMPARANOIA) && (MEMSLABMINLOG2 - MEMMINLOG2 < (LONGSIZELOG2 + 3))
    unsigned long  bmap;
#endif
    volatile long  ndx;
    volatile long  n;
    void          *ptab[1UL << (MEMSLABMINLOG2 - MEMMINLOG2)];
#if defined(MEMPARANOIA) &&  !(MEMSLABMINLOG2 - MEMMINLOG2 < (LONGSIZELOG2 + 3))
    uint8_t        bmap[__STRUCT_MEMMAG_BMAP_SIZE];
#endif /* defined(MEMPARANOIA) */
    uint8_t        _pad[__STRUCT_MEMMAG_PAD];
};
    
#define memgetmag(ptr, pool)                                            \
    (((ptr)                                                             \
      ? ((struct memmag *)(pool)->blktab + memgetblknum(ptr, pool))     \
      : NULL))

#endif /* __KERN_MEM_MAG_H__ */

