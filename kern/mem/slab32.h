#ifndef __MEM_SLAB32_H__
#define __MEM_SLAB32_H__

#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/trix.h>
#include <kern/task.h>
#include <kern/mem/slab.h>

#define SLABNHDR     (1UL << (PTRBITS - SLABMINLOG2))
#define SLABHDRTABSZ (SLABNHDR * sizeof(struct slabhdr))
#define SLABHDRBASE  (VIRTBASE - SLABHDRTABSZ)

#define slabgetprev(hp, tab)                                            \
    (!(hp)                                                              \
     ? NULL                                                             \
     : (((hp)->link & 0x0000ffffL)                                      \
        ? ((struct slabhdr *)(tab) + ((hp)->link & 0x0000ffffL))        \
        : NULL))
#define slabgetnext(hp, tab)                                            \
    (!(hp)                                                              \
     ? NULL                                                             \
     : (((hp)->link & 0xffff0000L)                                      \
        ? ((struct slabhdr *)(tab) + (((hp)->link & 0xffff0000L) >> 16)) \
        : NULL))
#define slabclrprev(hp)                                                 \
    ((hp)->link &= 0xffff0000L)
#define slabclrnext(hp)                                                 \
    ((hp)->link &= 0x0000ffffL)
#define slabclrlink(hp)                                                 \
    ((hp)->link = 0L)
#define slabsetprev(hp, hdr, tab)                                       \
    (slabclrprev(hp), (hp)->link |= (hp) ? slabhdrnum(hdr, tab) : 0)
#define slabsetnext(hp, hdr, tab)                                       \
    (slabclrnext(hp), (hp)->link |= (hp) ? (slabhdrnum(hdr, tab) << 16) : 0)

void  slabinit(unsigned long base, unsigned long size);
void *slaballoc(struct slabhdr **zone, struct slabhdr *hdrtab,
                unsigned long nb, unsigned long flg);
void  slabfree(struct slabhdr **zone, struct slabhdr *hdrtab, void *ptr);

#endif /* __MEM_SLAB32_H__ */

