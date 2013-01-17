#ifndef __MEM_MAG_H__
#define __MEM_MAG_H__

#include <zero/param.h>
#include <zero/mtx.h>

#include <kern/task.h>
#include <kern/mem/slab.h>

#define maglk(bkt)  mtxlk(&_freelktab[bkt], MEMPID)
#define magunlk(pq) mtxunlk(&_freelktab[bkt], MEMPID)

#define magpop(mp)       ((mp)->ptab[((mp)->ndx)++])
#define magpush(mp, ptr) ((mp)->ptab[--((mp)->ndx)] = (ptr))
#define magfull(mp)      ((mp)->ndx == (mp)->n)
#define magempty(mp)     (!(mp)->ndx)
struct maghdr {
    unsigned long  flg;
    unsigned long  n;
    unsigned long  ndx;
#if (PTRBITS == 32)
    void          *ptab[1U << (PTRBITS - SLABMINLOG2 - PAGESIZELOG2)];
#endif
};

#endif /* __MEM_MAG_H__ */

