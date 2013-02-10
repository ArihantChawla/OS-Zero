#ifndef __MEM_MAG_H__
#define __MEM_MAG_H__

#include <zero/param.h>
#include <zero/mtx.h>

#include <kern/task.h>
#include <kern/mem/slab.h>

#define MAGMIN     (1UL << MAGMINLOG2)
#define MAGMINLOG2 PAGESIZELOG2

#define maglk(bkt)       mtxlk(&freelktab[bkt], MEMPID)
#define magunlk(pq)      mtxunlk(&freelktab[bkt], MEMPID)

#define magpop(mp)       ((mp)->ptab[((mp)->ndx)++])
#define magpush(mp, ptr) ((mp)->ptab[--((mp)->ndx)] = (ptr))
#define magfull(mp)      ((mp)->ndx == (mp)->n)
#define magempty(mp)     (!(mp)->ndx)
struct maghdr {
    long           n;
    long           ndx;
    long           bkt;
    struct maghdr *prev;
    struct maghdr *next;
    void          *ptab[1UL << (SLABMINLOG2 - MAGMINLOG2)];
};

#if (PTRBITS > 32)
#define magnum(ptr)                                                     \
    ((uintptr_t)(ptr) >> SLABMINLOG2)
#else
#define magnum(ptr)                                                     \
    ((uintptr_t)(ptr) >> SLABMINLOG2)
#endif
#define magslabadr(ptr)                                                 \
    ((void *)((uintptr_t)(ptr) & ~(SLABMIN - 1)))
#define maggethdr(ptr, tab)                                             \
    ((tab) + magnum(ptr))

#endif /* __MEM_MAG_H__ */

