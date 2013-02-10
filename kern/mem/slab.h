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

#define slablk(bkt)   mtxlk(&virtlktab[bkt], MEMPID);
#define slabunlk(bkt) mtxunlk(&virtlktab[bkt], MEMPID);

#if (PTRBITS > 32)
#define slabnum(ptr)                                                    \
    ((uintptr_t)((uint8_t *)(ptr) - slabvirtbase) >> SLABMINLOG2)
#define slabgetadr(hdr, tab)                                            \
    ((void *)(slabvirtbase + ((unsigned long)slabhdrnum(hdr, tab) << SLABMINLOG2)))
#else
#define slabnum(ptr)                                                    \
    ((uintptr_t)(ptr) >> SLABMINLOG2)
#define slabgetadr(hdr, tab)                                            \
    ((void *)(slabhdrnum(hdr, tab) << SLABMINLOG2))
#endif
#define slabhdrnum(hdr, tab)                                            \
    (!(hdr) ? 0 : (uintptr_t)((hdr) - (struct slabhdr *)(tab)))
#define slabgethdr(ptr, tab)                                           \
    (!(ptr) ? NULL : (struct slabhdr *)(tab) + slabnum(ptr))

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
#if (PTRBITS > 32)
extern struct slabhdr *virthdrtab;
#else
extern struct slabhdr  virthdrtab[];
#endif

#define SLABMIN      (1UL << SLABMINLOG2)
#define SLABMINLOG2  16 // don't make this less than 16

#if defined(__i386__) || defined(__arm__) && !defined(__x86_64__) && !defined(__amd64__)
#include <kern/mem/slab32.h>
#elif defined(__x86_64__) || defined(__amd64__)
#include <kern/mem/slab64.h>
#endif

#endif /* __MEM_SLAB_H__ */

