#ifndef __KERN_MEM_MEM_H__
#define __KERN_MEM_MEM_H__

#define MEMNEWSLAB  0
//#define MEMPARANOIA 1
#undef MEMPARANOIA

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/mem/pool.h>
#include <kern/mem/slab.h>

#if (MEMNEWSLAB)
/* allocator parameters */
#define MEMNHDRHASH    (512 * 1024)     // # of entries in header hash table
#define MEMNHDRBUF     (roundup(__STRUCT_MEMBLK_SIZE, CLSIZE))
/* allocation flags */
#define MEMMIN         (1U << MEMMINSHIFT)
#define MEMMINSHIFT    8 // minimum allocation of 256 bytes
#define MEMMAXOBJ      (1U << MEMMAXOBJSHIFT)
#define MEMMAXOBJSHIFT 16
#define MEMNBKTBIT     8 // 8 low bits of info used for bucket ID (max 255)
#define MEMNTYPEBIT    8 // up to 256 object types
#define MEMBKTMASK     ((1UL << (MEMNBKTBIT - 1)) - 1)
#define MEMFLGSHIFT    (1 << (CHAR_BIT * sizeof(m_ureg_t) - 1))
#define MEMLOCKBIT     (1 << MEMFLGSHIFT) // set when manipulating header
#define MEMALLOCBIT    (1 << (MEMFLGSHIFT - 1))
#define MEMDIRTYBIT    (1 << (MEMFLGSHIFT - 2))
#define MEMWIREDBIT    (1 << (MEMBASEFHIT - 3))
#define MEMNFLGBIT     8
#define MEMFLGMASK                                                      \
    (((m_ureg_t)~0) << (CHAR_BIT * sizeof(m_ureg_t) - MEMNFLGBIT))
#define memslabsize(bkt)                                                \
    (1UL << (bkt))
#define memtrylkhdr(hdr)                                                \
    (!m_cmpsetbit(&hdr->info, MEMLOCKBIT))
#define memlkhdr(hdr)                                                   \
    do {                                                                \
        volatile long res;                                              \
                                                                        \
        do {                                                            \
            res =  !m_cmpsetbit(&hdr->info, MEMLOCKBIT);                \
        } while (!res);                                                 \
    } while (0)
#define memunlkhdr(hdr)                                                 \
    (m_unlkbit(&(hdr)->info, MEMLOCKBIT))
#define memgetbkt(hdr)                                                  \
    (&(hdr)->info & ((1 << MEMNBKTBIT) - 1))
#else /* !MEMNEWSLAB */
#define MEMMIN         (1UL << MEMMINLOG2)
#define MEMMINLOG2     PAGESIZELOG2
#define MEMFREE        0x00000001UL
#define MEMZERO        0x00000002UL
#define MEMWIRE        0x00000004UL
#define MEMFLGBITS     (MEMFREE | MEMZERO | MEMWIRE)
#define MEMNFLGBIT     4
#endif

void          meminit(size_t nbphys);
void          meminitphys(struct mempool *physpool, uintptr_t base,
                          size_t nbphys);
unsigned long meminitpool(struct mempool *pool, uintptr_t base, size_t nb);
void          memfree(struct mempool *pool, void *ptr);

#endif /* __KERN_MEM_MEM_H__ */

