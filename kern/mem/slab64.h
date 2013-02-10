#ifndef __MEM_SLAB64_H__
#define __MEM_SLAB64_H__

#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/trix.h>
#include <kern/task.h>
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

#if 0
#define SLABNLVLBIT  16
#define SLABNLVL0BIT (PTRBITS - 3 * SLABNLVLBIT)

static __inline__ void *
slabgethdr(void *adr, void *tab)
{
    void *ptr = tab;
    unsigned long mask = (1UL << NLVLBIT) - 1;
#if (SLABNLVL0BIT)
    unsigned long k0 = ((uintptr_t)adr >> (3 * SLABNLVLBIT))
        & (1UL << (SLABNLVL0BIT - 1));
#endif
    unsigned long k1 = ((uintptr_t)adr >> (2 * SLABNLVLBIT)) & mask;
    unsigned long k2 = ((uintptr_t)adr >> SLABLVLBIT) & mask;
    unsigned long k3 = (uintptr_t)adr & mask;
    
#if (SLABNLVL0BIT)
    ptr = (void **)ptr[k0];
    if (!ptr) {
        
        return ptr;
    }
#endif
    ptr = (void **)ptr[k1];
    if (!ptr) {

        return ptr;
    }
    ptr = (void **)ptr[k2];
    if (!ptr) {

        return ptr;
    }
    ptr = (void **)ptr[k3];

    return ptr;
}
#endif

#endif /* __MEM_SLAB64_H__ */

