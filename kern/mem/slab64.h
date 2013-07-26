#ifndef __MEM_SLAB64_H__
#define __MEM_SLAB64_H__

#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/trix.h>
#include <kern/proc/task.h>
#include <kern/mem/mem.h>

#define slabgetprev(hp, tab)                                            \
    ((hp)->prev)
#define slabgetnext(hp, tab)                                            \
    ((hp)->next)
#define slabclrprev(hp)                                                 \
    ((hp)->prev = NULL)
#define slabclrnext(hp)                                                 \
    ((hp)->next = NULL)
#define slabclrlink(hp)                                                 \
    ((hp)->prev = (hp)->next = NULL)
#define slabsetprev(hp, hdr, tab)                                       \
    ((hp)->prev = (hdr))
#define slabsetnext(hp, hdr, tab)                                       \
    ((hp)->next = (hdr))

#endif /* __MEM_SLAB64_H__ */

