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

#define MEM_LK_NONE 0x01        // don't use locks; single-thread
#define MEM_LK_PRIO 0x02        // priority-based locklessinc.com lock
#define MEM_LK_FMTX 0x04        // anonymous non-recursive mutex
#define MEM_LK_SPIN 0x08        // spinlock

#define MEM_LK_TYPE MEM_LK_SPIN // type of locks to use

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/asm.h>
#include <zero/unix.h>
#include <zero/trix.h>
#if (MEM_LK_TYPE == MEM_LK_PRIO)
#define PRIOLKUSEMMAP
#include <zero/priolk.h>
#endif
#if (MEM_LK_TYPE == MEM_LK_FMTX)
#include <zero/mtx.h>
#endif
#if (MEM_LK_TYPE == MEM_LK_SPIN)
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

#if (MEM_LK_TYPE & MEM_LK_PRIO)
typedef struct priolk MEMLK_T;
#elif (MEM_LK_TYPE & MEM_LK_FMTX)
typedef zerofmtx      MEMLK_T;
#elif (MEM_LK_TYPE & MEM_LK_SPIN)
typedef zerospin      MEMLK_T;
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

//#define MEMPAGEBIT      (MEMUWORD(1) << (PAGESIZELOG2 - 1))
//#define MEMPAGEIDMASK   (MEMPAGEBIT - 1)
#define MEMPAGEINFOMASK ((MEMUWORD(1) << PAGESIZELOG2) - 1)
/* use the low-order bit of the word or pointer to lock data */
#define MEMLKBITID      0
#define MEMLKBIT        (MEMUWORD(1) << MEMLKBITID)
#if (MEMDEBUGDEADLOCK)
#define memlkbitln(ptr)                                                 \
    do {                                                                \
        do {                                                            \
            ;                                                           \
        } while (m_cmpsetbit((m_atomic_t *)&(ptr)->list, MEMLKBITID));  \
        (ptr)->line = __LINE__;                                         \
    } while (0)
#define memrelbitln(ptr) \
    ((ptr)->line = __LINE__,                                            \
     m_cmpclrbit((m_atomic_t *)&(ptr)->list, MEMLKBITID))
#endif
#if (MEMDEBUGLOCK)
#define memlkbit(ptr)                                                   \
    do {                                                                \
        fprintf(stderr, "LK: %s: %d\n", __FILE__, __LINE__);            \
    } while (m_cmpsetbit((m_atomic_t *)ptr, MEMLKBITID))
#define memrelbit(ptr) (fprintf(stderr, "UNLK: %s: %d\n", __FILE__, __LINE__), \
                        m_cmpclrbit((m_atomic_t *)ptr, MEMLKBITID))

#else
#define memtrylkbit(ptr)                                                \
    (!m_cmpsetbit((m_atomic_t *)ptr, MEMLKBITID))
#define memlkbit(ptr)                                                   \
    do {                                                                \
        ;                                                               \
    } while (m_cmpsetbit((m_atomic_t *)ptr, MEMLKBITID))
#define memrelbit(ptr) m_cmpclrbit((m_atomic_t *)ptr, MEMLKBITID)
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
#define memcalcpageslot(sz, slot)                                       \
    do {                                                                \
        MEMUWORD_T _res = sz;                                           \
                                                                        \
        _res--;                                                         \
        _res >>= PAGESIZELOG2;                                          \
        (slot) = _res;                                                  \
    } while (0)

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
#define MEMSMALLSLOTS      (MEMMAXSMALLSHIFT + 1)
/* NOTES
 * -----
 * - all allocations except those from pagebin are power-of-two sizes
 * - pagebin allocations are PAGESIZE * slot
 */
/* minimum allocation block size */
#define MEMMINBLK          (MEMUWORD(1) << MEMALIGNSHIFT)
/* maximum allocation block size */
#define MEMMAXBUFBLK       (MEMUWORD(1) << MEMMAXBUFSLOT)
/* maximum slot # */
#define MEMMAXBUFSLOT      (PTRBITS - 1)
/* number of words in buf freemap */
//#define MEMBUFFREEMAPWORDS  (CLSIZE / WORDSIZE)
#define MEMBUFFREEMAPWORDS 8
/* number of block-bits in buf freemap */
#define MEMBUFBLKS          (MEMBUFFREEMAPWORDS * WORDSIZE * CHAR_BIT)
#if 0
/* maximum number of block-bits in buf freemap (internal limitation) */
#define MEMMAXBUFBLKS       (MEMUWORD(1) << PAGESIZELOG2)
#endif
/* minimum allocation block size in bigbins */
//#define MEMBIGMINSIZE      (2 * PAGESIZE)
#define MEMPAGESLOTS        (MEMUWORD(1) << MEMBUFSLOTBITS)

#define MEMSMALLPAGESLOT    64
#define MEMMIDPAGESLOT      128
#define MEMBIGPAGESLOT      192
//#define MEMSMALLBLKSHIFT    (PAGESIZELOG2 - 1)
#define MEMSMALLMAPSHIFT    20
//#define MEMBUFMIDMAPSHIFT 22
#define MEMBIGMAPSHIFT      24
//#define MEMBUFHUGEMAPSHIFT  26

struct membkt {
#if (MEMLFDEQ)
    struct lfdeq   list;
#else
    struct membuf *list;        // bi-directional list of bufs + lock-bit
#endif
//    MEMWORD_T      slot;        // bucket slot #
#if (MEMDEBUGDEADLOCK)
    MEMWORD_T      line;
#endif
    MEMUWORD_T     nbuf;        // number of bufs in list
#if 0
    uint8_t        _pad[CLSIZE
#if (MEMLFDEQ)
                        - sizeof(struct lfdeq)
#else
                        - sizeof(struct membuf *)
#endif
#if (MEMDEBUGDEADLOCK)
                        - sizeof(MEMWORD_T)
#endif
                        - sizeof(MEMWORD_T)];
#endif
};

/* type-bits for allocation buffers */
#define MEMSMALLBUF      0x00
#define MEMPAGEBUF       0x01
#define MEMBIGBUF        0x02
#define MEMBUFTYPES      3
#define MEMBUFTYPEBITS   2
#define MEMHEAPBIT       (MEMUWORD(1) << MEMBUFFLGSHIFT)
#define MEMBUFFLGMASK    (MEMHEAPBIT)
#define MEMBUFFLGSHIFT   (sizeof(MEMUWORD_T) * CHAR_BIT - 1)
#define MEMBUFFLGBITS    1
#define MEMBUFTYPESHIFT  (sizeof(MEMUWORD_T) * CHAR_BIT - 3)
#define MEMBUFTYPEMASK   (((MEMUWORD(1) << MEMBUFTYPEBITS) - 1)          \
                          << MEMBUFTYPESHIFT)
#define MEMBUFSLOTBITS   8
#define MEMBUFSLOTMASK   (((MEMUWORD(1) << MEMBUFSLOTBITS) - 1)          \
                          << MEMBUFSLOTSHIFT)
#define MEMBUFSLOTSHIFT  (2 * MEMBUFNBLKBITS)
#define MEMBUFNBLKBITS   10
#define MEMBUFNBLKMASK   ((MEMUWORD(1) << MEMBUFNBLKBITS) - 1)
#define MEMBUFNFREEBITS  MEMBUFNBLKBITS
#define MEMBUFNFREEMASK  (MEMBUFNBLKMASK << MEMBUFNFREESHIFT)
#define MEMBUFNFREESHIFT MEMBUFNBLKBITS

#if (MEMBITFIELD)

#define memsetbufflg(buf, f)   ((buf)->info.flg = (f))
#define memclrbufflg(buf, f)   ((buf)->info.flg &= ~(f))
#define memsetbufnblk(buf, n)  ((buf)->info.nblk = (n))
#define memsetbufnfree(buf, n) ((buf)->info.nfree = (n))
#define memsetbuftype(buf, t)  ((buf)->info.type = (t))
#define memsetbufslot(buf, id) ((buf)->info.slot = (id))
#define memgetbufflg(buf)      ((buf)->info.flg)
#define memgetbufnblk(buf)     ((buf)->info.nblk)
#define memgetbufnfree(buf)    ((buf)->info.nfree)
#define memgetbuftype(buf)     ((buf)->info.type)
#define memgetbufslot(buf)     ((buf)->info.slot)

#else

#define memsetbufflg(buf, flg) ((buf)->info |= (flg))
#define memsetbufnblk(buf, n)                                           \
    ((buf)->info |= (n))
#define memsetbuftype(buf, t)                                           \
    ((buf)->info |=  (t) << MEMBUFTYPESHIFT)
#define memsetbufslot(buf, slot)                                        \
    ((buf->info)  |= (slot) << MEMBUFSLOTSHIFT)
#define memsetbufnfree(buf, n)                                          \
    ((buf)->info = ((buf)->info & ~MEMBUFNFREEMASK) | ((n) << MEMBUFNBLKBITS))
#define memgetbufflg(buf)                                               \
    ((buf)->info & (flg))
#define memgetbufnblk(buf)                                              \
    ((buf)->info & MEMBUFNBLKMASK)
#define memgetbufnfree(buf)                                             \
    (((buf)->info >> MEMBUFNFREESHIFT) & ((MEMUWORD(1) << MEMBUFNFREEBITS) - 1))
#define memgetbuftype(buf)                                              \
    (((buf)->info >> MEMBUFTYPESHIFT) & ((MEMUWORD(1) << MEMBUFTYPEBITS) - 1))
#define memgetbufslot(buf)                                              \
    (((buf)->info >> MEMBUFSLOTSHIFT) & ((MEMUWORD(1) << MEMBUFSLOTBITS) - 1))

#endif

struct membufvals {
    MEMUWORD_T *nblk[MEMBUFTYPES];
    MEMUWORD_T *ntls[MEMBUFTYPES];
    MEMUWORD_T *nglob[MEMBUFTYPES];
};

#define MEMINITBIT   (1L << 0)
#define MEMNOHEAPBIT (1L << 1)
struct mem {
    struct membkt       smallbin[MEMSMALLSLOTS]; // blocks of 1 << slot
    struct membkt       bigbin[PTRBITS];         // mapped blocks of 1 << slot
    struct membkt       pagebin[MEMPAGESLOTS];   // maps of PAGESIZE * slot
    struct membufvals   bufvals;
#if (MEMMULTITAB)
    struct memtabl0    *tab;     // allocation lookup structure
#elif (MEMNEWHASH)
    struct memhashlist *hash;    // hash table
    struct memhash     *hashbuf; // buffer for hash items
#endif
    MEMWORD_T           flg;     // memory interface flags
    struct membuf      *heap;    // heap allocations (try sbrk(), then mmap())
    struct membuf      *maps;    // mapped blocks
#if (MEM_LK_TYPE == MEM_LK_PRIO)
    MEMLK_T             priolk;
    unsigned long       prioval; // locklessinc priority locks
#endif
    MEMLK_T             heaplk;  // lock for sbrk()
    MEMLK_T             initlk;  // lock for initialisation
};

#if (MEMBITFIELD)
struct membufinfo {
    unsigned int nblk  : MEMBUFNBLKBITS;
    unsigned int nfree : MEMBUFNBLKBITS;
    unsigned int slot  : MEMBUFSLOTBITS;
    unsigned int type  : MEMBUFTYPEBITS;
    unsigned int flg   : MEMBUFFLGBITS;
};
#endif

/*
 * buf structure for allocating runs of pages; crafted to fit in 8 words
 * (typical cacheline size)
 * - a second cacheline is used for the bitmap; 1-bits denote free blocks
 * - the actual buffers will be prefixed by this structure
 * - allocation shall take place with sbrk() or mmap()
 */

struct membuf {
#if (MEMBITFIELD)
    struct membufinfo       info;
#else
    MEMUWORD_T              info; // flags + lock-bit + # of total & free blks
#endif
    struct membuf          *heap; // previous buf in heap for bufs from sbrk()
    struct membuf          *prev; // previous buf in chain
    struct membuf          *next; // next buf in chain
    volatile struct membkt *bkt;  // pointer to parent bucket
//    MEMWORD_T      slot;        // bucket slot #
    MEMUWORD_T              size; // buffer bookkeeping + allocation blocks
    MEMPTR_T                base; // base address for allocations
    MEMPTR_T               *ptrtab; // original pointers for aligned blocks
    MEMUWORD_T              freemap[MEMBUFFREEMAPWORDS];
};

#if (MEMMULTITAB)

/* toplevel lookup table item */
struct memtabl0 {
    MEMLK_T        lk;
    struct memtab *tab;
};

/* lookup table item */
struct memtab {
    struct memtab *tab;
};

struct memitem {
    MEMADR_T val;
};

#elif (MEMNEWHASH)

struct memhashlist {
#if (MEMHASHLOCK)
    MEMLK_T         lk;
#endif
    struct memhash *chain;
};

#define MEMHASHDEL (-1)
#define MEMHASHCHK (0)
#define MEMHASHADD (1)

#if (WORDSIZE == 4)
#define memhashptr(page)                                                \
    (tmhash32((unsigned long)page))
#elif (WORDSIZE == 8)
#define memhashptr(page)                                                \
    (tmhash64((unsigned long)page))
#endif
#define MEMHASHNOTFOUND  0
#define MEMHASHFOUND     (~(MEMADR_T)0)

/*
 * - we have a 4-word header; adding total of 52 words as 13 hash-table entries
 *   lets us cache-color the table by adding a modulo-9 value to the pointer
 */
#define MEMHASHBITS     20
#define MEMHASHITEMS    (1U << MEMHASHBITS)
#if (MEMBIGHASHTAB)
#define MEMHASHSIZE     (256 * WORDSIZE)
#define MEMHASHTABITEMS 32      // allow a bit of table-address randomization
#else
#define MEMHASHSIZE     (128 * WORDSIZE)
#define MEMHASHTABITEMS 24
#endif
#define memhashsize()   MEMHASHSIZE

struct memhashitem {
    MEMUWORD_T nref;            // reference count for the page
    MEMUWORD_T nact;            // number of inserts, finds, and deletes
    MEMADR_T   page;            // page address
    MEMADR_T   val;             // stored value
};

struct memhash {
    struct memhash     *chain;  // next array in this chain
    MEMUWORD_T          ntab;   // number of occupied slots in this table
    struct memhashitem *tab;    // pointer to the item table
    struct memhashlist *list;   // pointer for head of list
    MEMUWORD_T          data;   // base address for generating table address
};

#endif

/*
 * NOTE: the arenas are mmap()'d as PAGESIZE-allocations so there's going
 * to be some room in the end for arbitrary data
 */
#define memtlssize() rounduppow2(sizeof(struct memtls), PAGESIZE)
struct memtls {
    struct membkt     smallbin[PTRBITS]; // blocks of size 1 << slot
    struct membkt     pagebin[MEMPAGESLOTS]; // maps of PAGESIZE * slot
#if (MEM_LK_TYPE & MEM_LK_PRIO)
    struct priolkdata priolkdata;
#endif
    MEMUWORD_T        flg;
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
    MEMUWORD_T  bits = ~MEMUWORD(0);      // all 1-bits
    MEMUWORD_T *ptr = buf->freemap;

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
    MEMUWORD_T  nblk = memgetbufnblk(buf);
    MEMUWORD_T *map = buf->freemap;
    MEMUWORD_T  ndx = 0;
    MEMUWORD_T  word;
    MEMUWORD_T  mask;
    MEMUWORD_T  res;

    do {
        word = *map;
        if (word) {                             // skip 0-words
            res = tzerol(word);                 // count trailing zeroes
            ndx += res;                         // add to ndx
            if (ndx < nblk) {
                mask = MEMUWORD(1) << res;      // create bit
                mask = ~mask;                   // invert for mask
                word &= mask;                   // mask the bit out
                *map = word;                    // update

                return ndx;                     // return index of first 1-bit
            }

            return -1;                          // 1-bit not found
        }
        ndx += sizeof(MEMUWORD_T) * CHAR_BIT;
        map++;                                  // try next word in freemap
    } while (ndx < nblk);

    return -1;                                  // 1-bit not found
}

#define memalignptr(ptr, pow2)                                          \
    ((MEMPTR_T)rounduppow2((uintptr_t)(ptr), (pow2)))
#define memadjptr(ptr, sz)                                              \
    (&((MEMPTR_T)(ptr))[sz])

/* this routine came from the lockless allocator, courtesy of locklessinc.com */
static __inline__ uint32_t
memrandptr(MEMPTR_T ptr)
{
    uint64_t val = (uint64_t)ptr;

    val *= UINT64_C(7319936632422683443);
    val ^= val >> 32;
    val *= UINT64_C(7319936632422683443);
    val ^= val >> 32;

    return (uint32_t)val;
}

static __inline__ MEMPTR_T
memgenptr(MEMPTR_T ptr, MEMUWORD_T blksz, MEMUWORD_T size)
{
    uint64_t xtra = blksz - size;
    uint64_t rnd = memrandptr(ptr);
    uint64_t ofs = xtra * rnd;
    MEMPTR_T ret;

    ofs >>= 32;
    ofs = rounddownpow2(ofs, MEMMINALIGN);
    ret = memadjptr(ptr, ofs);

    return ret;
}

static __inline__ MEMPTR_T
memgenptrcl(MEMPTR_T ptr, MEMUWORD_T blksz, MEMUWORD_T size)
{
    uint64_t xtra = blksz - size;
    uint64_t rnd = memrandptr(ptr);
    uint64_t ofs = (xtra * rnd) >> 32;
    MEMPTR_T ret;

    ofs = rounddownpow2(ofs, CLSIZE);
    ret = memadjptr(ptr, ofs);

    return ret;
}

/* compute adr + adr % 9 (# of cachelines in offset, aligned to cl boundary) */
static __inline__ MEMUWORD_T *
memgentlsadr(MEMPTR_T adr)
{
    /* division by 9 */
    MEMADR_T res = (MEMADR_T)adr;
    MEMADR_T q;
    MEMADR_T r;
    MEMADR_T div9;
    MEMADR_T dec;

    res >>= 16;
    /* divide by 9 */
    q = res - (res >> 3);
    q = q + (q >> 6);
    q = q + (q >> 12) + (q >> 24);
    q = q >> 3;
    r = res - q * 9;
    div9 = q + ((r + 7) >> 4);
    /* calculate res -= res/9 * 9 i.e. res % 9 (max 8) */
    dec = div9 * 9;
    res -= dec;
    /* scale to 0..64 (machine words) */
    res <<= 3;
    /* align to cacheline */
    res &= ~(CLSIZE - 1);
    /* add to original pointer */
    adr += res;
    /* align to machine word boundary */

    return adr;
}


/* compute adr + adr % 9 (# of words in offset, aligned to word boundary) */
static __inline__ MEMUWORD_T *
memgenhashtabadr(MEMUWORD_T *adr)
{
    /* division by 9 */
    MEMADR_T res = (MEMADR_T)adr;
    MEMADR_T q;
    MEMADR_T r;
    MEMADR_T div9;
    MEMADR_T dec;

    /* shift out some [mostly-aligned] low bits */
    res >>= 16;
    /* divide by 9 */
    q = res - (res >> 3);
    q = q + (q >> 6);
    q = q + (q >> 12) + (q >> 24);
    q = q >> 3;
    r = res - q * 9;
    div9 = q + ((r + 7) >> 4);
    /* calculate res -= res/9 * 9 i.e. res % 9 (max 8) */
    dec = div9 * 9;
    res -= dec;
#if (MEMBIGHASHTAB)
    /* scale res to 0..32 (machine words) */
    res <<= 2;
#else
    /* scale res to 0..16 (machine words) */
    res <<= 1;
#endif
    /* add to original pointer */
    adr += res;
    /* align to machine word boundary */

    return adr;
}

/*
 * for 32-bit pointers, we can use a flat lookup table for bookkeeping pointers
 * - for bigger pointers, we use a hash (or multilevel) table
 */
#if (MEMHASH) && !defined(MEMHASHITEMS)
#define MEMHASHBITS   20
#define MEMHASHITEMS  (MEMUWORD(1) << MEMHASHBITS)
#elif (MEMMULTITAB)
#if (PTRBITS > 32)
#define MEMADRBITS    (ADRBITS - PAGESIZELOG2)
#define MEMADRSHIFT   (PAGESIZELOG2)
#define MEMLVL1BITS   (MEMADRBITS - 3 * MEMLVLBITS)
#define MEMLVLBITS    10
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
#endif /* PTRBITS > 32 */
#endif /* MEMHASH && !defined(MEMHASHITEMS) */

/*
 * allocation headers
 * - 3 allocation classes:
 *   - small; block size is 1 << slot
 *   - page; block size is PAGESIZE * slot
 *   - big; block size is 1 << slot
 */
#define membufhdrsize()        (sizeof(struct membuf))
#define membufptrtabsize(nblk) (MEMBUFBLKS * sizeof(MEMPTR_T))
//#define membufslot(buf)       (memgetbufslot(buf))
#define membufblkofs(nblk)                                              \
    (rounduppow2(membufhdrsize() + membufptrtabsize(nblk), PAGESIZE))
#define memusesmallbuf(sz)     ((sz) <= (MEMUWORD(1) << MEMMAXSMALLSHIFT))
#define memusepagebuf(sz)      ((sz) <= (PAGESIZE * MEMPAGESLOTS))
/* allocations of PAGESIZE * slot */
#define memsmallbufsize(slot)                                           \
    (rounduppow2(membufblkofs() + (MEMBUFBLKS << (slot)),               \
                 PAGESIZE))
#define mempagebufsize(slot, nblk)                                      \
    (rounduppow2(membufblkofs()                                         \
                 + (MEMUWORD(PAGESIZE) + MEMUWORD(PAGESIZE) * (slot))   \
                 * (nblk),                                              \
                 PAGESIZE))
#define membigbufsize(slot, nblk)                                       \
    (rounduppow2(membufblkofs() + (MEMUWORD(1) << (slot)) * (nblk),     \
                 PAGESIZE))
#if 0
#define memnbufblk(slot, type)                                          \
    (((type) == MEMSMALLBUF)                                            \
     ? (MEMBUFBLKS)                                                     \
     : (((slot) <= MEMBUFMIDMAPSHIFT)                                   \
        ? 4                                                             \
        : (((slot) <= MEMBUFBIGMAPSHIFT)                                \
           ? 2                                                          \
           : 1)))
#endif
#define memnbufblk(slot, type)                                          \
    (((type) == MEMSMALLBUF)                                            \
     ? (MEMBUFBLKS)                                                     \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? 4                                                          \
           : (((slot) <= MEMMIDPAGESLOT)                                \
              ? 2                                                       \
              : 1))                                                     \
        : (((slot) <= MEMSMALLMAPSHIFT)                                 \
           ? 4                                                          \
           : (((slot) <= MEMBIGMAPSHIFT)                                \
              ? 2                                                       \
              : 1))))
#define memnbuftls(slot, type)                                          \
    (((type) == MEMSMALLBUF)                                            \
     ? 2                                                                \
     : 1)
#define memnbufglob(slot, type)                                         \
    (((type) == MEMSMALLBUF)                                            \
     ? 8                                                                \
     : (((type) == MEMPAGEBUF)                                          \
        ? 4                                                             \
        : 2))

#define membufblkadr(buf, ndx)                                          \
    ((buf)->base + ((ndx) << memgetbufslot(buf)))
#define membufblkid(buf, ptr)                                           \
    (((MEMPTR_T)(ptr) - (buf)->base) >> memgetbufslot(buf))
#define membufpageadr(buf, ndx)                                         \
    ((buf)->base + (ndx) * (MEMUWORD(PAGESIZE) + MEMUWORD(PAGESIZE) * memgetbufslot(buf)))
#define membufblksize(buf, type, slot)                                  \
    ((type != MEMPAGEBUF)                                               \
     ? (MEMUWORD(1) << (slot))                                          \
     : (MEMUWORD(PAGESIZE) + MEMUWORD(PAGESIZE) * (slot)))
#define membufgetptr(buf, ptr)                                          \
    ((buf)->ptrtab[membufblkid(buf, ptr)])
#define membufsetptr(buf, ptr, adr)                                     \
    ((buf)->ptrtab[membufblkid(buf, ptr)] = (adr))
#define membufgetpageadr(buf, ndx)                                      \
    ((buf)->ptrtab[(ndx)])
#define membufsetpageadr(buf, ndx, adr)                                 \
    ((buf)->ptrtab[(ndx)] = (adr))

#define memgetnbufblk(slot, type)                                       \
    (g_mem.bufvals.nblk[type][slot])
#define memgetnbuftls(slot, type)                                       \
    (g_mem.bufvals.ntls[type][slot])
#define memgetnbufglob(slot, type)                                      \
    (g_mem.bufvals.nglob[type][slot])

void            meminit(void);
struct memtls * meminittls(void);
MEMPTR_T        memgetblk(MEMUWORD_T slot, MEMUWORD_T type,
                          MEMUWORD_T size, MEMUWORD_T align);
MEMPTR_T        memsetbuf(MEMPTR_T ptr, struct membuf *buf, MEMUWORD_T id);
#if (MEMMULTITAB)
struct membuf * memfindbuf(void *ptr, MEMWORD_T incr);
#elif (MEMNEWHASH)
MEMADR_T        membufop(MEMPTR_T ptr, MEMWORD_T op, struct membuf *buf,
                         MEMUWORD_T id);
#elif (MEMHASH)
MEMADR_T        memfindbuf(void *ptr, MEMWORD_T incr, MEMADR_T *keyret);
#else
struct membuf * memfindbuf(void *ptr, long rel);
#endif
void            memputblk(void *ptr, struct membuf *buf, MEMUWORD_T id);
#if (MEMTEST)
void            memprintbuf(struct membuf *buf, const char *func);
long            _memchkptr(struct membuf *buf, MEMPTR_T ptr);
long            _memchkbuf(struct membuf *buf, MEMUWORD_T slot, MEMUWORD_T type,
                           MEMUWORD_T nblk, MEMUWORD_T flg, const char *func);
#endif

#if (MEMSTAT)
void                 memprintstat(void);
struct memstat {
    MEMUWORD_T nbsmall;
    MEMUWORD_T nbpage;
    MEMUWORD_T nbbig;
    MEMUWORD_T nbheap;
    MEMUWORD_T nbmap;
    MEMUWORD_T nbbook;
    MEMUWORD_T nbhash;
};
#endif

#endif /* __ZERO_MEM_H__ */

