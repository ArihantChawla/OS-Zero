#ifndef __MEM_SLAB_H__
#define __MEM_SLAB_H__

#include <kern/mem/slab.h>
#if defined(__x86_64__) || defined(__amd64__)
#include <kern/mem/slab64.h>
#elif defined(__i386__) || defined(__arm__)
#include <kern/mem/slab32.h>
#endif

#define slablk(bkt)   mtxlk(&virtlktab[bkt], MEMPID);
#define slabunlk(bkt) mtxunlk(&virtlktab[bkt], MEMPID);

/* slab header and macros for manipulation */
#define SLABFREE    0x01L
#define SLABWIRE    0x02L
#define SLABZERO    0x04L
#define SLABFLGBITS 0x07L
#define NSLABFLGBIT 3

#define slabclrnfo(hp)                                                  \
    ((hp)->nfo = 0L)
#define slabclrbkt(hp)                                                  \
    ((hp)->nfo &= SLABFLGBITS)
#define slabsetbkt(hp, bkt)                                             \
    (slabclrbkt(hp), (hp)->nfo |= ((bkt) << NSLABFLGBIT))
#define slabgetbkt(hp)                                                  \
    ((hp)->nfo >> NSLABFLGBIT)
#define slabisfree(hp)                                                  \
    (((hp)->nfo) & SLABFREE)
#define slabsetfree(hp)                                                 \
    ((hp)->nfo |= SLABFREE)
#define slabclrfree(hp)                                                 \
    ((hp)->nfo &= ~SLABFREE)
#define slabsetflg(hp, flg)                                             \
    ((hp)->nfo |= (flg))
#define slabclrflg(hp)                                                  \
    ((hp)->nfo &= ~SLABFLGBITS)
#if 0
#define slabsetprev(hp, hdr, tab)                                       \
    (slabclrprev(hp), (hp)->link |= slabhdrnum(hdr, tab))
#define slabsetnext(hp, hdr, tab)                                       \
    (slabclrnext(hp), (hp)->link |= (slabhdrnum(hdr, tab) << 16))
#endif

extern struct slabhdr *virtslabtab[];
extern struct slabhdr  virthdrtab[];

void  slabinit(struct slabhdr **zone, struct slabhdr *hdrtab,
               unsigned long base, unsigned long size);
void *slaballoc(struct slabhdr **zone, struct slabhdr *hdrtab,
                unsigned long nb, unsigned long flg);
void   slabfree(struct slabhdr **zone, struct slabhdr *hdrtab, void *ptr);

#define SLABMIN      (1UL << SLABMINLOG2)
#define SLABMINLOG2  16 // don't make this less than 16

#if (PTRBITS == 32)
#define SLABNHDR     (1UL << (PTRBITS - SLABMINLOG2))
#define SLABHDRTABSZ (SLABNHDR * sizeof(struct slabhdr))
#define SLABHDRBASE  (VIRTBASE - SLABHDRTABSZ)
#endif

#define slabnum(ptr)                                                    \
    ((uintptr_t)(ptr) >> SLABMINLOG2)
#define slabhdrnum(hdr, tab)                                            \
    (!(hdr) ? 0 : (uintptr_t)((hdr) - (tab)))
#if (PTRBITS == 32)
#define slabadr(hdr, tab)                                               \
    ((void *)(slabhdrnum(hdr, tab) << SLABMINLOG2))
#define slabhdr(ptr, tab)                                               \
    (!(ptr) ? NULL : (tab) + slabnum(ptr))
#else
#error slab.h does not support x86-64
#endif

#endif /* __MEM_SLAB_H__ */

