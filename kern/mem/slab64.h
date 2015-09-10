#ifndef __KERN_MEM_SLAB64_H__
#define __KERN_MEM_SLAB64_H__

#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/trix.h>
#include <kern/proc/task.h>
#include <kern/mem/mem.h>

#define slabgetprev(hp, zone)                                           \
    ((hp)->prev)
#define slabgetnext(hp, zone)                                           \
    ((hp)->next)
#define slabclrprev(hp)                                                 \
    ((hp)->prev = NULL)
#define slabclrnext(hp)                                                 \
    ((hp)->next = NULL)
#define slabclrlink(hp)                                                 \
    ((hp)->prev = (hp)->next = NULL)
#define slabsetprev(hp, hdr, zone)                                      \
    ((hp)->prev = (hdr))
#define slabsetnext(hp, hdr, zone)                                      \
    ((hp)->next = (hdr))

#endif /* __KERN_MEM_SLAB64_H__ */

