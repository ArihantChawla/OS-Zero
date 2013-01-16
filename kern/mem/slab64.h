#ifndef __MEM_SLAB64_H__
#define __MEM_SLAB64_H__

#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/trix.h>
#include <kern/task.h>

#define slablk(bkt)   mtxlk(&_physlktab[bkt], MEMPID);
#define slabunlk(bkt) mtxunlk(&_physlktab[bkt], MEMPID);

#define SLAB_CONST_SIZE_TRICK 1
#if (SLAB_CONST_SIZE_TRICK)
#define slabbkt(sz) slabfastbkt(sz)
#else
#define slabbkt(sz) slabcalcbkt(sz)
#endif

/* slab header and macros for manipulation */
#define SLABFREE    0x01L
#define SLABWIRE    0x02L
#define SLABZERO    0x04L
#define SLABFLGBITS 0x07L
#define NSLABFLGBIT 3

#define slabgetprev(hp, tab)                                            \
    (((hp)->link & 0x00000000ffffffffL)                                 \
     ? ((tab) + ((hp)->link & 0xffffffffL))                             \
     : NULL)
#define slabgetnext(hp, tab)                                            \
    (((hp)->link & 0xffffffff00000000L)                                 \
     ? ((tab) + (((hp)->link & 0xffffffff00000000L) >> 32))             \
     : NULL)
#define slabclrprev(hp)                                                 \
    ((hp)->link &= 0xffffffff00000000L)
#define slabclrnext(hp)                                                 \
    ((hp)->link &= 0x00000000ffffffffL)
#define slabclrlink(hp)                                                 \
    ((hp)->link = 0L)
#define slabsetprev(hp, hdr, tab)                                       \
    ((hp)->prev = (hdr))
#define slabsetnext(hp, hdr, tab)                                       \
    ((hp)->next = (hdr))

struct slabhdr {
    long nfo;                   // size shift count + free-bit
    struct slabhdr *prev;
    struct slabhdr *next;
} PACK();

#endif /* __MEM_SLAB64_H__ */

