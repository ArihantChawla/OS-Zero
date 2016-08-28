#ifndef __ZERO_MEM_H__
#define __ZERO_MEM_H__

/* generic memory manager definitions for libzero */

#define MEM_LK_NONE   0                 // don't use locks; single-thread
#define MEM_LK_PRIO   1                 // priority-based locklessinc.com lock
#define MEM_LK_FMTX   2                 // anonymous non-recursive mutex
#define MEM_LK_SPIN   3                 // spinlock
#define MEM_LK_BIT    4                 // bit-lock with atomic operations

#define MEM_LK_TYPE   MEM_LK_PRIO       // type of locks to use

#include <limits.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#if (MEM_LK_TYPE == MEM_LK_PRIO)
#include <zero/priolk.h>
#elif (MEM_LK_TYPE == MEM_LK_FMTX)
#include <zero/mtx.h>
#if (MEM_LK_TYPE == MEM_LK_SPIN)
#include <zero/spin.h>
#if (MEM_LK_TYPE == MEM_LK_BIT)
#include <zero/asm.h>
#endif

/* types */

#if (WORDSIZE == 4)
typedef int32_t   MEMWORD_T;    // machine word
typedef uint32_t  MEMUWORD_T;   // unsigned machine word
#elif (WORDSIZE == 8)
typedef int64_t   MEMWORD_T;
typedef uint64_t  MEMUWORD_T;
#endif
typedef uintptr_t MEMADR_T;     // address (with possible flag/lock-bits)
typedef intptr_t  MEMADRDIFF_T; // for possible negative values
typedef void *    MEMPTR_T;     // could be char * for older systems

#if (MEM_LK_TYPE == MEM_LK_PRIO)
typedef struct priolk MEMLK_T;
#elif (MEM_LK_TYPE == MEM_LK_FMTX)
typedef zerofmtx      MEMLK_T;
#if (MEM_LK_TYPE == MEM_LK_SPIN)
typedef zerospin      MEMLK_T;
#if (MEM_LK_TYPE == MEM_LK_BIT)
typedef volatile long MEMLK_T;
#endif

/* macros */

#if (MEM_LK_TYPE == MEM_LK_PRIO)
#define memgetlk(lp) priolk(lp)
#define memrellk(lp) priounlk(lp)
#elif (MEM_LK_TYPE == MEM_LK_FMTX)
#define memgetlk(lp) fmtxlk(lp)
#define memrellk(lp) fmtxunlk(lp)
#if (MEM_LK_TYPE == MEM_LK_SPIN)
#define memgetlk(lp) spinlk(lp)
#define memrellk(lp) spinunlk(lp)
#if (MEM_LK_TYPE == MEM_LK_BIT)
#define MEM_LK_BIT_POS 0
#define memgetlk(lp)                                                    \
    do {                                                                \
        ;                                                               \
    } while (m_cmpsetbit((volatile long *)lp, MEM_LK_BIT_POS))
#define memrellk(lp) m_cmpclrbit((volatile long *)lp, MEM_LK_BIT_POS)
#endif

/* bins shall be allocated as runs of pages of PAGESIZE */

#define MEM_BIN_MIN_BLK_SIZE __BIGGEST_ALIGNMENT__
#define MEM_BIN_MAX_BLK_SIZE PAGESIZE
#define MEM_BIN_MIN_SIZE     PAGESIZE     // basic heap or map allocation size
#define MEM_BIN_MAX_SIZE     (MEM_BIN_MAX_BLOCKS * MEM_BIN_MAX_BLK_SIZE)
#define MEM_BIN_FREEMAP_SIZE (4 * WORDSIZE) // bitmap size in bytes; 0 for free
#define MEM_BIN_MAX_BLOCKS   (MEM_BIN_MAP_SIZE * CHAR_BIT)

/* use the low-order bit of the words and pointer to lock data */
#define MEM_LK_BIT_POS     0
#define MEM_LK_BIT         (1L << MEM_LK_BIT_POS)

/*
 * bin structure for allocating runs of pages; crafted to fit in a cacheline
 * - the actual runs will be prefixed by this structure
 * - allocation shall take place with sbrk() or mmap() (see MEM_BIN_MAP_BIT)
 */

/* flag-bits for the data.info.flg-member */
#define MEM_BIN_MAP_BIT    (1 << 0)     // mapped-memory as opposed to heap
struct membin {
    MEMADR_T         adr;       // base address (PAGESIZE-aligned)
    struct membin   *prev;      // previous in-core bin (heap-trimming)
    struct membin   *next;      // next bin in chain
    /* this union should be 64 bits in size */
    union {
        MEMWORD_T    info;      // can be used for word-size access to members
        struct {
            uint8_t  blksz;     // block-size as shift-count for one
            uint8_t  npage;     // page-count as shift-count for one
            uint8_t  binsz;     // bin-size as shift-count for one
            uint8_t  flg;       // bin flag-bits
        } parm;
    } data;
    MEMWORD_T        freemap[4]; // free-bitmap; 0-bit means block is free
} ALIGNED(CLSIZE);

struct membinbkt {
    MEMADR_T       base;        // base address
    MEMWORD_T      slot;        // slot ID; bins are PAGESIZE << slot bytes
    MEMWORD_T      blksz;       // block-size as shift-count for one
    MEMWORD_T      nbin;        // number of bins in list
    struct membin *list;        // bi-directional list of bins in the bucket
    MEMWORD_T      nfree;       // number of free bins
    struct membin *free;        // list of free bins
    MEMWORD_T      nbuf;        // number of bins to allocate/buffer at a time
} ALIGNED(CLSIZE);

#if (MEMLFDEQ)
#define memlklist(lp)
#define memunlklist(lp)
typedef struct lfdeqnode MEMLISTNODE_T;
typedef struct lfdeq     MEMLIST_T;
#else
#define memlklist(lp)    memgetlk(lp)
#define memunlklist(lp)  memrellk(lp)
typedef struct {
    struct memmag *prev;
    struct memmag *next;
} MEMLISTNODE_T;
typedef MEMLISTNODE_T *  MEMLIST_T;
#endif

/* flg-member values */
#define MEM_MAG_MAP_BIT (1L << 0)
struct memmag {
    MEMADR_T        base;       // allocation base address
    long            bkt;        // bucket ID; allocations are 1UL << bkt bytes
    long            flg;        // magazine flag-bits
    long            top;        // current top of stack index
    long            lim;        // number of entries in stk
    uint8_t        *stk;        // pointer stack; uint8_t for easy arithmetics
    void          **adrtab;     // table of unaligned pointers
    MEMLISTNODE_T   link;       // list links
    struct memarn  *arn;
    uint8_t         data[EMPTY] ALIGNED(CLSIZE);
} ALIGNED(CLSIZE);

struct memmagbkt {
    MEMLIST_T lstab[PTRBITS];
} ALIGNED(CLSIZE);

#define memptrid(mag, ptr)                                              \
    (((MEMADR_T)(ptr) - (mag)->base) >> (mag)->bkt)
#define memputadr(mag, ptr, adr)                                        \
    ((mag)->adrtab[memptrid(mag, ptr)] = (adr))
#define memgetadr(mag, ptr)                                             \
    ((mag)->adrtab[memptrid(mag, ptr)])

#endif /* __ZERO_MEM_H__ */

