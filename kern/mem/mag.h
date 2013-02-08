#ifndef __MEM_MAG_H__
#define __MEM_MAG_H__

#include <zero/param.h>
#include <zero/mtx.h>

#include <kern/task.h>
#include <kern/mem/slab.h>

#define MAGMIN     (1UL << MAGMINLOG2)
#define MAGMINLOG2 PAGESIZELOG2

#define maglk(bkt)       mtxlk(&_freelktab[bkt], MEMPID)
#define magunlk(pq)      mtxunlk(&_freelktab[bkt], MEMPID)

#define magpop(mp)       ((mp)->ptab[((mp)->ndx)++])
#define magpush(mp, ptr) ((mp)->ptab[--((mp)->ndx)] = (ptr))
#define magfull(mp)      ((mp)->ndx == (mp)->n)
#define magempty(mp)     (!(mp)->ndx)
struct maghdr {
    long           n;
    long           ndx;
    struct maghdr *prev;
    struct maghdr *next;
    void          *ptab[1U << (SLABMINLOG2 - MAGMINLOG2)];
};

#define maghdrnum(ptr)                                                  \
    ((uintptr_t)ptr >> SLABMINLOG2)
#define magslabadr(ptr)                                                 \
    ((void *)((uintptr_t)(ptr) & ~(SLABMIN - 1)))

#endif /* __MEM_MAG_H__ */

