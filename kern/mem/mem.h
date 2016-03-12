#ifndef __KERN_MEM_MEM_H__
#define __KERN_MEM_MEM_H__

#define MEMNEWSLAB  1
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
//#define MEMNHDRBUF     (roundup(__STRUCT_MEMBLK_SIZE, CLSIZE))
#define MEMMINSIZE     (1U << MEMMINSHIFT)
#define MEMSLABSIZE    (1U << MEMSLABSHIFT)
#define MEMMINSHIFT    8 // minimum allocation of 256 bytes
#define MEMSLABSHIFT   20
#if 0
#define MEMMAXOBJ      (1U << MEMMAXOBJSHIFT)
#define MEMMAXOBJSHIFT 16
#endif
#define MEMNBKTBIT     8 // 8 low bits of info used for bucket ID (max 255)
#define MEMNTYPEBIT    8 // up to 256 object types
#define MEMBKTMASK     ((1UL << (MEMNBKTBIT - 1)) - 1)
/* allocation flags */
#define MEMFREE        0x00000001UL
#define MEMZERO        0x00000002UL
#define MEMWIRE        0x00000004UL
#define MEMDIRTY       0x00000008UL
#define MEMFLGBITS     (MEMFREE | MEMZERO | MEMWIRE | MEMDIRTY)
#define MEMNFLGBIT     4
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
#define MEMMINLOG2     CLSIZELOG2
#define MEMZERO        0x00000001UL
#define MEMWIRE        0x00000002UL
#define MEMFREE        0x000000004UL
#define MEMFLGBITS     (MEMZERO | MEMWIRE | MEMFREE)
#define MEMNFLGBIT     4
#endif

void          meminit(size_t nbphys, size_t nvirt);
void          meminitphys(struct mempool *physpool, uintptr_t base,
                          size_t nbphys);
unsigned long meminitpool(struct mempool *pool, uintptr_t base, size_t nb);
void          memfree(struct mempool *pool, void *ptr);

#endif /* __KERN_MEM_MEM_H__ */

