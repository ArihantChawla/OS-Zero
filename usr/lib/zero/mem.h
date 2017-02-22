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
#define memrelbit(ptr) m_cmpclrbit((m_atomic_t *)ptr, MEMLKBITID)

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

#define MEMSMALLTLSLIM      (4 * 1024 * 1024)
#define MEMPAGETLSLIM       (8 * 1024 * 1024)
#define MEMSMALLGLOBLIM     (32 * 1024 * 1024)
#define MEMPAGEGLOBLIM      (64 * 1024 * 1024)
#define MEMBIGGLOBLIM       (256 * 1024 * 1024)

/* determine minimal required alignment for blocks */
#if defined(__BIGGEST_ALIGNMENT__)
//#define MEMMINALIGN         max(__BIGGEST_ALIGNMENT__, 2 * PTRSIZE)
#define MEMMINALIGN         __BIGGEST_ALIGNMENT__
#else
//#define MEMMINALIGN         (2 * PTRSIZE) // allow for dual-word tagged pointers
#define MEMMINALIGN         PTRSIZE
#endif
#if (MEMMINALIGN == 8)
#define MEMALIGNSHIFT       3
#elif (MEMMINALIGN == 16)
#define MEMALIGNSHIFT       4
#elif (MEMMINALIGN == 32)
#define MEMALIGNSHIFT       5
#endif
/* maximum small buf size (MEMBUFMAXBLKS << MEMMAXSMALLSLOT) + bookkeeping */
#define MEMSMALLSLOTS       rounduppow2(MEMMAXSMALLSLOT, 8)
#if (MEMBIGPAGES)
//#define MEMMAXSMALLSLOT     (PAGESIZELOG2 - 1)
#else
//#define MEMMAXSMALLSLOT     (PAGESIZELOG2 - 1)
#define MEMMAXSMALLSLOT     (PAGESIZELOG2 + 4)
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
//#define MEMBUFBITMAPWORDS  (CLSIZE / WORDSIZE)
#define MEMBUFBITMAPWORDS                                               \
    rounduppow2(MEMBUFMAXBLKS / (CHAR_BIT * WORDSIZE), 8)
        
//#define MEMBUFBITMAPWORDS   32
#define MEMSLABSHIFT        18
#define MEMBUFMAXBLKS       (1L << (MEMSLABSHIFT - MEMALIGNSHIFT))
#if (!MEMBUFSTACK)
//#define MEMBUFMAXBLKS       (MEMBUFBITMAPWORDS * WORDSIZE * CHAR_BIT)
#else
//#define MEMBUFBITMAPWORDS   (MEMBUFMAXBLKS / (CHAR_BIT * WORDSIZE))
/* number of block-bits in buf freemap */
#if defined(MEMBUFSTACK) && (MEMBUFSTACK)
#if (MEMBUFMAXBLKS <= 65536)
#define MEMBLKID_T          uint16_t
#else
#define MEMBLKID_T          uint32_t
#endif
#endif
#endif
/* minimum allocation block size in bigbins */
//#define MEMBIGMINSIZE      (2 * PAGESIZE)
//#define MEMPAGESLOTS        (MEMWORD(1) << MEMBUFSLOTBITS)
#define MEMSMALLSLOT        8
#define MEMMIDSLOT          12
//#define MEMBIGSLOT          (MEMSMALLSLOTS - 1)
#if (MEMBIGPAGES)
#define MEMSMALLPAGESLOT    32
#define MEMMIDPAGESLOT      64
#define MEMBIGPAGESLOT      256
#define MEMPAGESLOTS        512
#else
#define MEMSMALLPAGESLOT    16
#define MEMMIDPAGESLOT      64
#define MEMBIGPAGESLOT      256
#define MEMPAGESLOTS        512
#endif
//#define MEMSMALLBLKSHIFT    (PAGESIZELOG2 - 1)
#define MEMSMALLMAPSHIFT    22
//#define MEMBUFMIDMAPSHIFT 22
#define MEMMIDMAPSHIFT      24
//#define MEMBUFHUGEMAPSHIFT  26

struct membkt {
#if (MEMLFDEQ)
    struct lfdeq   list;
#else
    struct membuf *list;        // bi-directional list of bufs + lock-bit
#endif
#if (MEMDEBUGDEADLOCK)
    MEMWORD_T      line;
#endif
    MEMWORD_T      nbuf;
#if 0
    MEMWORD_T      nblk;        // # of per-buffer blocks; 0 for default
    MEMWORD_T      nmax;        // max # to buffer; 0 for default
#endif
    uint8_t        _pad[CLSIZE
#if (MEMLFDEQ)
                        - sizeof(struct lfdeq)
#else
                        - sizeof(struct membuf *)
#endif
#if (MEMDEBUGDEADLOCK)
                        - sizeof(MEMWORD_T)
#endif
                        - 2 * sizeof(MEMWORD_T)];
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
    struct membkt       bigbin[MEMBIGSLOTS];
    struct membkt       pagebin[MEMPAGESLOTS];
    struct membkt       smallbin[MEMSMALLSLOTS];
    struct membkt       deadpage[MEMPAGESLOTS];
    struct membkt       deadsmall[MEMSMALLSLOTS];
//    struct membufvals   bufvals;
#if (MEMMULTITAB)
    struct memtabl0    *tab;     // allocation lookup structure
#elif (MEMNEWHASH)
    struct memhashlist *hash;    // hash table
    struct memhash     *hashbuf; // buffer for hash items
#endif
    MEMWORD_T           flg;     // memory interface flags
    MEMWORD_T           nbytetab[MEMBUFTYPES];
#if 0
    MEMWORD_T           nbsmall;
    MEMWORD_T           nbpage;
    MEMWORD_T           nbbig;
#endif
    struct membuf      *heap;    // heap allocations (try sbrk(), then mmap())
    struct membuf      *maps;    // mapped blocks
#if (MEM_LK_TYPE == MEM_LK_PRIO)
    zerospin            priolk;
    unsigned long       prioval; // locklessinc priority locks
#endif
    zerospin            initlk;  // lock for initialisation
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
    MEMLK_T             heaplk;  // lock for sbrk()
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
    MEMWORD_T               size; // buffer bookkeeping + allocation blocks
    MEMWORD_T               nfree;
#if defined(MEMBUFSTACK) && (MEMBUFSTACK)
    MEMWORD_T               relmap[MEMBUFBITMAPWORDS];
    MEMBLKID_T              stk[MEMBUFMAXBLKS];
    MEMWORD_T               stktop;
//    MEMWORD_T               stklim;
#else
    MEMWORD_T               freemap[MEMBUFBITMAPWORDS];
    MEMWORD_T               relmap[MEMBUFBITMAPWORDS];
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
#if (MEMBIGPAGES)
#define membufslotpageadr(buf, ndx, slot)                               \
    ((buf)->base + (ndx)                                                \
     * (4 * MEMWORD(PAGESIZE) + 4 * MEMWORD(PAGESIZE) * (slot)))
#define membufpageadr(buf, ndx)                                         \
    (membufslotpageadr(buf, ndx, memgetbufslot(buf)))
#else
#define membufslotpageadr(buf, ndx, slot)                               \
    ((buf)->base + (ndx)                                                \
     * (MEMWORD(PAGESIZE) + MEMWORD(PAGESIZE) * (slot)))
#define membufpageadr(buf, ndx)                                         \
    (membufslotpageadr(buf, ndx, memgetbufslot(buf)))
#endif
#define membufblkid(buf, ptr)                                           \
    (membufslotblkid(buf, ptr, memgetbufslot(buf)))
#define membufblkadr(buf, ndx)                                          \
    (membufslotblkadr(buf, ndx, memgetbufslot(buf)))
#if (MEMBIGPAGES)
#define membufblksize(buf, type, slot)                                  \
    ((type != MEMPAGEBUF)                                               \
     ? (MEMWORD(1) << (slot))                                           \
     : (4 * MEMWORD(PAGESIZE) + 4 * MEMWORD(PAGESIZE) * (slot)))
#else
#define membufblksize(buf, type, slot)                                  \
    ((type != MEMPAGEBUF)                                               \
     ? (MEMWORD(1) << (slot))                                           \
     : (MEMWORD(PAGESIZE) + MEMWORD(PAGESIZE) * (slot)))
#endif

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

#if defined(MEMHASHSUBTABS) && (MEMHASHSUBTABS)
struct memhashslot {
    MEMLK_T          lk;
    struct memhash **itab;
};
#endif
struct memhash {
    struct memhash *chain;      // next array in this chain
    MEMWORD_T       ntab;       // number of occupied slots in this table
    struct memhash *tab;        // pointer to the item table
    MEMWORD_T       pad;
    MEMWORD_T       data;       // base address for the table
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
    m_atomic_t      lk;
    struct memhash *chain;
    uint8_t         _pad[CLSIZE - sizeof(MEMLK_T) - sizeof(struct memchain *)];
};

#define MEMHASHDEL (-1)
#define MEMHASHCHK (0)
#define MEMHASHADD (1)

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
#define MEMHASHNOTFOUND  0

#if 0
struct memhashsubitem {
    MEMADR_T val;
};
#endif

struct memhashitem {
#if (!MEMHASHSUBTABS)
#if defined(MEMHASHNREF) && (MEMHASHNREF)
    MEMWORD_T nref;     // reference count for the page
#endif
#if defined(MEMHASHNACT) && (MEMHASHNACT)
    MEMWORD_T nact;     // number of inserts, finds, and deletes
#endif
#endif
    MEMADR_T  adr;      // allocation address
    MEMADR_T  val;      // stored value
};

/*
 * - we have a 4-word header; adding total of 52 words as 13 hash-table entries
 *   lets us cache-color the table by adding a modulo-9 value to the pointer
 */
#if (MEMBIGHASH)
#define MEMHASHBITS        20
#else
#define MEMHASHBITS        17
#endif
#define MEMHASHITEMS       (1 << MEMHASHBITS)
#if (MALLOCHASHSUBTABS)
#define MEMHASHSUBTABBITS  8
#define MEMHASHSUBTABITEMS (1 << MEMHASHSUBTABBITS)
#endif
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
#define memhashtabsize()   rounduppow2(MEMHASHSUBTABITEMS * sizeof(struct memhashitem *), \
                                       PAGESIZE)

#if (MEMHASHSUBTABS)
#if (MEMHASHSUBTABITEMS == 32)
#define MEMHASHTABSHIFT    MEMHASHSUBTABBITS
#elif (MEMHASHSUBTABITEMS == 64)
#define MEMHASHTABSHIFT    MEMHASHSUBTABBITS
#elif (MEMHASHSUBTABITEMS == 128)
#define MEMHASHTABSHIFT    MEMHASHSUBTABBITS
#elif (MEMHASHSUBTABITEMS == 256)
#define MEMHASHTABSHIFT    
#endif
#endif /* MEMHASHSUBTABS */

static __inline__ void
membufsetrel(struct membuf *buf, MEMWORD_T id)
{
    MEMWORD_T ndx = id;

    ndx >>= MEMWORDSIZESHIFT;
    id &= (MEMWORD(1) << MEMWORDSIZESHIFT) - 1;
    m_setbit((m_atomic_t *)&buf->relmap[ndx], id);

    return;
}

#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)

static __inline__ void
membufinitfree(struct membuf *buf)
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

static __inline__ void
membuffreerel(struct membuf *buf)
{
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    MEMWORD_T *freemap = buf->freemap;
#endif
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

#else

static __inline__ void
membufinitfree(struct membuf *buf, MEMWORD_T type, MEMWORD_T slot,
               MEMWORD_T nblk)
{
    MEMWORD_T   bsz = membufblksize(buf, type, slot);
    MEMBLKID_T *stk = buf->stk;
    MEMBLKID_T  id;
//    MEMWORD_T   cur;

    buf->nfree = nblk;
    buf->stktop = 0;
    cur = 0;
#if (MEMBUFSTACK)
    buf->stklim = nblk;
#endif
#if 0
    id = 0;
    while (cur < nblk) {
        stk[cur] = id;
        cur++;
        id++;
    }
#endif
    while (cur < nblk) {
        stk[id] = id;
        id++;
    }
    
    return;
}

static __inline__ MEMWORD_T
membufgetfree(struct membuf *buf)
{
    MEMWORD_T  nfree = buf->nfree;
    MEMWORD_T  cur = buf->stktop;
    MEMBLKID_T id;

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
membufputfree(struct membuf *buf, MEMBLKID_T id)
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
membuffreerel(struct membuf *buf)
{
    MEMWORD_T  *relmap = buf->relmap;
    MEMWORD_T   rel;
    MEMWORD_T   ofs;
    MEMWORD_T   ntz;
    MEMWORD_T   cnt;
    MEMWORD_T   mask;
    MEMBLKID_T  id;

    for (id = 0 ; id < MEMBUFBITMAPWORDS ; id++) {
        rel = relmap[id];
        if (rel) {
            ofs = id;
            do {
                id = tzerol(rel);
                mask = 1;
                if (id < WORDSIZE) {
                    ofs += WORDSIZE * CHAR_BIT;
                    mask <<= id;
                    id += ofs;
                    membufputfree(buf, id);
                    rel |= ~mask;
                } else {

                    break;
                }
            } while (1);
            id++;
        }
    }

    return;
}

#endif

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
#define membufhdrsize()        (sizeof(struct membuf))
//#define membufptrtabsize(nblk) (MEMBUFMAXBLKS * sizeof(MEMPTR_T))
//#define membufslot(buf)       (memgetbufslot(buf))
#define membufblkofs(nblk)                                              \
    (rounduppow2(membufhdrsize(), PAGESIZE))
#if 0
#define membufblkofs(nblk)                                              \
    (rounduppow2(membufhdrsize() + (nblk) * sizeof(MEMPTR_T), PAGESIZE))
#endif
#define memusesmallbuf(sz)     ((sz) <= (PAGESIZE << 3))
#if (MEMBIGPAGES)
#define memusepagebuf(sz)      ((sz) <= (PAGESIZE * MEMPAGESLOTS))
#define mempagebufsize(slot, nblk)                                      \
    (rounduppow2(membufblkofs(nblk)                                     \
                 + (MEMWORD(PAGESIZE) + MEMWORD(PAGESIZE) * (slot)) * (nblk)), \
     PAGESIZE))
#else
#define memusepagebuf(sz)      ((sz) <= (PAGESIZE * MEMPAGESLOTS))
#define mempagebufsize(slot, nblk)                                      \
    (rounduppow2(membufblkofs(nblk)                                     \
                 + ((MEMWORD(PAGESIZE) + MEMWORD(PAGESIZE) * (slot))   \
                    * (nblk)),                                         \
                 PAGESIZE))
#endif
/* allocations of PAGESIZE * slot */
#define memsmallbufsize(slot, nblk)                                     \
    (rounduppow2(membufblkofs(nblk) + ((nblk) << (slot)),               \
                 PAGESIZE))
#define membigbufsize(slot, nblk)                                       \
    (rounduppow2(membufblkofs(nblk) + (MEMWORD(1) << (slot)) * (nblk),  \
                 PAGESIZE))
#if (MEMBIGPAGES)
#define memnbufblk(type, slot)                                          \
    (((type) == MEMSMALLBUF)                                            \
     ? (((slot) <= MEMSMALLSLOT)                                        \
        ? (MEMBUFSMALLBLKS)                                             \
        : (((slot) <= MEMMIDSLOT)                                       \
           ? (MEMBUFMIDBLKS)                                            \
           : (MEMBUFBIGBLKS)))                                          \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? 32                                                         \
           : (((slot) <= MEMMIDPAGESLOT)                                \
              ? 16                                                      \
              : (((slot) <= MEMBIGPAGESLOT)                             \
                 ? 8                                                    \
                 : 4)))                                                 \
        : (((slot) <= MEMSMALLMAPSHIFT)                                 \
           ? 8                                                          \
           : 1)))
#elif (MEMOPTBUF)
#define memnbufblk(type, slot)                                          \
    (((type) == MEMSMALLBUF)                                            \
     ? (((slot) <= MEMSMALLSLOT)                                        \
        ? (1L << (MEMSLABSHIFT - (slot)))                               \
        : (((type <= MEMMIDSLOT))                                       \
           ? (1L << (MEMSLABSHIFT - (slot) + 1))                        \
           : (1L << (MEMSLABSHIFT - (slot) + 2))))                      \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? 32                                                         \
           : (((slot) <= MEMMIDPAGESLOT)                                \
              ? 16                                                      \
              : (((slot) <= MEMBIGPAGESLOT)                             \
                 ? 8                                                    \
                 : 4)))                                                 \
        : (((slot) <= MEMSMALLMAPSHIFT)                                 \
           ? 32                                                         \
           : (((slot) <= MEMMIDMAPSHIFT)                                \
              : (((slot) <= MEMBIGMAPSHIFT)                             \
                 ? 8                                                    \
                 : 4)))))
#else
#define memnbufblk(type, slot)                                          \
    (((type) == MEMSMALLBUF)                                            \
     ? (MEMSLABSHIFT - (slot))                                          \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? 16                                                         \
           : (((slot) <= MEMMIDPAGESLOT)                                \
              ? 8                                                       \
              : 4))                                                     \
        : (((slot) <= MEMSMALLMAPSHIFT)                                 \
           ? 8                                                          \
           : (((slot) <= MEMMIDMAPSHIFT)                                \
              ? 4                                                       \
              : 1))))
#if 0
#define memnbufblk(type, slot)                                          \
    (((type) == MEMSMALLBUF)                                            \
     ? (((slot) <= MEMSMALLSLOT)                                        \
        ? (1L << (MEMSLABSHIFT - (slot)))                               \
        : (((slot) <= MEMMIDSLOT)                                       \
           ? (1L << (MEMSLABSHIFT - (slot) + 1))                        \
           : (1L << (MEMSLABSHIFT - (slot) + 2))))                      \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? 16                                                         \
           : (((slot) <= MEMMIDPAGESLOT)                                \
              ? 8                                                       \
              : 4))                                                     \
        : (((slot) <= MEMSMALLMAPSHIFT)                                 \
           ? 8                                                          \
           : (((slot) <= MEMMIDMAPSHIFT)                                \
              ? 4                                                       \
              : 1))))
#endif
#endif
#define membktnbuftls(type, slot)                                       \
    (((type) == MEMSMALLBUF)                                            \
     ? (((slot) <= MEMSMALLSLOT)                                        \
        ? 4                                                             \
        : (((slot) <= MEMMIDSLOT)                                       \
           ? 2                                                          \
           : 1))                                                        \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? 4                                                          \
           : (((slot) <= MEMMIDPAGESLOT)                                \
              ? 2                                                       \
              : 1))                                                     \
        : 0))
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
              ? 8                                                       \
              : 4))                                                     \
        : (((slot) <= MEMSMALLMAPSHIFT)                                 \
           ? 8                                                          \
           : (((slot) <= MEMMIDMAPSHIFT)                                \
              ? 4                                                       \
              : 2))))
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
//          MEMWORD_T id)
{
    MEMPTR_T     adr = ptr;
    MEMWORD_T    diff = blksz - size;
    MEMADRDIFF_T ofs = memgenofs(ptr);

#if 0
    if (align <= CLSIZE && diff >= ofs) {
         ptr += ofs;
    } else {
        ptr = memalignptr(adr, align);
    }
#endif

    return ptr;
}
#else
#define memcalcadr(ptr, size, blksz, align)                             \
    (memalignptr(ptr, align))
#endif

#endif /* __ZERO_MEM_H__ */

