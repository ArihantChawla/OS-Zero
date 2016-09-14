#ifndef __ZERO_MEM_H__
#define __ZERO_MEM_H__

#if !defined(MEMLFDEQ)
#define MEMLFDEQ    0
#endif
#if !defined(MEMTABNREF)
#define MEMTABNREF  0
#endif
#if !defined(MEMDEBUG)
#define MEMDEBUG    0
#endif

#if defined(MEMDEBUG)
#define crash(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            *(uint8_t *)NULL = 0x00;                                    \
        }                                                               \
    } while (0)
#endif

/* generic memory manager definitions for libzero */

#define MEM_LK_NONE 0           // don't use locks; single-thread
#define MEM_LK_PRIO 1           // priority-based locklessinc.com lock
#define MEM_LK_FMTX 2           // anonymous non-recursive mutex
#define MEM_LK_SPIN 3           // spinlock

#define MEM_LK_TYPE MEM_LK_PRIO // type of locks to use

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/asm.h>
#include <zero/unix.h>
#include <zero/trix.h>
#if (MEM_LK_TYPE == MEM_LK_PRIO)
#include <zero/priolk.h>
#elif (MEM_LK_TYPE == MEM_LK_FMTX)
#include <zero/mtx.h>
#elif (MEM_LK_TYPE == MEM_LK_SPIN)
#include <zero/spin.h>
#endif
#if defined(MEMVALGRIND) && (MEMVALGRIND)
#define ZEROVALGRIND 1
#include <zero/valgrind.h>
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

#define MEMPAGEBIT      (1L << (PAGESIZELOG2 - 1))
#define MEMPAGENDXMASK  (MEMPAGEBIT - 1)
#define MEMPAGEINFOMASK (MEMPAGEBIT | MEMPAGENDXMASK)
/* use the low-order bit of the word or pointer to lock data */
#define MEMLKBITID      0
#define MEMLKBIT        (1L << MEMLKBITID)
#if (MEMDEBUGLOCK)
#define memlkbit(ptr)                                                   \
    do {                                                                \
        fprintf(stderr, "LK: %s: %d\n", __FILE__, __LINE__);            \
    } while (m_cmpsetbit((m_atomic_t *)ptr, MEMLKBITID))
#define memrelbit(ptr) (fprintf(stderr, "UNLK: %s: %d\n", __FILE__, __LINE__), \
                        m_clrbit((m_atomic_t *)ptr, MEMLKBITID))

#else
#define memlkbit(ptr)                                                   \
    do {                                                                \
        ;                                                               \
    } while (m_cmpsetbit((m_atomic_t *)ptr, MEMLKBITID))
#define memrelbit(ptr) m_clrbit((m_atomic_t *)ptr, MEMLKBITID)
#endif

#if (WORDSIZE == 4)
#define memcalcslot(sz, slot)                                           \
    do {                                                                \
        long _tmp;                                                      \
        long _slot;                                                     \
                                                                        \
        ceilpow2_32(sz, _tmp);                                          \
        _slot = tzerol(_tmp);                                           \
        (slot) = _slot;                                                 \
    } while (0)
#elif (WORDSIZE == 8)
#define memcalcslot(sz, slot)                                           \
    do {                                                                \
        long _tmp;                                                      \
        long _slot;                                                     \
                                                                        \
        ceilpow2_64(sz, _tmp);                                          \
        _slot = tzeroll(_tmp);                                          \
        (slot) = _slot;                                                 \
    } while (0)
#endif

/* determine minimal required alignment for blocks */
#if defined(__BIGGEST_ALIGNMENT__)
#define MEMMINALIGN        max(__BIGGEST_ALIGNMENT__, 2 * PTRSIZE)
#else
#define MEMMINALIGN        (2 * PTRSIZE) // allow for dual-word tagged pointers
#endif
#if (MEMMINALIGN == 8)
#define MEMALIGNSHIFT      3
#elif (MEMMINALIGN == 16)
#define MEMALIGNSHIFT      4
#elif (MEMMINALIGN == 32)
#define MEMALIGNSHIFT      5
#endif
/* maximum small buf size is (MEMBUFBLKS << MEMMAXSMALLSHIFT) + bookkeeping */
#define MEMMAXSMALLSHIFT   (PAGESIZELOG2 - 1)
/* maximum page bin block size is PAGESIZE * PTRBITS */
#define MEMMAXPAGESLOT     (PTRBITS - 1)
/* NOTES
 * -----
 * - all allocations except those from pagebin are power-of-two sizes
 * - pagebin allocations are PAGESIZE * (slot + 1)
 */
/* minimum allocation block size */
#define MEMMINBLK           (MEMUWORD(1) << MEMALIGNSHIFT)
/* maximum allocation block size */
#define MEMMAXBUFBLK        (MEMUWORD(1) << MEMMAXBUFSLOT)
/* maximum slot # */
#define MEMMAXBUFSLOT       (PTRBITS - 1)
/* number of words in buf freemap */
//#define MEMBUFFREEMAPWORDS  (CLSIZE / WORDSIZE)
#define MEMBUFFREEMAPWORDS  8
/* number of block-bits in buf freemap */
#define MEMBUFBLKS          (MEMBUFFREEMAPWORDS * WORDSIZE * CHAR_BIT)
/* maximum number of block-bits in buf freemap (internal limitation) */
#define MEMMAXBUFBLKS       (MEMWORD(1) << PAGESIZELOG2)
/* minimum allocation block size in bigbins */
//#define MEMBIGMINSIZE      (2 * PAGESIZE)

#define MEMBUFSMALLBLKSHIFT (PAGESIZELOG2 - 1)
#define MEMBUFSMALLMAPSHIFT 18
#define MEMBUFMIDMAPSHIFT   20
#define MEMBUFBIGMAPSHIFT   23
#define MEMBUFHUGEMAPSHIFT  26

struct membkt {
#if (MEMLFDEQ)
    struct lfdeq   list;
#else
    struct membuf *list;        // bi-directional list of bufs + lock-bit
#endif
    MEMWORD_T      slot;        // bucket slot #
    MEMWORD_T      nbuf;        // number of bufs in list
    uint8_t        _pad[CLSIZE
#if (MEMLFDEQ)
                        - sizeof(struct lfdeq)
#else
                        - sizeof(struct membuf *)
#endif
                        - 2 * sizeof(MEMWORD_T)];
};

/*
 * buf structure for allocating runs of pages; crafted to fit in a cacheline
 * - a second cacheline is used for the bitmap; 1-bits denote blocks in use
 * - the actual runs will be prefixed by this structure
 * - allocation shall take place with sbrk() or mmap() (see MEMMAPBIT)
 * - data is a placeholder/marker for the beginning of allocated blocks
 */

#define MEMINITBIT   (1L << 0)
#define MEMNOHEAPBIT (1L << 1)
struct mem {
    struct membkt    smallbin[PTRBITS]; // blocks of 1 << slot
    struct membkt    pagebin[PTRBITS]; // mapped blocks of PAGESIZE * (slot + 1)
    struct membkt    bigbin[PTRBITS]; // mapped blocks of 1 << slot
#if (MEMHASH)
    struct memhash  *hash;      // hash table
    struct memhash  *hashbuf;   // buffer for hash items
#elif (MEMHUGELOCK)
    struct memtabl0 *tab;       // allocation lookup structure
#else
    struct memtab   *tab;       // allocation lookup structure
#endif
    MEMWORD_T        flg;       // memory interface flags
    struct membuf   *heap;      // heap allocations (try sbrk(), then mmap())
    struct membuf   *maps;      // mapped blocks
    unsigned long    prioval;   // locklessinc priority locks
    MEMLK_T          initlk;    // lock for initialising the structure
    MEMLK_T          heaplk;    // lock for sbrk()
};

#define MEMHEAPBIT      (0x01L << (sizeof(MEMWORD_T) * CHAR_BIT - 1))
#define MEMEMPTYBIT     (0x01L << (sizeof(MEMWORD_T) * CHAR_BIT - 2))
#define MEMBUFFLGMASK   (MEMHEAPBIT | MEMEMPTYBIT)
#define MEMBUFTYPESHIFT (sizeof(MEMWORD_T) * CHAR_BIT - 4)
#define MEMBUFTYPEMASK  (0x03L << MEMBUFTYPESHIFT)
#define MEMBUFNBLKBITS  10
#if (PTRBITS == 64)
#define MEMBUFSLOTBITS  0x3f
//#define MEMBUFSLOTSHIFT (sizeof(MEMWORD_T) * CHAR_BIT - 10)
#elif (PTRBITS == 32)
#define MEMBUFSLOTBITS  0x1f
//#define MEMBUFSLOTSHIFT (sizeof(MEMWORD_T) * CHAR_BIT - 9)
#endif
#define MEMBUFSLOTSHIFT (2 * MEMBUFNBLKBITS)
#define MEMBUFSLOTMASK  (MEMBUFSLOTBITS << MEMBUFSLOTSHIFT)
#define MEMBUFNBLKMASK  ((MEMWORD(1) << MEMBUFNBLKBITS) - 1)
#define MEMBUFNFREEMASK (MEMBUFNBLKMASK << MEMBUFNBLKBITS)
#define memsetbufflg(buf, flg) ((buf)->info |= (flg))
#define memsetbufnblk(buf, n)                                           \
    ((buf)->info = ((buf)->info & ~MEMBUFNBLKMASK) | (n))
#define memsetbufnfree(buf, n)                                          \
    ((buf)->info = ((buf)->info & ~MEMBUFNFREEMASK) | ((n) << MEMBUFNBLKBITS))
#define memsetbuftype(buf, t)                                           \
    ((buf)->info = ((buf)->info & ~MEMBUFTYPEMASK)                      \
     | ((t) << MEMBUFTYPESHIFT))
#define memsetbufslot(buf, slot)                                        \
    ((buf->info) = ((buf)->info & ~MEMBUFSLOTMASK)                      \
     | ((slot) << MEMBUFSLOTSHIFT))
#define memgetbufflg(buf, flg) ((buf)->info & (flg))
#define memgetbufnblk(buf)                                              \
    ((buf)->info & MEMBUFNBLKMASK)
#define memgetbufnfree(buf)                                             \
    (((buf)->info >> MEMBUFNBLKBITS) & MEMBUFNBLKMASK)
#define memgetbuftype(buf)                                              \
    (((buf)->info >> MEMBUFTYPESHIFT) & MEMBUFTYPEBITS)
#define memgetbufslot(buf)                                              \
    (((buf)->info >> MEMBUFSLOTSHIFT) & MEMBUFSLOTBITS)
struct membuf {
    MEMUWORD_T     info;        // flag-bits + lock-bit + # of total & free blks
    struct membuf *heap;        // previous buf in heap for bufs from sbrk()
    struct membuf *prev;        // previous buf in chain
    struct membuf *next;        // next buf in chain
    struct membkt *bkt;         // pointer to parent bucket
//    MEMWORD_T      slot;        // bucket slot #
    MEMWORD_T      size;        // buffer bookkeeping + allocation blocks
    MEMPTR_T       base;        // base address
    MEMPTR_T      *ptrtab;      // unaligned base pointers for aligned blocks
    /* note: the first bit in freemap is reserved (unused) */
    MEMWORD_T      freemap[MEMBUFFREEMAPWORDS];
};

#if (MEMHASH)

struct memhash {
    struct memhash     *chain;
    volatile MEMWORD_T  nref;
    MEMADR_T            adr;
    MEMADR_T            val;
};

#else

#if (MEMHUGELOCK)
struct memtabl0 {
    MEMLK_T        lk;
    struct memtab *tab;
};
#endif

/* toplevel lookup table item */
struct memtab {
    struct memtab *tab;
};

/* lookup table structure for upper levels */
#if 0
struct memitem {
#if (MEMTABNREF)
    volatile long  nref;
#endif
    void          *tab;
};
#endif

struct memitem {
    volatile long nref;
    MEMADR_T      val;
};

#endif

/*
 * we'll have 2 or 3 levels of these + a level of MEMADR_T values for lookups
 * under the toplevel table
 */
/* type-bits for the final-level table pointers */
#define MEMSMALLBUF    0x00
#define MEMPAGEBUF     0x01
#define MEMBIGBUF      0x02
#define MEMBUFTYPES    3
#define MEMBUFTYPEBITS 0x03

/*
 * NOTE: the arenas are mmap()'d as PAGESIZE-allocations so there's going
 * to be some room in the end for arbitrary data
 */
#define MEMARNSIZE rounduppow2(sizeof(struct memarn), PAGESIZE)
struct memarn {
    struct membkt smallbin[PTRBITS]; // blocks of size 1 << slot
    struct membkt pagebin[PTRBITS];  // mapped blocks of PAGESIZE * (slot + 1)
/* possible auxiliary data here; arena is of PAGESIZE */
};

/* mark the first block of buf as allocated */
#define _memfillmap0(ptr, ofs, mask)                                    \
    ((ptr)[(ofs)] = (mask) & ~MEMWORD(0x01),                            \
     (ptr)[(ofs) + 1] = (mask),                                         \
     (ptr)[(ofs) + 2] = (mask),                                         \
     (ptr)[(ofs) + 3] = (mask))

#define _memfillmap(ptr, ofs, mask)                                     \
    ((ptr)[(ofs)] = (mask),                                             \
     (ptr)[(ofs) + 1] = (mask),                                         \
     (ptr)[(ofs) + 2] = (mask),                                         \
     (ptr)[(ofs) + 3] = (mask))

static __inline__ void
membufinitfree(struct membuf *buf, MEMWORD_T nblk)
{
    MEMWORD_T  bits = ~MEMWORD(0);      // all 1-bits
    MEMWORD_T *ptr = buf->freemap;

#if (MEMBUFFREEMAPWORDS >= 4)
    _memfillmap0(ptr, 0, bits);
#elif (MEMBUFFREEMAPWORDS >= 8)
    _memfillmap(ptr, 4, bits);
#elif (MEMBUFFREEWORD == 16)
    _memfillmap(ptr, 8, bits);
    _memfillmap(ptr, 12, bits);
#else
    memset(buf->freemap, 0xff, sizeof(buf->freemap));
#endif
    memsetbufnblk(buf, nblk);

    return;
}

/*
 * find and clear the lowest 1-bit (free block) in buf->freemap
 * - caller has to lock the buf; memlkbit(&buf->flg, MEMLKBIT);
 * - return index or 0 if not found (bit #0 indicates buf header)
 * - the routine is bitorder-agnostic... =)
 */
static __inline__ MEMWORD_T
membufgetfree(struct membuf *buf)
{
    MEMWORD_T  nblk = memgetbufnblk(buf);
    MEMWORD_T *map = buf->freemap;
    MEMWORD_T  ndx = 0;
    MEMWORD_T  word;
    MEMWORD_T  mask;
    MEMWORD_T  res;

    do {
        word = *map;
        if (word) {                     // skip 0-words
            res = tzerol(word);         // count trailing zeroes
            ndx += res;                 // add to ndx
            if (ndx < nblk) {
                mask = 1L << res;       // create bit
                mask = ~mask;           // invert for mask
                word &= mask;           // mask the bit out
                *map = word;            // update

                return ndx;             // return index of first 1-bit
            }

            return -1;                  // 1-bit not found
        }
        map++;                          // try next word in freemap
        ndx += WORDSIZE * CHAR_BIT;
    } while (ndx < nblk);

    return -1;                          // 1-bit not found
}

#define memalignptr(ptr, pow2)                                          \
    ((void *)rounduppow2((uintptr_t)(ptr), (pow2)))

/*
 * for 32-bit pointers, we can use a flat lookup table for bookkeeping pointers
 * - for bigger pointers, we use a multilevel table
 */
#if (MEMHASH)
#define MEMHASHBITS   20
#define MEMHASHITEMS  (MEMUWORD(1) << MEMHASHBITS)
#elif (PTRBITS > 32)
#define MEMADRSHIFT   PAGESIZELOG2
#define MEMADRBITS    (ADRBITS - MEMADRSHIFT)
#define MEMLVL1BITS   (MEMADRBITS - 3 * MEMLVLBITS)
#define MEMLVLBITS    12
#define MEMLVL1ITEMS  (MEMWORD(1) << MEMLVL1BITS)
#define MEMLVLITEMS   (MEMWORD(1) << MEMLVLBITS)
#define MEMLVL1MASK   ((MEMWORD(1) << MEMLVL1BITS) - 1)
#define MEMLVLMASK    ((MEMWORD(1) << MEMLVLBITS) - 1)
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
#if 0
#define memgetkeybits(p, k1, k2, k3, k4)                                \
    do {                                                                \
        MEMADR_T _p1 = (MEMADR_T)(p) >> MEMADRSHIFT;                    \
        MEMADR_T _p2 = (MEMADR_T)(p) >> (MEMADRSHIFT + MEMLVLBITS);     \
                                                                        \
        (k1) = _p1 & MEMLVLMASK;                                        \
        (k2) = _p2 & MEMLVLMASK;                                        \
        _p1 >>= 2 * MEMLVLBITS;                                         \
        _p2 >>= 2 * MEMLVLBITS;                                         \
        (k3) = _p1 & MEMLVLMASK;                                        \
        (k4) = _p2 & MEMLVL1MASK;                                       \
    } while (0)
#endif
#endif

/*
 * allocation headers
 * - 3 allocation classes:
 *   - small; block size is 1 << slot
 *   - page; block size is PAGESIZE * (slot + 1)
 *   - big; block size is 1 << slot
 */
#define membufhdrsize()       (sizeof(struct membuf))
#define membufptrtabsize()    (MEMBUFBLKS * sizeof(MEMPTR_T))
//#define membufslot(buf)       (memgetbufslot(buf))
#define membufblkofs()                                                  \
    (rounduppow2(membufhdrsize() + membufptrtabsize(), PAGESIZE))
#define memusesmallbuf(sz)    ((sz) <= (MEMUWORD(1) << MEMBUFSMALLBLKSHIFT))
#define memusepagebuf(sz)     ((sz) <= (PAGESIZE * PTRBITS))
/* allocations of PAGESIZE << slot */
#define memsmallbufsize(slot)                                           \
    (rounduppow2(membufblkofs() + (MEMBUFBLKS << (slot)),               \
                 PAGESIZE))
#define mempagebufsize(slot, nblk)                                      \
    (rounduppow2(membufblkofs() + (PAGESIZE + PAGESIZE * (slot)) * (nblk), \
                 PAGESIZE))
#define membigbufsize(slot, nblk)                                       \
    (rounduppow2(membufblkofs() + (MEMUWORD(1) << (slot)) * (nblk),     \
                 PAGESIZE))
#define membufnblk(slot, type)                                          \
    (((type) == MEMSMALLBUF)                                            \
     ? (MEMBUFBLKS)                                                     \
     : (((slot) <= MEMBUFMIDMAPSHIFT)                                   \
        ? 4                                                             \
        : (((slot) <= MEMBUFBIGMAPSHIFT)                                \
           ? 2                                                          \
           : 1)))
#define membufnarn(slot, type)                                          \
    (2)
#define membufnglob(slot, type)                                         \
    (4)

#define membufblkadr(buf, ndx)                                          \
    ((buf)->base + ((ndx) << memgetbufslot(buf)))
#define membufblkid(buf, ptr)                                           \
    ((((MEMPTR_T)(ptr) - (buf)->base) >> memgetbufslot(buf)) \
     & ((MEMUWORD(1) << memgetbufslot(buf)) - 1))
#define membufblksize(buf)                                              \
    ((memgetbuftype(buf) != MEMPAGEBUF)                                 \
     ? (MEMUWORD(1) << memgetbufslot(buf))                              \
     : (PAGESIZE + PAGESIZE * memgetbufslot(buf)))
#define membufgetptr(buf, ptr)                                          \
    ((buf)->ptrtab[membufblkid(buf, ptr)])
#define membufsetptr(buf, ptr, adr)                                     \
    ((buf)->ptrtab[membufblkid(buf, ptr)] = (adr))
#define membufpageadr(buf, ndx)                                         \
    ((buf)->base + (ndx) * (PAGESIZE + PAGESIZE * memgetbufslot(buf)))
#define membufgetpage(buf, ndx)                                         \
    ((buf)->ptrtab[(ndx)])
#define membufsetpage(buf, ndx, adr)                                    \
    ((buf)->ptrtab[(ndx)] = (adr))

void            meminit(void);
struct memarn * meminitarn(void);
MEMPTR_T        memgetblk(long slot, long type, size_t align);
void *          memsetbuf(void *ptr, struct membuf *buf, MEMUWORD_T info);
#if (MEMHASH)
MEMADR_T        memfindbuf(void *ptr, MEMWORD_T incr, MEMADR_T *keyret);
#else
MEMADR_T        memfindbuf(void *ptr, long rel);
#endif
void            memputblk(void *ptr, struct membuf *buf, MEMUWORD_T info);
#if (MEMTEST)
long            _memchkptr(struct membuf *buf, MEMPTR_T ptr);
long            _memchkbuf(struct membuf *buf);
#endif

#endif /* __ZERO_MEM_H__ */

