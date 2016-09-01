#ifndef __ZERO_MEM_H__
#define __ZERO_MEM_H__

#if !defined(MEMLFDEQ)
#define MEMLFDEQ      0
#endif

/* generic memory manager definitions for libzero */

#define MEM_LK_NONE   0                 // don't use locks; single-thread
#define MEM_LK_PRIO   1                 // priority-based locklessinc.com lock
#define MEM_LK_FMTX   2                 // anonymous non-recursive mutex
#define MEM_LK_SPIN   3                 // spinlock

#define MEM_LK_TYPE   MEM_LK_PRIO       // type of locks to use

#include <limits.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/trix.h>
#if (MEM_LK_TYPE == MEM_LK_PRIO)
#include <zero/priolk.h>
#elif (MEM_LK_TYPE == MEM_LK_FMTX)
#include <zero/mtx.h>
#elif (MEM_LK_TYPE == MEM_LK_SPIN)
#include <zero/spin.h>
#endif
#include <zero/asm.h>

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
typedef uint8_t * MEMPTR_T;     // could be char * for older systems

#if (MEM_LK_TYPE == MEM_LK_PRIO)
typedef struct priolk MEMLK_T;
#elif (MEM_LK_TYPE == MEM_LK_FMTX)
typedef zerofmtx      MEMLK_T;
#elif (MEM_LK_TYPE == MEM_LK_SPIN)
typedef zerospin      MEMLK_T;
#elif (MEM_LK_TYPE == MEM_LK_BIT)
typedef volatile long MEMLK_T;
#endif

/* macros */

#if (WORDSIZE == 4)
#define MEMWORD(u)  INT32_C(u)
#define MEMUWORD(u) UINT32_C(u)
#elif (WORDSIZE == 8)
#define MEMWORD(u)  INT64_C(u)
#define MEMUWORD(u) UINT64_C(u)
#endif

#if (MEM_LK_TYPE == MEM_LK_PRIO)
#define memgetlk(lp) priolk(lp)
#define memrellk(lp) priounlk(lp)
#elif (MEM_LK_TYPE == MEM_LK_FMTX)
#define memgetlk(lp) fmtxlk(lp)
#define memrellk(lp) fmtxunlk(lp)
#elif (MEM_LK_TYPE == MEM_LK_SPIN)
#define memgetlk(lp) spinlk(lp)
#define memrellk(lp) spinunlk(lp)
#endif

/* use the low-order bit of the word or pointer to lock data */
#define MEMLKBITID 0
#define MEMLKBIT   (1L << MEMLKBITID)
#define MEMMAPBIT  (1L << 1)
#define memlkbit(lp)                                                    \
    do {                                                                \
        ;                                                               \
    } while (m_cmpsetbit((volatile long *)lp, MEMLKBITID))
#define memrelbit(lp) m_cmpclrbit((MEMLK_T *)lp, MEMLKBITID)

#if (WORDSIZE == 4)
#define memcalcslot(sz)                                                 \
    ceilpow2_32(sz)
#elif (WORDSIZE == 8)
#define memcalcslot(sz)                                                 \
    ceilpow2_64(sz)
#endif
#define membinhdrsize() (sizeof(struct membin))
#define membinsize(slot)                                                \
    (rounduppow2(membinhdrsize() + (MEMBINBLKS << (slot)), PAGESIZE))
#define memptrid(mag, ptr)                                              \
    (((MEMPTR_T)(ptr) - (mag)->base) >> (mag)->bkt)
#define memputadr(mag, ptr, adr)                                        \
    ((mag)->adrtab[memptrid(mag, ptr)] = (adr))
#define memgetadr(mag, ptr)                                             \
    ((mag)->adrtab[memptrid(mag, ptr)])

#if defined(__BIGGEST_ALIGNMENT__)
#define MEMMINALIGN     __BIGGEST_ALIGNMENT__
#else
#define MEMMINALIGN     (PTRSIZELOG2 + 1) // allow for dual-word tagged pointers
#endif
#if (MEMMINALIGN == 8)
#define MEMALIGNSHIFT   3
#elif (MEMMINALIGN == 16)
#define MEMALIGNSHIFT   4
#elif (MEMMINALIGN == 32)
#define MEMALIGNSHIFT   5
#endif
#define MEMMINBLK       (MEMUWORD(1) << MEMALIGNSHIFT)
#define MEMBINMAXBLK    (MEMUWORD(1) << MEMBINMAXSLOT)
#define MEMBINMAXSLOT   (PAGESIZELOG2 - 1)
#define MEMBINFREEWORDS (CLSIZE / WORDSIZE)
/* NOTE: the first block is the bin header */
#define MEMBINBLKS      (MEMBINFREEWORDS * WORDSIZE * CHAR_BIT)
#define MEMMAPMINSIZE   (1UL << 20)

/*
 * bin structure for allocating runs of pages; crafted to fit in a cacheline
 * - a second cacheline is used for the bitmap; 1-bits denote blocks in use
 * - the actual runs will be prefixed by this structure
 * - allocation shall take place with sbrk() or mmap() (see MEMMAPBIT)
 * - data is a placeholder/marker for the beginning of allocated blocks
 */

#define MEMINITBIT   (1L << 0)
#define MEMNOHEAPBIT (1L << 1)
struct mem {
    MEMWORD_T      flg;
    struct membin *heap;
    MEMLK_T        initlk;
    MEMLK_T        heaplk;
};

struct membin {
    MEMWORD_T         flg;      // flag-bits + lock-bit
    MEMPTR_T         *base;     // base address
    struct membin    *heap;     // previous bin in heap for bins from sbrk()
    struct membin    *prev;     // previous bin in chain
    struct membin    *next;     // next bin in chain
    struct membinbkt *bkt;      // pointer to parent bucket
    MEMWORD_T         slot;     // bucket slot #
    /* note: the first bit in freemap is reserved (unused) */
    MEMWORD_T         freemap[MEMBINFREEWORDS] ALIGNED(CLSIZE);
};

struct membinbkt {
    struct membin *list;        // bi-directional list of bins + lock-bit
    MEMWORD_T      slot;        // bucket slot #
    MEMWORD_T      nbin;        // number of bins in list
    MEMWORD_T      nbuf;        // number of bins to allocate/buffer at a time
};

#if (MEMLFDEQ)
typedef struct lfdeqnode MEMLISTNODE_T;
typedef struct lfdeq     MEMLIST_T;
#else
typedef struct {
    struct memmag *prev;
    struct memmag *next;
} MEMLISTNODE_T;
typedef struct memmag * MEMLIST_T;
#endif

struct memmag {
    MEMLISTNODE_T   link;       // list-linkage
    long            flg;        // magazine flag-bits + lock-bitn
    long            top;        // current top of stack index
    long            nblk;       // number of entries in stk
    void          **stk;        // pointer stack + unaligned pointers
    long            bkt;        // bucket ID; allocations are 1UL << bkt bytes
    MEMADR_T        base;       // magazine base address
    MEMUWORD_T      size;       // magazine size (header + blocks)
} ALIGNED(CLSIZE);

/*
 * NOTE: the arenas are mmap()'d as PAGESIZE-allocations so there's going
 * to be some room in the end for arbitrary data
 */
#define MEMARNSIZE PAGESIZE
#define memarndatasize() (PAGESIZE - sizeof(struct memarn))
struct memarn {
    MEMLIST_T qtab[PTRBITS];    // magazine buckets
    uint8_t   data[EMPTY];      // room for data
};

struct memmagbkt {
    MEMLIST_T qtab[PTRBITS];
} ALIGNED(CLSIZE);

/* mark the first block of bin as allocated; bit #0 is unused */
#define _memfillmap0(ptr, ofs, mask)                                    \
    ((ptr)[(ofs)] = (mask) & ~MEMWORD(2),                               \
     (ptr)[(ofs) + 1] = (mask),                                         \
     (ptr)[(ofs) + 2] = (mask),                                         \
     (ptr)[(ofs) + 3] = (mask))
#define _memfillmap(ptr, ofs, mask)                                     \
    ((ptr)[(ofs)] = (mask),                                             \
     (ptr)[(ofs) + 1] = (mask),                                         \
     (ptr)[(ofs) + 2] = (mask),                                         \
     (ptr)[(ofs) + 3] = (mask))

static __inline__ void
membininitfree(struct membin *bin)
{
    MEMWORD_T  bits = ~MEMWORD(0); // all 1-bits
    MEMWORD_T *ptr = bin->freemap;

#if (MEMBINFREEWORDS >= 4)
    _memfillmap0(ptr, 0, bits);
#elif (MEMBINFREEWORDS >= 8)
    _memfillmap(ptr, 4, bits);
#elif (MEMBINFREEWORD == 16)
    _memfillmap(ptr, 8, bits);
    _memfillmap(ptr, 12, bits);
#else
    memset(bin->freemap, 0xff, sizeof(bin->freemap));
#endif

    return;
}

#endif /* __ZERO_MEM_H__ */

