#ifndef __KERN_MEM_SLAB32_H__
#define __KERN_MEM_SLAB32_H__

#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/trix.h>
//#include <kern/proc/task.h>
#include <kern/mem/slab.h>

#define MEMNSLABHDR  (1UL << (PTRBITS - SLABMINLOG2))
#define MEMSLABTABSZ (MEMNSLABHDR * sizeof(struct memslab))
#define MEMSLABBASE  (VIRTBASE - MEMSLABTABSZ)

#define memslabgetprev(hp, pool)                                        \
    (!(hp)                                                              \
     ? NULL                                                             \
     : (((hp)->link & 0x0000ffffL)                                      \
        ? ((struct memslab *)((pool)->tab)                              \
           + ((hp)->link & 0x0000ffffL))   \
        : NULL))
#define memslabgetnext(hp, pool)                                        \
    (!(hp)                                                              \
     ? NULL                                                             \
     : (((hp)->link & 0xffff0000L)                                      \
        ? ((struct memslab *)((pool)->tab)                              \
           + (((hp)->link & 0xffff0000L) >> 16))                        \
        : NULL))
#define memslabclrprev(hp)                                              \
    ((hp)->link &= 0xffff0000L)
#define memslabclrnext(hp)                                              \
    ((hp)->link &= 0x0000ffffL)
#define memslabclrlink(hp)                                              \
    ((hp)->link = 0L)
#define memslabsetprev(hp, hdr, pool)                                   \
    (memslabclrprev(hp), (hp)->link                                     \
     |= ((hdr)                                                          \
         ? memgetblknum(hdr, pool)                                      \
         : 0))
#define memslabsetnext(hp, hdr, pool)                                   \
    (memslabclrnext(hp), (hp)->link                                     \
     |= ((hdr)                                                          \
         ? (memgetblknum(hdr, pool) << 16)                              \
         : 0))

#endif /* __KERN_MEM_SLAB32_H__ */

