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
#include <stdio.h>
#define crash(cond)                                                     \
    (!(cond)                                                            \
     ? ((void)0)                                                        \
     : (fprintf(stderr,                                                 \
                "Assertion failed: %s, file %s, function %s, line %d\n", \
                #cond,                                                  \
                __FILE__,                                               \
                __func__,                                               \
                __LINE__),                                              \
        *(volatile uint8_t *)NULL = 0x00))
#endif

/* generic memory manager definitions for libzero */

#define MEM_LK_NONE 0x01        // don't use locks; single-thread
#define MEM_LK_PRIO 0x02        // priority-based locklessinc.com lock
#define MEM_LK_FMTX 0x04        // anonymous non-recursive mutex
#define MEM_LK_SPIN 0x08        // spinlock

#define MEM_LK_TYPE MEM_LK_PRIO // type of locks to use

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/asm.h>
#include <zero/unix.h>
#include <zero/trix.h>
#include <zero/spin.h>
#if (MEM_LK_TYPE == MEM_LK_PRIO)
#define PRIOLKUSEMMAP
#include <zero/priolk.h>
#elif (MEM_LK_TYPE == MEM_LK_FMTX)
#include <zero/mtx.h>
#endif

/* types */

#if (WORDSIZE == 4)
#define MEMWORDSIZESHIFT 2
typedef int32_t   MEMWORD_T;    // machine word
typedef uint32_t  MEMUWORD_T;   // unsigned machine word
#elif (WORDSIZE == 8)
#define MEMWORDSIZESHIFT 3
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
#endif

struct membuf;

void                     meminit(void);
MEMPTR_T                 memgetblk(MEMWORD_T slot, MEMWORD_T type,
                                   MEMWORD_T size, MEMWORD_T align);
MEMPTR_T                 memsetbuf(MEMPTR_T ptr, struct membuf *buf,
                                   MEMWORD_T id);
#if (MEMMULTITAB)
struct membuf          * memfindbuf(void *ptr, MEMWORD_T incr);
#elif (MEMNEWHASH)
MEMADR_T                 membufop(MEMPTR_T ptr, MEMWORD_T op,
                                  struct membuf *buf,
                                  MEMWORD_T id);
#elif (MEMHASH)
MEMADR_T                 memfindbuf(void *ptr, MEMWORD_T incr,
                                    MEMADR_T *keyret);
#else
struct membuf          * memfindbuf(void *ptr, long rel);
#endif
void                     memrelblk(struct membuf *buf, MEMWORD_T id);
#if (MEMTEST)
void                     memprintbuf(struct membuf *buf, const char *func);
long                     _memchkptr(struct membuf *buf, MEMPTR_T ptr);
long                     _memchkbuf(struct membuf *buf, MEMWORD_T type,
                                    MEMWORD_T nblk, MEMUWORD_T flg,
                                    const char *func);
#endif
#if defined(MEMBUFSTACK) && (MEMBUFSTACK)
void                     memprintbufstk(struct membuf *buf, const char *msg);
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
#define memgetlk(lp) priolkget(lp)
#define memrellk(lp) priolkrel(lp)
#elif (MEM_LK_TYPE == MEM_LK_FMTX)
#define memgetlk(lp) fmtxlk(lp)
#define memrellk(lp) fmtxunlk(lp)
#elif (MEM_LK_TYPE == MEM_LK_SPIN)
#define memgetlk(lp) spinlk(lp)
#define memrellk(lp) spinunlk(lp)
#endif

//#define MEMPAGEBIT      (MEMWORD(1) << (PAGESIZELOG2 - 1))
//#define MEMPAGEIDMASK   (MEMPAGEBIT - 1)
#define MEMPAGEIDMASK   ((MEMWORD(1) << PAGESIZELOG2) - 1)
/* use the low-order bit of the word or pointer to lock data */
#define MEMLKBITID      0
#define MEMLKBIT        (MEMWORD(1) << MEMLKBITID)

#define memtrylkbit(ptr)                                                \
    (!m_cmpsetbit((m_atomic_t *)ptr, MEMLKBITID))
#define memlkbit(ptr)                                                   \
    do {                                                                \
        if (!m_cmpsetbit((m_atomic_t *)ptr, MEMLKBITID)) {              \
            m_waitint();                                                \
        } else {                                                        \
                                                                        \
            break;                                                      \
        }                                                               \
    } while (1)
//#define memrelbit(ptr) m_cmpclrbit((m_atomic_t *)ptr, MEMLKBITID)
#define memrelbit(ptr) m_clrbit((m_atomic_t *)ptr, MEMLKBITID)

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
        MEMWORD_T _res = sz;                                            \
                                                                        \
        _res--;                                                         \
        _res >>= PAGESIZELOG2;                                          \
        (slot) = _res;                                                  \
    } while (0)

#define MEMSMALLTLSLIM      (32 * 1024 * 1024)
#define MEMPAGETLSLIM       (128 * 1024 * 1024)
#define MEMSMALLGLOBLIM     (256 * 1024 * 1024)
#define MEMPAGEGLOBLIM      (512 * 1024 * 1024)
#define MEMBIGGLOBLIM       (1024 * 1024 * 1024)

#define MEMMINALIGN         CLSIZE
#if !defined(MEMMINALIGN)
/* determine minimal required alignment for blocks */
#if defined(__BIGGEST_ALIGNMENT__)
//#define MEMMINALIGN         max(__BIGGEST_ALIGNMENT__, 2 * PTRSIZE)
#define MEMMINALIGN         __BIGGEST_ALIGNMENT__
#else
//#define MEMMINALIGN         (2 * PTRSIZE) // allow for dual-word tagged pointers
#define MEMMINALIGN         PTRSIZE
#endif
#endif
#if (MEMMINALIGN == 8)
#define MEMALIGNSHIFT       3
#elif (MEMMINALIGN == 16)
#define MEMALIGNSHIFT       4
#elif (MEMMINALIGN == 32)
#define MEMALIGNSHIFT       5
#elif (MEMMINALIGN == 64)
#define MEMALIGNSHIFT       6
#endif
/* maximum small buf size (MEMBUFMAXBLKS << MEMMAXSMALLSLOT) + bookkeeping */
#define MEMSMALLSLOTS       rounduppow2(MEMMAXSMALLSLOT, 8)
#if (MEMBIGPAGES)
//#define MEMMAXSMALLSLOT     (PAGESIZELOG2 - 1)
#else
//#define MEMMAXSMALLSLOT     (PAGESIZELOG2 - 1)
#define MEMMAXSMALLSLOT     PAGESIZELOG2
#endif
/* NOTES
 * -----
 * - all allocations except those from pagebin are power-of-two sizes
 * - pagebin allocations are PAGESIZE * slot
 */
/* minimum allocation block size */
#define MEMMINBLK           (MEMWORD(1) << MEMALIGNSHIFT)
/* maximum allocation block size */
#define MEMMAXBIGBLK        (MEMWORD(1) << MEMMAXBIGSLOT)
/* maximum slot # */
#define MEMBIGSLOTS         PTRBITS
#define MEMMAXBIGSLOT       (MEMBIGSLOTS - 1)
/* number of words in buf freemap */
#define MEMBUFMAXBITS       256
#define MEMBUFBITMAPWORDS                                               \
    rounduppow2(MEMBUFMAXBITS / (WORDSIZE * CHAR_BIT), 8)
        
//#define MEMBUFBITMAPWORDS   32
//#define MEMSLABSHIFT        18
#define MEMSLABSHIFT        21
#define MEMBUFMAXBLKS       (1L << (MEMSLABSHIFT - MEMALIGNSHIFT))
#if (MEMBUFMAXBLKS <= 65536)
#define MEMBLKID_T          uint16_t
#else
#define MEMBLKID_T          uint32_t
#endif
#define MEMSMALLSLOT        10
#define MEMMIDSLOT          15
//#define MEMBIGSLOT          (MEMSMALLSLOTS - 1)
#if 0
#define MEMSMALLPAGESLOT    64
#define MEMMIDPAGESLOT      128
#define MEMBIGPAGESLOT      256
#define MEMPAGESLOTS        512
#endif
#define MEMSMALLPAGESLOT    16
#define MEMMIDPAGESLOT      64
#define MEMBIGPAGESLOT      128
#define MEMPAGESLOTS        256
//#define MEMSMALLBLKSHIFT    (PAGESIZELOG2 - 1)
#define MEMSMALLMAPSLOT     22
#define MEMMIDMAPSLOT       24
#define MEMBIGMAPSLOT       26

struct membkt {
#if (MEMLFDEQ)
    struct lfdeq   list;
#else
    struct membuf *list;        // bi-directional list of bufs + lock-bit
#endif
    MEMWORD_T      nbuf;
    uint8_t        _pad[CLSIZE
#if (MEMLFDEQ)
                        - sizeof(struct lfdeq)
#else
                        - sizeof(struct membuf *)
#endif
                        - sizeof(MEMWORD_T)];
};

/*
 * struct membufinfo {
 *     unsigned nblk : MEMBUFNBLKBITS;
 *     unsigned slot : MEMBUFSLOTBITS;
 *     unsigned type : MEMBUFTYPEBITS;
 * };
 */
/* type-bits for allocation buffers */
#define MEMSMALLBUF      0x00
#define MEMPAGEBUF       0x01
#define MEMBIGBUF        0x02
#define MEMBUFTYPES      3
#define MEMBUFTYPEBITS   2
//#define MEMTLSBIT        (MEMWORD(1) << (8 * sizeof(MEMWORD_T) - 1))
#define MEMBUFLKBIT      (MEMWORD(1) << (8 * sizeof(MEMWORD_T) - 1))
//#define MEMBUFGLOBBIT    (MEMWORD(1) << (8 * sizeof(MEMWORD_T) - 2))
#define MEMBUFLKBITID    (8 * sizeof(MEMWORD_T) - 1)
//#define MEMBUFGLOBBITID  (8 * sizeof(MEMWORD_T) - 2)
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
#define MEMHEAPBIT       (MEMWORD(1) << (8 * sizeof(MEMWORD_T) - 2))
#endif
#define MEMBUFNBLKBITS   16
//#define MEMBUFMAXBLKS    (MEMWORD(1) << MEMBUFNBLKBITS)
#define MEMBUFSLOTBITS   12
#define MEMBUFSLOTSHIFT  (MEMBUFNBLKBITS)
#define MEMBUFTYPESHIFT  (MEMBUFSLOTSHIFT + MEMBUFSLOTBITS)
#if 0
#define MEMBUFNFREEBITS   MEMBUFNBLKBITS
#define MEMBUFNFREEMASK  (MEMBUFNBLKMASK << MEMBUFNFREESHIFT)
#define MEMBUFNFREESHIFT  MEMBUFNBLKBITS
#endif

#define meminitbufnblk(buf, n)                                          \
    ((buf)->info |= (n))
#define meminitbuftype(buf, t)                                          \
    ((buf)->info |=  (t) << MEMBUFTYPESHIFT)
#define meminitbufslot(buf, slot)                                       \
    ((buf->info)  |= (slot) << MEMBUFSLOTSHIFT)
#define memsetbufnfree(buf, n)                                          \
    ((buf)->nfree = (n))
#define memgetbufheapflg(buf)                                           \
    ((buf)->info & MEMHEAPBIT)
#define memgetbufnblk(buf)                                              \
    ((buf)->info & ((MEMWORD(1) << MEMBUFNBLKBITS) - 1))
#define memgetbufnfree(buf)                                             \
    ((buf)->nfree)
#define memgetbuftype(buf)                                              \
    (((buf)->info >>  MEMBUFTYPESHIFT) & ((MEMWORD(1) << MEMBUFTYPEBITS) - 1))
#define memgetbufslot(buf)                                              \
    (((buf)->info >> MEMBUFSLOTSHIFT) & ((MEMWORD(1) << MEMBUFSLOTBITS) - 1))

struct membufvals {
    MEMWORD_T *nblk[MEMBUFTYPES];
    MEMWORD_T *ntls[MEMBUFTYPES];
    MEMWORD_T *nglob[MEMBUFTYPES];
};

#define MEMINITBIT   (1L << 0)
#define MEMNOHEAPBIT (1L << 1)
struct mem {
//    struct membuf       *bighdr[MEMBIGSLOTS];
    struct membkt        bigbin[MEMBIGSLOTS];
//    struct membuf       *pagehdr[MEMPAGESLOTS];
    struct membkt        pagebin[MEMPAGESLOTS];
//    struct membuf       *smallhdr[MEMSMALLSLOTS];
    struct membkt        smallbin[MEMSMALLSLOTS];
#if (MEMDEADBINS)
    struct membkt        deadpage[MEMPAGESLOTS];
    struct membkt        deadsmall[MEMSMALLSLOTS];
#endif
    struct membuf       *hdrbuf;
    struct membuf       *stkbuf;
//    struct membufvals   bufvals;
#if (MEMMULTITAB)
    struct memtabl0     *tab;     // allocation lookup structure
#elif (MEMHASHSUBTABS)
    struct memhashbkt  **hash;
    struct memhash      *hashbuf;
#elif (MEMNEWHASH)
    struct memhashlist  *hash;    // hash table
    struct memhash      *hashbuf; // buffer for hash items
#endif
    MEMWORD_T            flg;     // memory interface flags
    MEMWORD_T            nbytetab[MEMBUFTYPES];
#if 0
    MEMWORD_T            nbsmall;
    MEMWORD_T            nbpage;
    MEMWORD_T            nbbig;
#endif
    struct membuf       *heap;    // heap allocations (try sbrk(), then mmap())
    struct membuf       *maps;    // mapped blocks
#if (MEM_LK_TYPE == MEM_LK_PRIO)
    zerospin             priolk;
    unsigned long        prioval; // locklessinc priority locks
#endif
    zerospin             initlk;  // lock for initialisation
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
    MEMLK_T              heaplk;  // lock for sbrk()
#endif
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

#define MEMNOBLK (MEMWORD(-1))

#if (MEMMAPSTACK)
#define membufhdrsize()                                                 \
    (rounduppow2(sizeof(struct membuf), CLSIZE))
#elif (MEMNEWHDR)
#define membufhdrsize(type)                                             \
    (((type) == MEMSMALLBUF)                                            \
     ? rounduppow2(offsetof(struct membuf, stk)                         \
                   + (nblk) * sizeof(MEMBLKID_T),                       \
                   PAGESIZE)                                            \
     : rounduppow2(sizeof(struct membuf), PAGESIZE))
#endif
struct membuf {
    volatile struct memtls *tls;
    MEMPTR_T                base; // base address for allocations
#if (MEMBITFIELD)
    struct membufinfo       info;
#else
    MEMWORD_T               info; // slot + # of total & free blks
#endif
    struct membuf          *heap; // previous buf in heap for bufs from sbrk()
    struct membuf          *prev; // previous buf in chain
    struct membuf          *next; // next buf in chain
    MEMWORD_T               size; // allocation blocks
    MEMWORD_T               nfree;
    MEMWORD_T               freemap[MEMBUFBITMAPWORDS];
    MEMWORD_T               relmap[MEMBUFBITMAPWORDS];
    MEMWORD_T               stktop;
    MEMWORD_T               stklim;
#if (MEMMAPSTACK)
    MEMBLKID_T             *stk;
    MEMWORD_T               stksize;
#elif (MEMMAPHDR) || (MEMNEWHDR)
    MEMBLKID_T              stk[EMPTY];
#endif
};

#define memtlssize() rounduppow2(sizeof(struct memtls), 2 * PAGESIZE)
struct memtls {
    struct membkt     bigbin[MEMBIGSLOTS];
    struct membkt     pagebin[MEMPAGESLOTS];
    struct membkt     smallbin[MEMSMALLSLOTS];
#if (MEM_LK_TYPE & MEM_LK_PRIO)
    struct priolkdata priolkdata;
#endif
    MEMWORD_T         nbytetab[MEMBUFTYPES];
    MEMWORD_T         flg;
#if 0
    MEMWORD_T         nbsmall;
    MEMWORD_T         nbpage;
#endif
};

volatile struct memtls * meminittls(void);

#define membufslotblkid(buf, ptr, slot)                                 \
    (((MEMADR_T)(ptr) - (MEMADR_T)(buf)->base) >> (slot))
#define membufslotblkadr(buf, ndx, slot)                                \
    ((buf)->base + ((ndx) << (slot)))
#define membufslotpageadr(buf, ndx, slot)                               \
    ((buf)->base + (ndx) * (PAGESIZE + PAGESIZE * (slot)))
#define membufpageadr(buf, ndx)                                         \
    (membufslotpageadr(buf, ndx, memgetbufslot(buf)))
#define membufblkid(buf, ptr)                                           \
    (membufslotblkid(buf, ptr, memgetbufslot(buf)))
#define membufblkadr(buf, ndx)                                          \
    (membufslotblkadr(buf, ndx, memgetbufslot(buf)))
#define membufblksize(buf, type, slot)                                  \
    ((type != MEMPAGEBUF)                                               \
     ? (MEMWORD(1) << (slot))                                           \
     : (PAGESIZE) + (PAGESIZE) * (slot))

/* mark the first block of buf as allocated */
#define _memfillmap0(ptr, ofs, mask)                                    \
    ((ptr)[(ofs)] = (mask) & ~MEMWORD(1),                               \
     (ptr)[(ofs) + 1] = (mask),                                         \
     (ptr)[(ofs) + 2] = (mask),                                         \
     (ptr)[(ofs) + 3] = (mask))
#define _memfillmap(ptr, ofs, mask)                                     \
    ((ptr)[(ofs)] = (mask),                                             \
     (ptr)[(ofs) + 1] = (mask),                                         \
     (ptr)[(ofs) + 2] = (mask),                                         \
     (ptr)[(ofs) + 3] = (mask))

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

#else

#define MEMHASHDEL      (-1)
#define MEMHASHCHK        0
#define MEMHASHADD        1
#define MEMHASHNOTFOUND   0

struct memhashitem {
#if defined(MEMHASHNREF) && (MEMHASHNREF)
    MEMWORD_T nref;     // reference count for the page
#endif
#if defined(MEMHASHNACT) && (MEMHASHNACT)
    MEMWORD_T nact;     // number of inserts, finds, and deletes
#endif
    MEMADR_T  adr;      // allocation address
    MEMADR_T  val;      // stored value
};

#if (MEMHUGEHASHTAB)
#define MEMHASHARRAYSIZE   (512 * WORDSIZE)
#elif (MEMBIGHASHTAB)
#define MEMHASHARRAYSIZE   (256 * WORDSIZE)
#elif (MEMSMALLHASHTAB)
#define MEMHASHARRAYSIZE   (64 * WORDSIZE)
#elif (MEMTINYHASHTAB)
#define MEMHASHARRAYSIZE   (32 * WORDSIZE)
#else
#define MEMHASHARRAYSIZE   (128 * WORDSIZE)
#endif
#define MEMHASHARRAYITEMS                                               \
    ((MEMHASHARRAYSIZE - offsetof(struct memhash, data))                \
     / sizeof(struct memhashitem))
#define memhashsize()      MEMHASHARRAYSIZE
#endif
#if (MEMBIGHASH)
#define MEMHASHBITS        20
#else
#define MEMHASHBITS        17
#endif
#if (MEMHASHSUBTABS)
#define MEMHASHITEMS       (1 << (MEMHASHBITS - MEMHASHSUBTABSHIFT))
#define MEMHASHSUBTABSHIFT 8
#define MEMHASHSUBTABITEMS (1 << MEMHASHSUBTABSHIFT)
#else
#define MEMHASHITEMS       (1 << MEMHASHBITS)
#endif

struct memhash {
    struct memhash     *chain;  // next array in this chain
    MEMWORD_T           ntab;   // number of occupied slots in this table
    struct memhashitem *tab;    // pointer to the item table
    MEMWORD_T           pad;
    MEMWORD_T           data;   // base address for the table
};

#if (MEMHASHSUBTABS)
struct memhashbkt {
    m_atomic_t      lk;
    struct memhash *tab[1L << MEMHASHSUBTABSHIFT];
};
#endif

#if (MEMNEWHASH)

struct memhashlist {
    m_atomic_t      lk;
    struct memhash *chain;
    uint8_t         _pad[CLSIZE - sizeof(MEMLK_T) - sizeof(struct memchain *)];
};

#if (MEMHASHMURMUR)
#define memhashptr(page) MurmurHash3Mixer((MEMADR_T)page)
#else
#if (WORDSIZE == 4)
#define memhashptr(page)                                                \
    (tmhash32((MEMADR_T)page))
#elif (WORDSIZE == 8)
#define memhashptr(page)                                                \
    (tmhash64((MEMADR_T)page))
#endif
#endif

#if 0
struct memhashsubitem {
    MEMADR_T val;
};
#endif

/*
 * - we have a 4-word header; adding total of 52 words as 13 hash-table entries
 *   lets us cache-color the table by adding a modulo-9 value to the pointer
 */

static __inline__ void
membufsetrel(struct membuf *buf, MEMWORD_T id)
{
    MEMUWORD_T ndx = id;

    ndx >>= MEMWORDSIZESHIFT;
    id &= (MEMWORD(1) << MEMWORDSIZESHIFT) - 1;
    m_setbit((m_atomic_t *)&buf->relmap[ndx], (m_atomic_t)id);

    return;
}

static __inline__ void
membufinitmap(struct membuf *buf, MEMWORD_T nblk)
{
    MEMWORD_T  bits = ~MEMWORD(0);      // all 1-bits
    MEMWORD_T *ptr = buf->freemap;
    MEMWORD_T  n = sizeof(MEMWORD_T) * CHAR_BIT;

    while (nblk >= n) {
        *ptr = bits;
        nblk -= n;
        ptr++;
    }
    bits = MEMWORD(1) << nblk;
    if (nblk) {
//        bits >>= (WORDSIZE * CHAR_BIT - nblk);
        bits--;
        *ptr = bits;
    }

    return;
}
#if 0
static __inline__ void
membufinitmap(struct membuf *buf, MEMWORD_T nblk)
{
    MEMWORD_T  bits = ~MEMWORD(0);      // all 1-bits
    MEMWORD_T *ptr = buf->freemap;

    _memfillmap0(ptr, 0, bits);
#if (MEMBUFBITMAPWORDS >= 8)
    _memfillmap(ptr, 4, bits);
#endif
#if (MEMBUFBITMAPWORDS == 16)
    _memfillmap(ptr, 8, bits);
    _memfillmap(ptr, 12, bits);
#endif

    return;
}
#endif

static __inline__ MEMWORD_T
membufscanblk(struct membuf *buf)
{
    MEMWORD_T  nblk = memgetbufnblk(buf);
    MEMWORD_T *map = buf->freemap;
    MEMWORD_T  ndx = 0;
    MEMWORD_T  word;
    MEMWORD_T  mask;
    MEMWORD_T  res;

    do {
        word = *map;
        if (word) {                             // skip 0-words
            res = tzerol(word);                 // count trailing zeroes
            ndx += res;                         // add to ndx
            if (ndx < nblk) {
                mask = MEMWORD(1) << res;       // create bit
                mask = ~mask;                   // invert for mask
                word &= mask;                   // mask the bit out
                *map = word;                    // update

                return ndx;                     // return index of first 1-bit
            }

            return MEMNOBLK;                    // 1-bit not found
        }
        ndx += sizeof(MEMWORD_T) * CHAR_BIT;
        map++;                                  // try next word in freemap
    } while (ndx < nblk);

    return MEMNOBLK;                            // 1-bit not found
}

static __inline__ MEMWORD_T
membufputblk(struct membuf *buf, MEMWORD_T id)
{
    MEMWORD_T nfree = buf->nfree;
    MEMWORD_T *map = buf->freemap;
#if (MEMBUFMAXBLKS <= 65536)
    MEMWORD_T  word = id >> 2;
    MEMWORD_T  pos = id & (0x03);
#else
    MEMWORD_T  word = id >> 4;
    MEMWORD_T  pos = id & (0x0f);
#endif
    MEMWORD_T  mask = MEMWORD(1) << pos;

    nfree++;
    map[word] |= mask;
    buf->nfree = nfree;

    return nfree;
}

static __inline__ void
membuffreemap(struct membuf *buf)
{
    MEMWORD_T *freemap = buf->freemap;
    MEMWORD_T *relmap = buf->relmap;
    MEMWORD_T  nfree;
    MEMWORD_T  free;
    MEMWORD_T  rel;
    MEMWORD_T  cnt;
    MEMWORD_T  ndx;

    nfree = buf->nfree;
    for (ndx = 0 ; ndx < MEMBUFBITMAPWORDS ; ndx++) {
        rel = relmap[ndx];
        free = freemap[ndx];
        if (rel) {
#if (WORDSIZE == 4)
            cnt = bitcnt1u32(rel);
#elif (WORDSIZE == 8)
            cnt = bitcnt1u64(rel);
#endif
            relmap[ndx] = 0;
            free |= rel;
            nfree += cnt;
            freemap[ndx] = free;
        }
    }
    buf->nfree = nfree;

    return;
}

static __inline__ void
membufinitstk(struct membuf *buf, MEMWORD_T nblk)
{
#if (MEMMAPSTACK)
    MEMBLKID_T *stk;
    MEMWORD_T   ssz = rounduppow2(nblk * sizeof(MEMBLKID_T), PAGESIZE);
#else
    MEMBLKID_T *stk = buf->stk;
#endif
    MEMWORD_T   ndx;
    MEMBLKID_T  id;

#if (MEMMAPSTACK)
    stk = mapanon(0, ssz);
    if (stk == MAP_FAILED) {

        abort();
    }
#endif
    buf->nfree = nblk;
    buf->stktop = 0;
    buf->stklim = nblk;
    ndx = 0;
    id = 0;
    while (ndx < nblk) {
        stk[ndx] = id;
        ndx++;
        id++;
    }
#if (MEMMAPSTACK)
    buf->stksize = ssz;
    buf->stk = stk;
#endif
    
    return;
}

static __inline__ MEMWORD_T
membufpopblk(struct membuf *buf)
{
    MEMWORD_T nfree = buf->nfree;
    MEMWORD_T cur = buf->stktop;
    MEMWORD_T id;

    nfree--;
    id = buf->stk[cur];
    cur++;
    buf->nfree = nfree;
    buf->stktop = cur;
#if (MEMTESTSTACK) && 0
    memprintbufstk(buf, "MEMBUFGETFREE");
    fprintf(stderr, "ID == %ld\n", id);
#endif

    return id;
}

static __inline__ MEMWORD_T
membufpushblk(struct membuf *buf, MEMBLKID_T id)
{
    MEMWORD_T nfree = buf->nfree;
    MEMWORD_T cur = buf->stktop;

    cur--;
    nfree++;
    buf->stk[cur] = id;
    buf->stktop = cur;
    buf->nfree = nfree;
#if (MEMTESTSTACK) && 0
    memprintbufstk(buf, "MEMBUFPUTFREE");
    fprintf(stderr, "ID == %ld\n", id);
#endif

    return nfree;
}

static __inline__ void
membuffreestk(struct membuf *buf)
{
    MEMWORD_T *relmap = buf->relmap;
    MEMWORD_T  rel;
    MEMWORD_T  mask;
    MEMWORD_T  ndx;
    MEMWORD_T  id;
    MEMWORD_T  ofs;

    ofs = 0;
    for (ndx = 0 ; ndx < MEMBUFBITMAPWORDS ; ndx++) {
        rel = relmap[ndx];
        if (rel) {
            do {
                id = tzerol(rel);
                mask = 1;
                if (id < WORDSIZE) {
                    mask <<= id;
                    id += ofs;
                    membufpushblk(buf, id);
                    rel |= ~mask;
                } else {
                    
                    break;
                }
            } while (1);
        }
        ofs += WORDSIZE * CHAR_BIT;
    }

    return;
}

#define memalignptr(ptr, pow2)                                          \
    ((MEMPTR_T)rounduppow2((uintptr_t)(ptr), (pow2)))
#define memadjptr(ptr, sz)                                              \
    (&((MEMPTR_T)(ptr))[sz])

#if (MEMCACHECOLOR)

/* compute adr + adr % 9 (# of cachelines in offset, aligned to cl boundary) */
static __inline__ MEMWORD_T *
memgentlsadr(MEMWORD_T *adr)
{
    /* division by 9 */
    MEMADR_T res = (MEMADR_T)adr;
    MEMADR_T q;
    MEMADR_T r;
    MEMADR_T div9;
    MEMADR_T dec;

    res >>= PAGESIZELOG2;
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
    /* scale to 0..256 (machine words) */
    res <<= 5;
    /* align to cacheline */
    res &= ~(CLSIZE - 1);
    /* add to original pointer */
    adr += res;

    return adr;
}

static __inline__ MEMPTR_T
memgenadr(MEMPTR_T ptr)
{
    MEMPTR_T   adr = ptr;
    MEMADR_T   res = (MEMADR_T)ptr;
//    MEMWORD_T lim = blksz - size;
    MEMWORD_T  shift;
    MEMADR_T   q;
    MEMADR_T   r;
    MEMADR_T   div9;
    MEMADR_T   dec;

#if (CLSIZE == 32)
    shift = 3;
#elif (CLSIZE == 64)
    shift = 4;
#endif
    /* shift out some [mostly-aligned] low bits */
    res >>= MEMALIGNSHIFT;
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
    /* scale by shifting the result of the range 0..8 */
    res <<= shift;
    /* round down to a multiple of cacheline */
    res &= ~(CLSIZE - 1);
    /* add offset to original address */
    adr += res;

    return adr;
}

/* generate an offset in the range [0, 4 * CLSIZE] */
static __inline__ MEMADR_T
memgenofs(MEMPTR_T ptr)
{
    MEMADR_T  res = (MEMADR_T)ptr;
    MEMWORD_T shift;
    MEMADR_T  q;
    MEMADR_T  r;
    MEMADR_T  div9;
    MEMADR_T  dec;

#if (CLSIZE == 32)
    shift = 4;
#elif (CLSIZE == 64)
    shift = 5;
#endif
    /* shift out some [mostly-aligned] low bits */
    res >>= MEMALIGNSHIFT;
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
    /* scale by shifting the result of the range 0..8 */
    res <<= shift;
    /* round down to a multiple of cacheline */
    res &= ~(CLSIZE - 1);
    /* add offset to original address */

    return res;
}

/* compute adr + adr % 9 (# of words in offset, aligned to word boundary) */
static __inline__ MEMWORD_T *
memgenhashtabadr(MEMWORD_T *adr)
{
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

#else

#define memgentlsadr(adr)     (adr)
#define memgenadr(adr)        (adr)
//#define memgenofs(adr)        (adr)
#define memgenhashtabadr(adr) (adr)

#endif /* MEMCACHECOLOR */

/*
 * for 32-bit pointers, we can use a flat lookup table for bookkeeping pointers
 * - for bigger pointers, we use a hash (or multilevel) table
 */
#if ((MEMNEWHASH) || (MEMHASH)) && !defined(MEMHASHITEMS)
#define MEMHASHBITS   14
#define MEMHASHITEMS  (MEMWORD(1) << MEMHASHBITS)
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
/* compute level keys in reverse order */
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
#endif /* (MEMNEWHASH || MEMHASH) && !defined(MEMHASHITEMS) */

/*
 * allocation headers
 * - 3 allocation classes:
 *   - small; block size is 1 << slot
 *   - page; block size is PAGESIZE + PAGESIZE * slot
 *   - big; block size is 1 << slot
 */
//#define membufptrtabsize(nblk) (MEMBUFMAXBLKS * sizeof(MEMPTR_T))
//#define membufslot(buf)       (memgetbufslot(buf))
#if 0
#define membufblkofs(nblk)                                              \
    (rounduppow2(sizeof(struct membuf), PAGESIZE))
#endif
#define memusesmallbuf(sz)     ((sz) <= PAGESIZE)
#define memusepagebuf(sz)      ((sz) <= PAGESIZE * MEMPAGESLOTS)
#if (MEMMAPHDR) || (MEMNEWHDR)
#define memsmallbufsize(slot, nblk)                                     \
    ((nblk) << (slot))
#define mempagebufsize(slot, nblk)                                      \
    ((PAGESIZE + PAGESIZE * (slot)) * (nblk))
#define membigbufsize(slot, nblk)                                       \
    ((nblk) << (slot))
#else
#define memsmallbufsize(slot, nblk)                                     \
    (rounduppow2(membufblkofs(nblk) + ((nblk) << (slot)),               \
                 PAGESIZE))
#define mempagebufsize(slot, nblk)                                      \
    (rounduppow2(membufblkofs(nblk)                                     \
                 + (PAGESIZE + PAGESIZE * (slot)) * (nblk),             \
                 PAGESIZE))
#define membigbufsize(slot, nblk)                                       \
    (rounduppow2(membufblkofs(nblk) + (MEMWORD(1) << (slot)) * (nblk),  \
                 PAGESIZE))
#endif
#define memnbufblk(type, slot)                                          \
    (((type) == MEMSMALLBUF)                                            \
     ? (1L << (MEMSLABSHIFT - (slot)))                                  \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? 32                                                         \
           : (((slot) <= MEMMIDPAGESLOT)                                \
              ? 16                                                      \
              : (((slot) <= MEMBIGPAGESLOT)                             \
                 ? 4                                                    \
                 : 2)))                                                 \
        : (((slot) <= MEMSMALLMAPSLOT)                                  \
           ? 16                                                         \
           : (((slot) <= MEMMIDMAPSLOT)                                 \
              ? 8                                                       \
              : (((slot) <= MEMBIGMAPSLOT)                              \
                 ? 4                                                    \
                 : 1)))))

#define membktnbuftls(type, slot)                                       \
    (((type) == MEMSMALLBUF)                                            \
     ? (((slot) <= MEMSMALLSLOT)                                        \
        ? 4                                                             \
        : (((slot) <= MEMMIDSLOT)                                       \
           ? 2                                                          \
           : 1))                                                        \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? 2                                                          \
           : (((slot) <= MEMBIGPAGESLOT)                                \
              ? 4                                                       \
              : 8))                                                     \
        : (((slot) <= MEMSMALLMAPSLOT)                                  \
           ? 4                                                          \
           : (((slot) <= MEMMIDMAPSLOT)                                 \
              ? 2                                                       \
              : (((slot) <= MEMBIGMAPSLOT)                              \
                 ? 1                                                    \
                 : 0)))))
#if 0
#define membktnbuftls(type, slot)                                       \
    (((type) == MEMSMALLBUF)                                            \
     ? (((slot) <= MEMSMALLSLOT)                                        \
        ? 4                                                             \
        : (((slot) <= MEMMIDSLOT)                                       \
           ? 2                                                          \
           : 1))                                                        \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? 8                                                          \
           : (((slot) <= MEMBIGPAGESLOT)                                \
              ? 4                                                       \
              : 2))                                                     \
        : 0))
#endif
#if (!MEMUNMAP)
#define membktnbufglob(type, slot) (~MEMWORD(0))
#else
#define membktnbufglob(type, slot)                                      \
    (((type) == MEMSMALLBUF)                                            \
     ? (((slot) <= MEMSMALLSLOT)                                        \
        ? 32                                                            \
        : (((slot) <= MEMMIDSLOT)                                       \
           ? 16                                                         \
           : 8))                                                        \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? 16                                                         \
           : (((slot) <= MEMMIDPAGESLOT)                                \
              ? 32                                                      \
              : 64))                                                    \
        : (((slot) <= MEMSMALLMAPSLOT)                                  \
           ? 4                                                          \
           : (((slot) <= MEMMIDMAPSLOT)                                 \
              ? 8                                                       \
              : (((slot) <= MEMBIGMAPSLOT)                              \
                 ? 16                                                   \
                 : 4)))))
#endif

#define memgetnbufblk(type, slot)  memnbufblk(type, slot)
#define memgetnbuftls(type, slot)  memnbuftls(type, slot)
#define memgetnbufglob(type, slot) memnbufglob(type, slot)

#endif

#if (MEMSTAT)
void                     memprintstat(void);
struct memstat {
    MEMWORD_T nbsmall;
    MEMWORD_T nbpage;
    MEMWORD_T nbbig;
    MEMWORD_T nbheap;
    MEMWORD_T nbmap;
    MEMWORD_T nbunmap;
    MEMWORD_T nbbook;
    MEMWORD_T nbhashtab;
    MEMWORD_T nbhash;
    MEMWORD_T nhashchain;
    MEMWORD_T nhashitem;
};
#endif

#if (MEMCACHECOLOR)
static __inline__ MEMPTR_T
memcalcadr(MEMPTR_T ptr, MEMWORD_T size,
           MEMWORD_T blksz, MEMWORD_T align)
{
    MEMPTR_T     adr = ptr;
    MEMWORD_T    diff = blksz - size;
    MEMADRDIFF_T ofs = memgenofs(ptr);

    if (align <= CLSIZE && diff >= ofs) {
         ptr += ofs;
    } else {
        ptr = memalignptr(adr, align);
    }

    return ptr;
}
#else
#define memcalcadr(ptr, size, blksz, align)                             \
    (memalignptr(ptr, align))
#endif

#endif /* __ZERO_MEM_H__ */

