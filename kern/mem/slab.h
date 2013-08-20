#ifndef __MEM_SLAB_H__
#define __MEM_SLAB_H__

#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <kern/mem/mem.h>

struct slabhdr {
    unsigned long   nfo;
#if (NEWLK)
    unsigned long   bkt;
#endif
#if (PTRBITS <= 32)
    uint32_t        link;
#else
    struct slabhdr *prev;
    struct slabhdr *next;
#endif
} PACK();

#define slabnum(ptr, zone)                                              \
    (((uintptr_t)(ptr) - ((zone)->base)) >> SLABMINLOG2)
#define slabgetadr(hdr, zone)                                            \
    ((void *)((zone)->base + ((uintptr_t)slabhdrnum(hdr, zone) << SLABMINLOG2)))
#define slabhdrnum(hdr, zone)                                           \
    (!(hdr) ? 0 : (uintptr_t)((hdr) - (struct slabhdr *)(zone)->hdrtab))
#define slabgethdr(ptr, zone)                                      \
    (!(ptr) ? NULL : (struct slabhdr *)((zone)->hdrtab) + slabnum(ptr, zone))

#define slabclrnfo(hp)                                                  \
    ((hp)->nfo = 0L)
#define slabclrbkt(hp)                                                  \
    ((hp)->nfo &= MEMFLGBITS)
#define slabsetbkt(hp, bkt)                                             \
    (slabclrbkt(hp), (hp)->nfo |= ((bkt) << MEMNFLGBIT))
#define slabgetbkt(hp)                                                  \
    ((hp)->nfo >> MEMNFLGBIT)
#define slabisfree(hp)                                                  \
    (((hp)->nfo) & MEMFREE)
#define slabsetfree(hp)                                                 \
    ((hp)->nfo |= MEMFREE)
#define slabclrfree(hp)                                                 \
    ((hp)->nfo &= ~MEMFREE)
#define slabsetflg(hp, flg)                                             \
    ((hp)->nfo |= (flg))
#define slabclrflg(hp)                                                  \
    ((hp)->nfo &= ~MEMFLGBITS)

#define SLABMIN      (1UL << SLABMINLOG2)
#define SLABMINLOG2  16 // don't make this less than 16

#if defined(__i386__) || defined(__arm__) && !defined(__x86_64__) && !defined(__amd64__)
#include <kern/mem/slab32.h>
#elif defined(__x86_64__) || defined(__amd64__)
#include <kern/mem/slab64.h>
#endif

void   slabinit(struct memzone *virtzone,
                unsigned long base, unsigned long nbphys);
void * slaballoc(struct memzone *zone, unsigned long nb, unsigned long flg);
void   slabfree(struct memzone *zone, void *ptr);

#endif /* __MEM_SLAB_H__ */

