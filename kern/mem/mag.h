#ifndef __KERN_MEM_MAG_H__
#define __KERN_MEM_MAG_H__

#include <stdint.h>
#include <kern/mem/mem.h>

#define mempop(mp)       ((mp)->ptab[((mp)->ndx)++])
#define mempush(mp, ptr) ((mp)->ptab[--((mp)->ndx)] = (ptr))
#define memmagempty(mp)  ((mp)->ndx == (mp)->n)
#define memmagfull(mp)   (!(mp)->ndx)
struct memmag {
    uintptr_t      base;
    volatile long  n;
    volatile long  ndx;
    volatile long  bkt;
    struct memmag *prev;
    struct memmag *next;
#if defined(MEMPARANOIA)
#if (MEMSLABMINLOG2 - MEMMINLOG2 < (LONGSIZELOG2 + 3))
    unsigned long  bmap;
#else
    uint8_t        bmap[1UL << (MEMSLABMINLOG2 - MEMMINLOG2 - 3)];
#endif
#endif /* defined(MEMPARANOIA) */
    void           *ptab[1UL << (MEMSLABMINLOG2 - MEMMINLOG2)];
};

#define memgetmag(ptr, pool)                                            \
    (!(ptr)                                                             \
     ? NULL                                                             \
     : (struct memmag *)(pool)->blktab + memgetblknum(ptr, pool))

#endif /* __KERN_MEM_MAG_H__ */

