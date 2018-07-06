#ifndef __KERN_MEM_MEM_H__
#define __KERN_MEM_MEM_H__

//#define MEMPARANOIA 1
#undef MEMPARANOIA

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <mach/param.h>
#include <zero/trix.h>

/* memory zones */
#define MEM_LOW_ZONE    0 // low-memory (x86: boot)
#define MEM_IO_ZONE     1 // I/O zone (x86: DMA)
#define MEM_SYS_ZONE    2 // system zone (page-tables and such)
#define MEM_VIRT_ZONE   3 // virtual memory zone
#define MEM_PHYS_ZONE   4 // physical memory zone
#define MEM_DEV_ZONE    5 // memory-mapped device zone
#define MEM_BUF_ZONE    6 // buffer cache zone
#define MEM_ZONES       8

/* memory caching type */
#define MEMWRBIT     0x01
#define MEMCACHEBIT  0x02
#define MEMWRBUFBIT  0x04
#define MEMWRPROT    0x00 // write-protectected
/* uncacheable */
#define MEMNOCACHE   (MEMWRBIT)
/* write-combining, uncached */
#define MEMWRCOMB    (MEMWRBIT | MEMIOBUFBIT)
/* write-through */
#define MEMWRTHRU    (MEMWRBIT | MEMCACHEBIT)
/* write-back */
#define MEMWRBACK    (MEMWRBIT | MEMCACHEBIT | MEMWRBUFBIT)

/* allocator parameters */
#define MEMNHDRHASH   (512 * 1024)     // # of entries in header hash table
//#define MEMNHDRBUF     (roundup(__STRUCT_MEMBLK_SIZE, CLSIZE))
#define MEMMINSIZE    (1U << MEMMINSHIFT)
#define MEMSLABSIZE   (1U << MEMSLABSHIFT)
#define MEMMINSHIFT   CLSIZELOG2 // minimum allocation of 256 bytes
#define MEMSLABSHIFT  16
#define MEMBKTBITS    8 // 8 low bits of info used for bucket ID (max 255)
#define MEMNTYPEBIT   8 // up to 256 object types
#define MEMBKTMASK    ((1UL << (MEMBKTBITS - 1)) - 1)
#define MEMLKBIT      (1ULL << (WORDSIZE * CHAR_BIT - 1))
/* allocation flags */
#define MEMFREE       0x00000001UL
#define MEMZERO       0x00000002UL
#define MEMWIRE       0x00000004UL
#define MEMDIRTY      0x00000008UL
#define MEMFLGBITS    (MEMFREE | MEMZERO | MEMWIRE | MEMDIRTY)
#define MEMNFLGBIT    4
#define memslabsize(bkt)                                                \
    (1UL << (bkt))
#define memtrylkhdr(hdr)                                                \
    (!m_cmpsetbit(&hdr->info, MEMLKBIT))
#define memlkhdr(hdr)                                                   \
    do {                                                                \
        volatile long res;                                              \
                                                                        \
        do {                                                            \
            while (hdr->info & MEMLKBIT) {                              \
                m_waitspin();                                           \
            }                                                           \
            res =  !m_cmpsetbit(&hdr->info, MEMLKBIT);                  \
        } while (!res);                                                 \
    } while (0)
#define memunlkhdr(hdr)                                                 \
    (m_clrbit(&(hdr)->info, MEMLKBIT))
#define memgetbkt(hdr)                                                  \
    (&(hdr)->info & ((1 << MEMBKTBITS) - 1))

#include <kern/mem/slab.h>
#include <kern/mem/mag.h>
#include <kern/mem/bkt.h>
#include <kern/mem/zone.h>
//#include <kern/mem/page.h>

void meminit(size_t nbphys, size_t nbvirt);
void meminitphys(struct memzone *zone, uintptr_t base, size_t nbyte);
void meminitvirt(struct memzone *zone, size_t nbvirt);
void memfree(struct memzone *zone, void *ptr);

#endif /* __KERN_MEM_MEM_H__ */

