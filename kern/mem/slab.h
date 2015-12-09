#ifndef __KERN_MEM_SLAB_H__
#define __KERN_MEM_SLAB_H__

#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <kern/mem/mem.h>

struct slabhdr {
    unsigned long   info;
#if (PTRBITS <= 32)
    uint32_t        link;
#else
    struct slabhdr *prev;
    struct slabhdr *next;
#endif
};

#define slabnum(ptr, zone)                                              \
    (((uintptr_t)(ptr) - ((zone)->base)) >> SLABMINLOG2)
#define slabgetadr(hdr, zone)                                           \
    ((void *)((zone)->base + ((uintptr_t)slabhdrnum(hdr, zone) << SLABMINLOG2)))
#define slabhdrnum(hdr, zone)                                           \
    (!(hdr) ? 0 : (uintptr_t)((hdr) - (struct slabhdr *)(zone)->hdrtab))
#define slabgethdr(ptr, zone)                                           \
    (!(ptr) ? NULL : (struct slabhdr *)((zone)->hdrtab) + slabnum(ptr, zone))

#define slabclrinfo(hp)                                                 \
    ((hp)->info = 0UL)
#define slabclrbkt(hp)                                                  \
    ((hp)->info &= MEMFLGBITS)
#define slabsetbkt(hp, bkt)                                             \
    (slabclrbkt(hp), (hp)->info |= ((bkt) << MEMNFLGBIT))
#define slabgetbkt(hp)                                                  \
    ((hp)->info >> MEMNFLGBIT)
#define slabisfree(hp)                                                  \
    (((hp)->info) & MEMFREE)
#define slabsetfree(hp)                                                 \
    ((hp)->info |= MEMFREE)
#define slabclrfree(hp)                                                 \
    ((hp)->info &= ~MEMFREE)
#define slabsetflg(hp, flg)                                             \
    ((hp)->info |= (flg))
#define slabclrflg(hp)                                                  \
    ((hp)->info &= ~MEMFLGBITS)

#define SLABMIN     (1UL << SLABMINLOG2)
#define SLABMINLOG2 16 // don't make this less than 16

#if (PTRBITS <= 32)
#include <kern/mem/slab32.h>
#elif (PTRBITS <= 64)
#include <kern/mem/slab64.h>
#endif

void   slabinit(struct memzone *virtzone,
                unsigned long base, unsigned long nbphys);
void * slaballoc(struct memzone *zone, unsigned long nb, unsigned long flg);
void   slabfree(struct memzone *zone, void *ptr);

#endif /* __KERN_MEM_SLAB_H__ */

