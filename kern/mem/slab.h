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

extern struct slabhdr *virtslabtab[];
extern struct slabhdr  virthdrtab[];

#define SLABMIN      (1UL << SLABMINLOG2)
#define SLABMINLOG2  16 // don't make this less than 16

#if (PTRBITS == 32)
#define SLABNHDR     (1UL << (PTRBITS - SLABMINLOG2))
#define SLABHDRTABSZ (SLABNHDR * sizeof(struct slabhdr))
#define SLABHDRBASE  (VIRTBASE - SLABHDRTABSZ)
#endif
#if (PTRBITS != 32)
#error slab.h does not support x86-64
#endif

#endif /* __MEM_SLAB_H__ */

