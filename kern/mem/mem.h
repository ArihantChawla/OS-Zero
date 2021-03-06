#ifndef __KERN_MEM_MEM_H__
#define __KERN_MEM_MEM_H__

//#define MEMPARANOIA 1
#undef MEMPARANOIA

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <mach/param.h>
#include <mach/types.h>
#include <zero/trix.h>

#if (MEMTKTLK)
#include <mt/tktlk.h>
#define MEM_LK_T       zerotktlk
#define memlk(lp)      tktlk(lp)
#define memunlk(lp)    tktunlk(lp)
#define memlkbkt(lp)   tktlk(lp)
#define memunlkbkt(lp) tktunlk(lp)
#else
#include <mt/mtx.h>
#define MEM_LK_T       zerofmtx
#define memlk(lp)      fmtxlk(lp)
#define memunlk(lp)    fmtxunlk(lp)
#define memlkbkt(lp)   fmtxlk(lp)
#define memunlkbkt(lp) fmtxunlk(lp)
#endif

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
#define MEMHASHHDRS   (512 * 1024)     // # of entries in header hash table
//#define MEMNHDRBUF     (roundup(__STRUCT_MEMBLK_SIZE, CLSIZE))
#define MEMMINSIZE    (1U << MEMMINSHIFT)
#define MEMSLABSIZE   (1U << MEMSLABSHIFT)
#define MEMMINSHIFT   CLSIZELOG2 // minimum allocation of 256 bytes
#define MEMSLABSHIFT  16
#define MEMBKTBITS    8 // 8 low bits of info used for bucket ID (max 255)
#define MEMNTYPEBIT   8 // up to 256 object types
#define MEMBKTMASK    ((1UL << (MEMBKTBITS - 1)) - 1)
#define MEMLKBIT      (1UL << (WORDSIZE * CHAR_BIT - 1))
#define MEMLKBITPOS   (WORDSIZE * CHAR_BIT - 1)
/* allocation flags */
#define MEMFREE       0x00000001UL
#define MEMZERO       0x00000002UL
#define MEMWIRE       0x00000004UL
#define MEMDIRTY      0x00000008UL
#define MEMFLGBITS    (MEMFREE | MEMZERO | MEMWIRE | MEMDIRTY)
#define MEMNFLGBIT    4
#define memslabsize(bkt)                                                \
    (1UL << (bkt))
#define memtrylkhdr(hdr) m_trylkbit(&hdr->info, MEMLKBITPOS)
#define memlkhdr(hdr)    m_lkbit(&hdr->info, MEMLKBITPOS)
#define memunlkhdr(hdr)  m_unlkbit(&hdr->info, MEMLKBITPOS)
#define memgetbkt(hdr)   ((hdr)->info & MEMBKTMASK)

void meminit(m_ureg_t base, m_ureg_t nbphys, m_ureg_t nbvirt);

#endif /* __KERN_MEM_MEM_H__ */

