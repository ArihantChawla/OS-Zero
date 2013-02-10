#ifndef __MEM_SLAB64_H__
#define __MEM_SLAB64_H__

#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/trix.h>
#include <kern/task.h>
#include <kern/mem/mem.h>

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

#define slabgetadr(hdr, tab)                                            \
    (slab->base)
#define slabgethdr(ptr, tab)                                            \
    (!(ptr) ? NULL : (struct slabhdr *)(tab) + slabnum(ptr))
#define slabnum(ptr)                                                    \
    ((uintptr_t)(ptr) >> SLABMINLOG2)
#if 0
#define slabhdrnum(hdr, tab)                                            \
    (!(hdr) ? 0 : (uintptr_t)((hdr) - (struct slabhdr *)(tab)))
#endif

#endif /* __MEM_SLAB64_H__ */

