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

#if (!MEMNEWSLAB)
#define memslabgetprev(sp, pool)                                        \
    (!(sp)                                                              \
     ? NULL                                                             \
     : (((sp)->link & 0x0000ffffL)                                      \
        ? ((struct memslab *)((pool)->tab)                              \
           + ((sp)->link & 0x0000ffffL))   \
        : NULL))
#define memslabgetnext(sp, pool)                                        \
    (!(sp)                                                              \
     ? NULL                                                             \
     : (((sp)->link & 0xffff0000L)                                      \
        ? ((struct memslab *)((pool)->tab)                              \
           + (((sp)->link & 0xffff0000L) >> 16))                        \
        : NULL))
#define memslabclrprev(sp)                                              \
    ((sp)->link &= 0xffff0000L)
#define memslabclrnext(sp)                                              \
    ((sp)->link &= 0x0000ffffL)
#define memslabclrlink(sp)                                              \
    ((sp)->link = 0L)
#define memslabsetprev(sp, hdr, pool)                                   \
    (memslabclrprev(sp), (sp)->link                                     \
     |= ((hdr)                                                          \
         ? memgetblknum(hdr, pool)                                      \
         : 0))
#define memslabsetnext(sp, hdr, pool)                                   \
    (memslabclrnext(sp), (sp)->link                                     \
     |= ((hdr)                                                          \
         ? (memgetblknum(hdr, pool) << 16)                              \
         : 0))
#endif

#endif /* __KERN_MEM_SLAB32_H__ */

