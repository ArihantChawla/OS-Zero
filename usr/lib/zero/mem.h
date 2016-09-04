#ifndef __ZERO_MEM_H__
#define __ZERO_MEM_H__

#if !defined(MEMLFDEQ)
#define MEMLFDEQ      0
#endif
#if !defined(MEMTABNREF)
#define MEMTABNREF    0
#endif

/* generic memory manager definitions for libzero */

#define MEM_LK_NONE   0                 // don't use locks; single-thread
#define MEM_LK_PRIO   1                 // priority-based locklessinc.com lock
#define MEM_LK_FMTX   2                 // anonymous non-recursive mutex
#define MEM_LK_SPIN   3                 // spinlock

#define MEM_LK_TYPE   MEM_LK_PRIO       // type of locks to use

#include <limits.h>
#include <stddef.h>
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
typedef uint8_t * MEMPTR_T;     // could be char * too; needs to be single-byte

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
#define MEMWORD(i)  INT32_C(i)
#define MEMUWORD(u) UINT32_C(u)
#elif (WORDSIZE == 8)
#define MEMWORD(i)  INT64_C(i)
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
#define MEMLKBITID  0
#define MEMLKBIT    (1L << MEMLKBITID)
#define memlkbit(lp)                                                    \
    do {                                                                \
        ;                                                               \
    } while (m_cmpsetbit((volatile long *)lp, MEMLKBITID))
#define memrelbit(lp) m_cmpclrbit((volatile long *)lp, MEMLKBITID)

#if (WORDSIZE == 4)
#define memcalcslot(sz)                                                 \
    ceilpow2_32(sz)
#elif (WORDSIZE == 8)
#define memcalcslot(sz)                                                 \
    ceilpow2_64(sz)
#endif

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
#define MEMMINBLK          (MEMUWORD(1) << MEMALIGNSHIFT)
#define MEMBINMAXBLK       (MEMUWORD(1) << MEMBINMAXSLOT)
#define MEMBINMAXSLOT      (PAGESIZELOG2 - 1)
#define MEMBINFREEMAPWORDS (CLSIZE / WORDSIZE)
/* NOTE: the first block is the bin header */
#define MEMBINBLKS         (MEMBINFREEMAPWORDS * WORDSIZE * CHAR_BIT)
#define MEMMAPMINSIZE      (1UL << 20)

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
    MEMWORD_T      flg;         // memory interface flags
    struct membin *heap;        // heap allocations (try sbrk(), then mmap())
    struct membin *maps;        // mapped regions
    struct memtab *tab;         // allocation lookup structure
    MEMLK_T        initlk;      // lock for initialising the structure
    MEMLK_T        heaplk;      // lock for sbrk()
};

#define MEMHEAPBIT    (1L << 1)
#define MEMEMPTYBIT   (1L << 2)
#define MEMBINFLGMASK ((1L << MEMBINFLGBITS) - 1)
#define MEMBINFLGBITS 3
#define memsetbinflg(bin, flg) ((bin)->info |= (flg))
#define memsetbinnblk(bin, n)                                           \
    ((bin)->info = ((bin)->info & MEMBINFLGMASK) | ((n) << MEMBINFLGBITS))
#define memgetbinnblk(bin)     ((bin)->info >> MEMBINFLGBITS)
struct membin {
    MEMUWORD_T     info;        // flag-bits + lock-bit
    MEMPTR_T       base;        // base address
    struct membin *heap;        // previous bin in heap for bins from sbrk()
    struct membin *prev;        // previous bin in chain
    struct membin *next;        // next bin in chain
    struct membkt *bkt;         // pointer to parent bucket
    MEMWORD_T      slot;        // bucket slot #
    MEMPTR_T      *atab;        // unaligned base pointers for aligned blocks
    /* note: the first bit in freemap is reserved (unused) */
    MEMWORD_T      freemap[MEMBINFREEMAPWORDS] ALIGNED(CLSIZE);
};

struct membkt {
#if (MEMLFDEQ)
    struct lfdeq   list;
#else
    struct membin *list;        // bi-directional list of bins + lock-bit
#endif
    MEMWORD_T      slot;        // bucket slot #
    MEMWORD_T      nbin;        // number of bins in list
    MEMWORD_T      nbuf;        // number of bins to allocate/buffer at a time
};

#if 0
struct memmagbkt {
    struct memmag *list;        // bi-directional list of bins + lock-bit
    MEMWORD_T      slot;        // bucket slot #
    MEMWORD_T      nbin;        // number of bins in list
    MEMWORD_T      nbuf;        // number of bins to allocate/buffer at a time
};
#endif

/* toplevel lookup table item */
struct memtab {
    MEMLK_T         lk;
    struct memitem *tab;
};
/*
 * we'll have 2 or 3 levels of these + a level of MEMADR_T values for lookups
 * under the toplevel table
 */
/* type-bits for the final-level table pointers */
#define MEMSMALLBLK 0x00
#define MEMPAGEBLK  0x01
#define MEMBIGBLK   0x02
#define MEMBINTYPES 3
/* lookup table structure for upper levels */
struct memitem {
#if (MEMTABNREF)
    volatile long   nref;
#endif
    struct memitem *tab;
};

/*
 * NOTE: the arenas are mmap()'d as PAGESIZE-allocations so there's going
 * to be some room in the end for arbitrary data
 */
#define MEMARNSIZE PAGESIZE
#define memarndatasize() (PAGESIZE - sizeof(struct memarn))
struct memarn {
    struct membkt small[PTRBITS]; // magazine buckets of size 1 << slot
    struct membkt page[PTRBITS];  // mapped regions of PAGESIZE * slot
    struct membkt big[PTRBITS];   // mapped regions of PAGESIZE << slot
/* possible auxiliary data here; arena is of PAGESIZE */
};

/* mark the first block of bin as allocated; bit #0 is magazine header */
#define _memfillmap0(ptr, ofs, mask)                                    \
    ((ptr)[(ofs)] = (mask) & ~MEMWORD(0x03),                            \
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

#if (MEMBINFREEMAPWORDS >= 4)
    _memfillmap0(ptr, 0, bits);
#elif (MEMBINFREEMAPWORDS >= 8)
    _memfillmap(ptr, 4, bits);
#elif (MEMBINFREEWORD == 16)
    _memfillmap(ptr, 8, bits);
    _memfillmap(ptr, 12, bits);
#else
    memset(bin->freemap, 0xff, sizeof(bin->freemap));
#endif

    return;
}

/*
 * find the lowest 1-bit (free block) in bin->freemap
 * - caller has to lock the bin; memlkbit(&bin->flg, MEMLKBIT);
 * - return index or 0 if not found (bit #0 indicates bin header)
 * - the routine is bitorder-agnostic... =)
 */
static __inline__ MEMWORD_T
membinfindblk(struct membin *bin)
{
    MEMUWORD_T  nblk = memgetbinnblk(bin);
    MEMWORD_T  *map = bin->freemap;
    MEMWORD_T   ndx = 0;
    MEMWORD_T  *lim = map + MEMBINFREEMAPWORDS;
    MEMWORD_T   word;
    MEMWORD_T   res;
    MEMWORD_T   bit;

    /* determine how many words to scan for free-bit (1) */
    if (!nblk) {
        lim = map + MEMBINFREEMAPWORDS;
    } else {
        lim = map + rounduppow2(nblk, WORDSIZE * CHAR_BIT) / WORDSIZE;
    }
    do {
        word = *map;
        if (word) {                             // skip 0-words
            bit = lo1bit(word);                 // extract low 1 in word
            tzerol(bit, res);                   // count trailing zeroes
            ndx += res;                         // add to ndx
            if (ndx < nblk) {
                
                return ndx;                     // return index of first 1-bit
            }

            return 0;
        }
        map++;                                  // try next word in freemap
        ndx += WORDSIZE * CHAR_BIT;
    } while (map < lim);

    return 0;                                   // 1-bit not found
}

#if 0
static __inline__ long
membingetblk(struct membin *bin)
{
    MEMUWORD_T nblk = memgetbinnblk(bin);
    long *map = bin->freemap;
    long  ndx = 0;
    long *lim = map + MEMBINFREEMAPWORDS;
    long  res;

    if (!nblk) {
        do {
            ndx++;
            res = m_cmpclrbit((volatile long *)map, ndx);
            if (res) {
                
                return ndx;
            }
            if (ndx == PTRBITS - 1) {
                map++;
                ndx = -1;
            }
        } while (!res && map < lim);
    } else {
        do {
            ndx++;
            res = m_cmpclrbit((volatile long *)map, ndx);
            if (res) {
                
                return ndx;
            }
            if (ndx == PTRBITS - 1) {
                map++;
                ndx = -1;
            }
        } while ((--nblk) && !res && map < lim);
    }

    return 0;
}
#endif

/*
 * for 32-bit pointers, we can use a flat lookup table for bookkeeping pointers
 * - for bigger pointers, we use a multilevel table
 */
#if (PTRBITS > 32)
#define MEMADRSHIFT   (PAGESIZELOG2 + MEMALIGNSHIFT)
#define MEMADRBITS    (ADRBITS - MEMADRSHIFT)
#if (ADRBITS > 52)
#define MEMLVL1BITS   (MEMADRBITS - 3 * MEMLVLBITS)
#else
#define MEMLVL1BITS   (MEMADRBITS - 2 * MEMLVLBITS)
#endif
#if (ADRBITS <= 52)
#define MEMLVLBITS    10
#else
#define MEMLVLBITS    12
#endif
#define MEMLVL1ITEMS  (MEMWORD(1) << MEMLVL1BITS)
#define MEMLVLITEMS   (MEMWORD(1) << MEMLVLBITS)
#define MEMLVL1MASK   ((MEMWORD(1) << MEMLVL1BITS) - 1)
#define MEMLVLMASK    ((MEMWORD(1) << MEMLVLBITS) - 1)
#define memlvl1key(p) (((MEMADR_T)(p) >> MEMLVL1SHIFT) & MEMLVL1MASK)
#define memlvl2key(p) (((MEMADR_T)(p) >> MEMLVL2SHIFT) & MEMLVLMASK)
#define memlvl3key(p) (((MEMADR_T)(p) >> MEMLVL3SHIFT) & MEMLVLMASK)
#if (ADRBITS > 52)
#define memlvl4key(p) (((MEMADR_T)(p) >> MEMLVL4SHIFT) & MEMLVLMASK)
#endif
#if (ADRBITS <= 52)
#define memgetkeybits(p, k1, k2, k3)                                    \
    do {                                                                \
        MEMADR_T _p1 = (MEMADR_T)(p) >> MEMADRSHIFT;                    \
        MEMADR_T _p2 = (MEMADR_T)(p) >> (MEMADRSHIFT + MEMLVLBITS);     \
        MEMADR_T _p3 = (MEMADR_T)(p) >> (MEMADRSHIFT + 2 * MEMLVLBITS); \
                                                                        \
        (k3) = _p1 & MEMLVLMASK;                                        \
        (k2) = _p2 & MEMLVLMASK;                                        \
        (k1) = _p3 & MEMLVL1MASK;                                       \
    } while (0)
#else
#define memgetkeybits(p, k1, k2, k3, k4)                                \
    do {                                                                \
        MEMADR_T _p1 = (MEMADR_T)(p) >> MEMADRSHIFT;                    \
        MEMADR_T _p2 = (MEMADR_T)(p) >> (MEMADRSHIFT + MEMLVLBITS);     \
                                                                        \
        (k4) = _p1 & MEMLVLMASK;                                        \
        (k3) = _p2 & MEMLVLMASK;                                        \
        _p1 >>= 2 * MEMLVLBITS;                                         \
        _p2 >>= 2 * MEMLVLBITS;                                         \
        (k2) = _p1 & MEMLVLMASK;                                        \
        (k1) = _p2 & MEMLVL1MASK;                                       \
    } while (0)
#endif
#endif

#define membinhdrsize()       (sizeof(struct membin))
#define membinatabofs(slot)   (membinhdrsize())
#define membinatabsize()      (MEMBINBLKS * sizeof(MEMPTR_T))
#define memsmallbinsize(slot)                                           \
    (rounduppow2(rounduppow2(membinhdrsize()                            \
                             + membinatabsize(),                        \
                             PAGESIZE)                                  \
                 + (MEMBINBLKS << (slot)),                              \
                 PAGESIZE))
#define mempagebinsize(slot, nblk)                                      \
    (rounduppow2(rounduppow2(membinhdrsize()                            \
                             + membinatabsize(),                        \
                             PAGESIZE)                                  \
                 + PAGESIZE * (slot) * (nblk),                          \
                 PAGESIZE))
#define membigbinsize(slot, nblk)                                       \
    (rounduppow2(rounduppow2(membinhdrsize()                            \
                             + membinatabsize(),                        \
                             PAGESIZE)                                  \
                 + (nblk) + ((nblk) << (slot)),                         \
                 PAGESIZE))

#define memptrid(mag, ptr)                                              \
    (((MEMPTR_T)(ptr) - (mag)->base) >> (mag)->bkt)

#endif /* __ZERO_MEM_H__ */

