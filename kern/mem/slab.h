#ifndef __MEM_SLAB_H__
#define __MEM_SLAB_H__

#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>

struct slabhdr {
    unsigned long   nfo;
#if (PTRBITS <= 32)
    uint32_t        link;
#else
    struct slabhdr *prev;
    struct slabhdr *next;
#endif
} PACK();

#define slablk(tab, bkt)   printf("%d: LK: %ld\n", __LINE__, bkt); mtxlk(&(tab)[bkt], MEMPID)
#define slabunlk(tab, bkt) printf("%d: UNLK: %ld\n", __LINE__, bkt); mtxunlk(&(tab)[bkt], MEMPID)
#if 0
#define slablk(tab, bkt)   mtxlk(&(tab)[bkt], MEMPID)
#define slabunlk(tab, bkt) mtxunlk(&(tab)[bkt], MEMPID)
#endif

#define slabnum(ptr, base)                                              \
    (((uintptr_t)(ptr) - (base)) >> SLABMINLOG2)
#define slabgetadr(hdr, tab)                                            \
    ((void *)(slabvirtbase + ((unsigned long)slabhdrnum(hdr, tab) << SLABMINLOG2)))
#define slabhdrnum(hdr, tab)                                            \
    (!(hdr) ? 0 : (uintptr_t)((hdr) - (struct slabhdr *)(tab)))
#define slabgethdr(ptr, tab, base)                                      \
    (!(ptr) ? NULL : (struct slabhdr *)(tab) + slabnum(ptr, base))

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

void   slabinit(unsigned long base, unsigned long nb);
void * slaballoc(struct slabhdr **zone, struct slabhdr *hdrtab,
                 unsigned long nb, unsigned long flg);
void slabfree(struct slabhdr **zone, struct slabhdr *hdrtab, void *ptr);

#endif /* __MEM_SLAB_H__ */

