#ifndef __KERN_MEM_MAG_H__
#define __KERN_MEM_MAG_H__

#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <kern/mem/mem.h>

#define mempop(mp)       ((mp)->ptab[((mp)->ndx)++])
#define mempush(mp, ptr) ((mp)->ptab[--((mp)->ndx)] = (ptr))
#define memmagempty(mp)  ((mp)->ndx == (mp)->n)
#define memmagfull(mp)   (!(mp)->ndx)

#define __STRUCT_MEMMAG_PTAB_SIZE                                       \
    ((1UL << (MEMSLABSHIFT - MEMMINSHIFT)) * sizeof(void *))
#if defined(MEMPARANOIA)
#if (MEMSLABSHIFT - MEMMINSHIFT < (LONGSIZESHIFT + 3))
#define __STRUCT_MEMMAG_BMAP_SIZE sizeof(long)
#else
#define
#define __STRUCT_MEMMAG_BMAP_SIZE                                       \
    (1UL << (MEMSLABSHIFT - MEMMINSHIFT - 3))
#endif
#else
#define __STRUCT_MEMMAG_BMAP_SIZE 0
#endif /* defined(MEMPARANOIA) */
struct memmag {
    uintptr_t      base;
    volatile long  info;
    struct memmag *prev;
    struct memmag *next;
#if defined(MEMPARANOIA) && (MEMSLABSHIFT - MEMMINSHIFT < (LONGSIZESHIFT + 3))
    unsigned long  bmap;
#endif
    volatile long  ndx;
    volatile long  n;
    void          *ptab[1UL << (MEMSLABSHIFT - MEMMINSHIFT)];
#if defined(MEMPARANOIA) &&  !(MEMSLABSHIFT - MEMMINSHIFT < (LONGSIZESHIFT + 3))
    uint8_t        bmap[__STRUCT_MEMMAG_BMAP_SIZE];
#endif /* defined(MEMPARANOIA) */
};

#if 0
    
#define memgetmag(ptr, pool)                                            \
    (((ptr)                                                             \
      ? ((struct memmag *)(pool)->blktab + memgetmagnum(ptr, pool))     \
      : NULL))

#endif

#define memmagclrinfo(mp)                                               \
    ((mp)->info = ((m_ureg_t)0))
#define memmagclrbkt(mp)                                                \
    ((mp)->info &= ~MEMBKTMASK)
#define memmagsetbkt(mp, bkt)                                           \
    (memmagclrbkt(mp), (mp)->info |= (bkt))
#define memmaggetbkt(mp)                                                \
    ((mp)->info & MEMBKTMASK)
#define memmagisfree(mp)                                                \
    (!((mp)->info & MEMFREE))
#define memmagsetfree(mp)                                               \
    ((mp)->info |= MEMFREE)
#define memmagclrfree(mp)                                               \
    ((mp)->info &= ~MEMFREE)
#define memmagsetflg(mp, flg)                                           \
    ((mp)->info |= (flg))
#define memmagclrflg(mp)                                                \
    ((mp)->info &= ~MEMFLGBITS)
#define memmagclrlink(mp)                                               \
    ((mp)->prev = (mp)->next = NULL)
#define memmagsetprev(mp, hdr)                                          \
    ((mp)->prev = hdr)
#define memmagsetnext(mp, hdr)                                          \
    ((mp)->next = hdr)
#define memmaggetprev(mp)                                               \
    ((mp)->prev)
#define memmaggetnext(mp)                                               \
    ((mp)->next)
#define memmagclrprev(mp)                                               \
    ((mp)->prev = NULL)
#define memmagclrnext(mp)                                               \
    ((mp)->next = NULL)

#endif /* __KERN_MEM_MAG_H__ */

