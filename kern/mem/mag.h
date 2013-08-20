#ifndef __MEM_MAG_H__
#define __MEM_MAG_H__

#include <zero/param.h>
#include <zero/mtx.h>

//#include <kern/task.h>
#include <kern/mem/slab.h>

#define MAGMIN     (1UL << MAGMINLOG2)
#define MAGMINLOG2 PAGESIZELOG2

#define maglkq(tab, bkt)   mtxlk(&(tab)[bkt])
#define magunlkq(tab, bkt) mtxunlk(&(tab)[bkt])

#define magpop(mp)         ((mp)->ptab[((mp)->ndx)++])
#define magpush(mp, ptr)   ((mp)->ptab[--((mp)->ndx)] = (ptr))
#define magempty(mp)       ((mp)->ndx == (mp)->n)
#define magfull(mp)        (!(mp)->ndx)
struct maghdr {
#if (MAGLK)
    volatile long  lk;
#endif
#if (MAGBITMAP)
    uintptr_t      base;
#endif
    volatile long  n;
    volatile long  ndx;
    volatile long  bkt;
    struct maghdr *prev;
    struct maghdr *next;
#if (MAGBITMAP)
    uint8_t        bmap[1UL << (SLABMINLOG2 - MAGMINLOG2 - 3)];
#endif
    void          *ptab[1UL << (SLABMINLOG2 - MAGMINLOG2)];
};

#define magblknum(ptr, zone)                                            \
    (((uintptr_t)(ptr) - (zone)->base) >> SLABMINLOG2)
#if 0
#define magslabadr(ptr)                                                 \
    ((void *)((uintptr_t)(ptr) & ~(SLABMIN - 1)))
#endif
#define maggethdr(ptr, zone)                                            \
    (!(ptr) ? NULL : (struct maghdr *)((zone)->hdrtab) + magblknum(ptr, zone))

#endif /* __MEM_MAG_H__ */

