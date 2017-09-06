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
    ((cond)                                                             \
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

#define MEM_LK_NONE    0x01      // don't use locks; single-thread
#define MEM_LK_PRIO    0x02      // priority-based locklessinc.com lock
#define MEM_LK_FMTX    0x04      // anonymous non-recursive mutex
#define MEM_LK_SPIN    0x08      // spinlock
#define MEM_LK_SPINWT  0x10      // spin-wait lock

#define MEM_LK_TYPE    MEM_LK_FMTX // type of locks to use

#if (MEMBLKHDR)
#define MEMBUFDEL      0
#define MEMBUFCHK      1
#define MEMBUFADD      2
#define MEMGETPAD      3
#else
#define MEMBUFDEL      (-1)
#define MEMBUFCHK      0
#define MEMBUFADD      1
#endif
#define MEMBUFNOTFOUND 0

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
#elif (MEM_LK_TYPE == MEM_LK_SPIN) || (MEM_LK_TYPE == MEM_LK_SPINWT)
typedef zerospin      MEMLK_T;
#endif

struct membuf;

void                     meminit(void);
MEMPTR_T                 memgetblk(MEMWORD_T slot, MEMWORD_T type,
                                   MEMWORD_T size, MEMWORD_T align);
MEMADR_T                 membufop(MEMPTR_T ptr, MEMWORD_T op,
                                  struct membuf *buf, MEMWORD_T id);
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
#define memgetlk(lp)    priolkget(lp)
#define memrellk(lp)    priolkrel(lp)
#elif (MEM_LK_TYPE == MEM_LK_FMTX)
#define memgetlk(lp)    fmtxlk(lp)
#define memrellk(lp)    fmtxunlk(lp)
#elif (MEM_LK_TYPE == MEM_LK_SPIN)
#define memgetlk(lp)    spinlk(lp)
#define memrellk(lp)    spinunlk(lp)
#elif (MEM_LK_TYPE == MEM_LK_SPINWT)
#define memgetlk(lp)    spinwtlk(lp, ZEROSPINLKVAL, 4096)
#define memrellk(lp)    spinwtunlk(lp)
#endif

#if (MEMBKTLOCK)
#define memlkbkt(bkt)   memgetlk(&bkt->lk)
#define memunlkbkt(bkt) memrellk(&bkt->lk)
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
    while (!m_cmpsetbit((m_atomic_t *)ptr, MEMLKBITID)) {               \
        m_waitint();                                                    \
    }
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
        MEMUWORD_T _res = sz;                                           \
                                                                        \
        _res--;                                                         \
        _res >>= PAGESIZELOG2;                                          \
        (slot) = _res;                                                  \
    } while (0)

#define MEMSMALLTLSLIM      (8 * 1024 * 1024)
#define MEMPAGETLSLIM       (16 * 1024 * 1024)
#define MEMSMALLGLOBLIM     (128 * 1024 * 1024)
#define MEMPAGEGLOBLIM      (256 * 1024 * 1024)
#define MEMBIGGLOBLIM       (512 * 1024 * 1024)

//#define MEMMINALIGN         CLSIZE
#if !defined(MEMMINALIGN)
/* determine minimal required alignment for blocks */
#if defined(__BIGGEST_ALIGNMENT__)
//#define MEMMINALIGN         max(__BIGGEST_ALIGNMENT__, 2 * PTRSIZE)
#define MEMMINALIGN         __BIGGEST_ALIGNMENT__
#else
//#define MEMMINALIGN         (2 * PTRSIZE) // allow for dual-word tagged pointers
#define MEMMINALIGN         (2 * PTRSIZE)
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

//#define MEMBUFBITMAPWORDS   32
//#define MEMSLABSHIFT        18
#define MEMSLABSHIFT        18
#define MEMBUFMAXBLKS       (1L << (MEMSLABSHIFT - MEMALIGNSHIFT))
#if (MEMBUFMAXBLKS <= 65536)
#define MEMBLKID_T          uint16_t
#else
#define MEMBLKID_T          uint32_t
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
#define MEMBUFMAXBITS       MEMBUFMAXBLKS
#define MEMBUFBITMAPWORDS                                               \
    rounduppow2(MEMBUFMAXBITS / (WORDSIZE * CHAR_BIT), CLSIZE)

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
#define MEMMIDPAGESLOT      32
#define MEMBIGPAGESLOT      128
#define MEMPAGESLOTS        256
#if 0
#define MEMSMALLPAGELIM     (MEMSMALLPAGESLOT + 2)
#define MEMMIDPAGELIM       (MEMMIDPAGESLOT + 1)
#define MEMBIGPAGELIM       (MEMBIGPAGESLOT + 0)
#endif
//#define MEMSMALLBLKSHIFT    (PAGESIZELOG2 - 1)
#define MEMSMALLMAPSLOT     21
#define MEMMIDMAPSLOT       23
#define MEMBIGMAPSLOT       25
#if 0
#define MEMSMALLMAPLIM      (MEMSMALLMAPSLOT + 2)
#define MEMMIDMAPLIM        (MEMMIDMAPSLOT + 1)
#define MEMBIGMAPLIM        (MEMBIGMAPSLOT)
#endif

struct membkt {
#if (MEMBKTLOCK)
    MEMLK_T        lk;
#endif
#if (MEMLFDEQ)
    struct lfdeq   list;
#else
    struct membuf *list;        // bi-directional list of bufs + lock-bit
#endif
    MEMWORD_T      nbuf;
    uint8_t        _pad[CLSIZE
#if (MEMBKTLOCK)
                        + CLSIZE
                        - sizeof(MEMLK_T)
#endif
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
//#define MEMBUFLKBIT      (MEMWORD(1) << (8 * sizeof(MEMWORD_T) - 1))
//#define MEMBUFLKBITID    (8 * sizeof(MEMWORD_T) - 1)
//#define MEMBUFGLOBBITID  (8 * sizeof(MEMWORD_T) - 2)
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
#define MEMBUFHEAPBIT    (MEMWORD(1) << (CHAR_BIT * sizeof(MEMWORD_T) - 1))
#define MEMFLAGBITS      1
#else
#define MEMBUFFLAGBITS   0
#endif
#define MEMBUFSLOTBITS                                                  \
    (CHAR_BIT * sizeof(MEMWORD_T) - MEMBUFTYPEBITS - MEMBUFFLAGBITS)
#if 0
#define MEMBUFNBLKBITS   16
//#define MEMBUFMAXBLKS    (MEMWORD(1) << MEMBUFNBLKBITS)
#define MEMBUFSLOTSHIFT  (MEMBUFNBLKBITS)
#define MEMBUFTYPESHIFT  (MEMBUFSLOTSHIFT + MEMBUFSLOTBITS)
#endif
#if 0
#define MEMBUFNFREEBITS  MEMBUFNBLKBITS
#define MEMBUFNFREEMASK  (MEMBUFNBLKMASK << MEMBUFNFREESHIFT)
#define MEMBUFNFREESHIFT MEMBUFNBLKBITS
#endif

#define meminitbufnblk(buf, n)                                          \
    ((buf)->nblk = (n))
#define meminitbuftype(buf, t)                                          \
    ((buf)->type = (t))
#define meminitbufslot(buf, slot)                                       \
    ((buf)->slot = (slot))
#if 0
#define meminitbuftype(buf, t)                                          \
    ((buf)->flg |=  (t))
#define meminitbufslot(buf, slot)                                       \
    ((buf->flg) |= (slot) << MEMBUFTYPEBITS)
#endif
#define memsetbufnfree(buf, n)                                          \
    ((buf)->nfree = (n))
#define memgetbufheapflg(buf)                                           \
    ((buf)->flg & MEMHEAPBIT)
#define memgetbufnblk(buf)                                              \
    ((buf)->nblk)
#define memgetbufnfree(buf)                                             \
    ((buf)->nfree)
#define memgetbuftype(buf)                                              \
    ((buf)->type)
#define memgetbufslot(buf)                                              \
    ((buf)->slot)

#define memsetblk(ptr, buf, id) membufop(ptr, MEMBUFADD, buf, id)
#define memdelblk(ptr)          membufop(ptr, MEMBUFDEL, NULL, 0)
#define memchkblk(ptr)          membufop(ptr, MEMBUFCHK, NULL, 0)
#define memchkpad(ptr)          membufop(ptr, MEMGETPAD, NULL, 0)
#if (MEMBLKHDR)
#define memblkhdrsize()        (sizeof(struct memblkhdr))
#define memputblkdesc(ptr, d)  (((MEMADR_T *)(ptr))[-2] = (d))
#define memgetblkdesc(ptr)     (((MEMADR_T *)(ptr))[-2])
#define memputblkpad(ptr, pad) (((MEMADR_T *)(ptr))[-1] = (pad))
#define memgetblkpad(ptr)      (((MEMADR_T *)(ptr))[-1])

struct memblkhdr {
    MEMADR_T desc;
    MEMADR_T pad;
};
#endif

#if 0
struct membufvals {
    MEMWORD_T *nblk[MEMBUFTYPES];
    MEMWORD_T *ntls[MEMBUFTYPES];
    MEMWORD_T *nglob[MEMBUFTYPES];
};
#endif

#define MEMINITBIT (1L << 0)
#define MEMHEAPBIT (1L << 1)
struct mem {
    /* global allocation freelists */
    struct membkt        pagebin[MEMPAGESLOTS];
    struct membkt        smallbin[MEMSMALLSLOTS];
    struct membkt        bigbin[MEMBIGSLOTS];
#if (MEMDEADBINS)
    struct membkt        deadpage[MEMPAGESLOTS];
    struct membkt        deadsmall[MEMSMALLSLOTS];
#endif
    /* allocation header buffers */
    struct membuf       *maphdr;
    struct membuf       *stkhdr;
#if 0
    struct membuf       *stkbuf;
    struct membuf       *bigbuf;
#endif
//    struct membufvals   bufvals;
#if (MEMHASHSUBTABS)
    struct memhashbkt  **hash;
    struct memhash      *hashbuf;
#elif (!MEMBLKHDR)
    struct memhash      *hash;    // hash table
    struct memhash      *hashbuf; // buffer for hash items
#endif
    MEMWORD_T            flg;     // memory interface flags
//    MEMWORD_T            nbytetab[MEMBUFTYPES];
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

#define MEMNOBLK (MEMWORD(-1))

#if 0
struct membufmap {
    MEMWORD_T freemap[MEMBUFBITMAPWORDS];
    MEMWORD_T relmap[MEMBUFBITMAPWORDS];
};
#endif

struct membuf {
    volatile struct memtls *tls;
    MEMPTR_T                base; // base address for allocations
//    MEMWORD_T               info; // slot + buffer type
    struct membuf          *heap; // previous buf in heap for bufs from sbrk()
    struct membuf          *prev; // previous buf in chain
    struct membuf          *next; // next buf in chain
    MEMWORD_T               size; // allocation blocks
    MEMWORD_T               flg;
    MEMWORD_T               type;
    MEMWORD_T               slot;
    MEMWORD_T               nblk;
    MEMWORD_T               nfree;
    MEMWORD_T               stktop;
    MEMWORD_T               stklim;
    MEMBLKID_T             *stk;
    MEMWORD_T               relmap[MEMBUFBITMAPWORDS];
    MEMWORD_T              *freemap;
};

#define membufhdrsize()                                                 \
    (rounduppow2(sizeof(struct membuf), PAGESIZE))
#define membufstksize()                                                 \
    (MEMBUFMAXBLKS * sizeof(MEMBLKID_T))
#define membufmapsize(nblk)                                             \
    (rounduppow2((nblk) * sizeof(MEMBLKID_T), PAGESIZE))

#define memtlssize() rounduppow2(sizeof(struct memtls), 8 * PAGESIZE)
struct memtls {
    struct membkt     pagebin[MEMPAGESLOTS];
    struct membkt     smallbin[MEMSMALLSLOTS];
    struct membkt     bigbin[MEMBIGSLOTS];
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
     : (PAGESIZE + PAGESIZE * (slot)))

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
#define MEMHASHBITS        21
#else
#define MEMHASHBITS        18
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

#if (MEMHASHMURMUR)
#define memhashptr(page) MurmurHash3Mixer((MEMADR_T)page)
#elif (WORDSIZE == 4)
#define memhashptr(page)                                                \
    (tmhash32((MEMADR_T)page))
#elif (WORDSIZE == 8)
#define memhashptr(page)                                                \
    (tmhash64((MEMADR_T)page))
#endif

#if (MEMNEWHASH)

struct memhashlist {
    m_atomic_t      lk;
    struct memhash *chain;
    uint8_t         _pad[CLSIZE - sizeof(MEMLK_T) - sizeof(struct memchain *)];
};

#if 0
struct memhashsubitem {
    MEMADR_T val;
};
#endif

#endif /* MEMNEWHASH */

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
    MEMWORD_T  n = sizeof(MEMWORD_T) * CHAR_BIT;
    MEMWORD_T *ptr = buf->freemap;

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
membufscanblk(struct membuf *buf, MEMWORD_T *nfreeret)
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
                res = --buf->nfree;
                word &= mask;                   // mask the bit out
                buf->nfree = res;
                *map = word;                    // update
                *nfreeret = res;

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
    MEMWORD_T   nfree = buf->nfree;
    MEMWORD_T  *map = buf->freemap;
    MEMUWORD_T  word = id;
#if (MEMBUFMAXBLKS <= 65536)
    MEMWORD_T   pos = id & (0x03);
#else
    MEMWORD_T   pos = id & (0x0f);
#endif
    MEMWORD_T   mask = MEMWORD(1) << pos;

#if (MEMBUFMAXBLKS <= 65536)
    word >>= 2;
#else
    word >>= 4;
#endif
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
    MEMBLKID_T *stk = buf->stk;
    MEMWORD_T   ndx;
    MEMBLKID_T  id;

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
#define memusesmallbuf(sz)     ((sz) <= PAGESIZE)
#define memusepagebuf(sz)      ((sz) <= PAGESIZE * MEMPAGESLOTS)
#define memsmallbufsize(slot, nblk)                                     \
    (rounduppow2(membufhdrsize() + membufstksize() + ((nblk) << (slot)), \
                 PAGESIZE))
#define mempagebufsize(slot, nblk)                                      \
    (rounduppow2(membufhdrsize() + membufmapsize(nblk)                  \
                + (PAGESIZE + PAGESIZE * (slot)) * (nblk),              \
                 PAGESIZE))
#define membigbufsize(slot, nblk)                                       \
    (rounduppow2(membufhdrsize() + membufmapsize(nblk)                  \
                + (PAGESIZE + PAGESIZE * (slot)) * (nblk),              \
                 PAGESIZE))
#if 0
#define memnbufblk(type, slot)                                          \
    (((type) == MEMSMALLBUF)                                            \
     ? (1L << (MEMSLABSHIFT - (slot)))                                  \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? (1L << (MEMSMALLPAGELIM - (slot)))                         \
           : (((slot) <= MEMMIDPAGESLOT)                                \
              ? (1L << (MEMMIDPAGELIM - (slot)))                        \
              : (((slot) <= MEMBIGPAGESLOT)                             \
                 ? (1L << (MEMBIGPAGELIM - (slot)))                     \
                 : 1)))                                                 \
        : (((slot) <= MEMSMALLMAPSLOT)                                  \
           ? (1L << (MEMSMALLMAPLIM - (slot)))                          \
           : (((slot) <= MEMMIDMAPSLOT)                                 \
              ? (1L << (MEMMIDMAPLIM - (slot)))                         \
              : (((slot) <= MEMBIGMAPSLOT)                              \
                 ? (1L << (MEMBIGMAPLIM - (slot)))                      \
                 : 1)))))
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
                 ? 8                                                    \
                 : 4)))                                                 \
        : (((slot) <= MEMSMALLMAPSLOT)                                  \
           ? 8                                                          \
           : (((slot) <= MEMMIDMAPSLOT)                                 \
              ? 4                                                       \
              : (((slot) <= MEMBIGMAPSLOT)                              \
                 ? 2                                                    \
                 : 1)))))

#define membktnbuftls(type, slot)                                       \
    (((type) == MEMSMALLBUF)                                            \
     ? 2                                                                \
     : 1)
#if (!MEMUNMAP)
#define membktnbufglob(type, slot) (~MEMWORD(0))
#else
#define membktnbufglob(type, slot)                                      \
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
#if 0
#define membktnbufglob(type, slot)                                      \
    (((type) == MEMSMALLBUF)                                            \
     ? (((slot) <= MEMSMALLSLOT)                                        \
        ? 8                                                             \
        : (((slot) <= MEMMIDSLOT)                                       \
           ? 4                                                          \
           : 2))                                                        \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? 8                                                          \
           : (((slot) <= MEMMIDPAGESLOT)                                \
              ? 4                                                       \
              : 2))                                                     \
        : (((slot) <= MEMSMALLMAPSLOT)                                  \
           ? 8                                                          \
           : (((slot) <= MEMMIDMAPSLOT)                                 \
              ? 4                                                       \
              : (((slot) <= MEMBIGMAPSLOT)                              \
                 ? 2                                                    \
                 : 1)))))
#endif
#endif
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
#endif
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
#if (!MEMUNMAP)
#define membktnbufglob(type, slot) (~MEMWORD(0))
#else
#define membktnbufglob(type, slot)                                      \
    (((type) == MEMSMALLBUF)                                            \
     ? (((slot) <= MEMSMALLSLOT)                                        \
        ? 16                                                            \
        : (((slot) <= MEMMIDSLOT)                                       \
           ? 8                                                         \
           : 4))                                                        \
     : (((type) == MEMPAGEBUF)                                          \
        ? (((slot) <= MEMSMALLPAGESLOT)                                 \
           ? 4                                                         \
           : (((slot) <= MEMMIDPAGESLOT)                                \
              ? 8                                                      \
              : 16))                                                    \
        : (((slot) <= MEMSMALLMAPSLOT)                                  \
           ? 2                                                          \
           : (((slot) <= MEMMIDMAPSLOT)                                 \
              ? 4                                                       \
              : (((slot) <= MEMBIGMAPSLOT)                              \
                 ? 8                                                   \
                 : 2)))))
#endif
#endif

#define memgetnbufblk(type, slot)  memnbufblk(type, slot)
#define memgetnbuftls(type, slot)  memnbuftls(type, slot)
#define memgetnbufglob(type, slot) memnbufglob(type, slot)

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

#define memalignptr(ptr, pow2)                                          \
    ((MEMPTR_T)rounduppow2((uintptr_t)(ptr), (pow2)))
#define memadjptr(ptr, sz)                                              \
    (&((MEMPTR_T)(ptr))[sz])

#if (MEMCACHECOLOR)

static __inline__ MEMWORD_T *
memgentlsadr(MEMWORD_T *adr)
{
    /* division by 9 */
    MEMADR_T res = (MEMADR_T)adr;
    MEMADR_T q;
    MEMADR_T r;
    MEMADR_T div9;
    MEMADR_T dec;
    MEMADR_T ofs;

    /* compute adr + (adr >> 16) % 9; # of offset cachelines, aligned to cl */
    res >>= 16;
    /* divide by 9 */
    q = res - (res >> 3);
//    ofs = res;
    q = q + (q >> 6);
//    ofs &= 0x07;
    q = q + (q >> 12) + (q >> 24);
//    ofs = max(0, --ofs);
    q = q >> 3;
//    ofs <<= PAGESIZELOG2;
    r = res - q * 9;
    div9 = q + ((r + 7) >> 4);
    /* calculate res -= res/9 * 9 i.e. res % 9 (max 8) */
    dec = div9 * 9;
    res -= dec;
//    adr += ofs;
    /* scale to 0..256 (machine words) */
    res <<= 5;
    /* align to cacheline */
    res &= ~(CLSIZE - 1);
    /* add to original pointer */
    adr += res;

    return adr;
}

#if 0
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
#endif

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

static __inline__ MEMPTR_T
memcalcadr(MEMPTR_T ptr, MEMWORD_T size,
           MEMWORD_T blksz, MEMWORD_T align,
           MEMADR_T *padret)
{
    MEMPTR_T  adr = ptr;
    MEMADR_T diff = blksz - size;
    MEMADR_T  ofs = memgenofs(ptr);

    if (align <= CLSIZE && diff >= ofs) {
        ptr += ofs;
    } else {
        ptr = memalignptr(adr, align);
        ofs = ptr - adr;
    }
    if (padret) {
        *padret = ofs;
    }

    return ptr;
}

#else

static __inline__ MEMPTR_T
memcalcadr(MEMPTR_T ptr, MEMWORD_T size,
           MEMWORD_T blksz, MEMWORD_T align,
           MEMADR_T *padret)
{
    MEMPTR_T adr = ptr;
    MEMADR_T ofs;

    ptr = memalignptr(adr, align);
    if (padret) {
        ofs = ptr - adr;
        *padret = ofs;
    }

    return ptr;
}

#define memgentlsadr(adr)                   (adr)
#define memgenadr(adr)                      (adr)
#define memgenofs(adr)                      (adr)
#define memgenhashtabadr(adr)               (adr)

#endif /* MEMCACHECOLOR */

#endif /* __ZERO_MEM_H__ */

