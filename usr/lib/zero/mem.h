#ifndef __ZERO_MEM_H__
#define __ZERO_MEM_H__

#define MEMMINSLABSIZE (1024 * 1024)

/* internal allocator parameters */

#define MEMBUFHDRSIZE  (MEMBUFTABOFS + MEMBUFTABSIZE)
/* allocation buffer types */
#define MEMSMALLBUF    0x00 // cacheline multiples
#define MEMPAGEBUF     0x01 // page multiples
#define MEMBIGBUF      0x02 // big allocations
#define MEMBINTYPES    8
/* numbers of buckets for allocation types */
#define MEMMAXSIZES    (MEMWORD_T(1) << MEMBUFBINBITS)
#define MEMSMALLSIZES  (PAGESIZE / CLSIZE)
#define MEMPAGESIZES   min(MEMMINSLABSIZE / PAGESIZE, MEMMAXSIZES)
#define MEMBIGSIZES    MEMPTRBITS
/* queue parameters */
#define MEMSLOTQUEUES  16 // number of queues per slot; 2^x, x > 0
/* buffer/slab parameters */
/* maximum number of blocks in buffer */
#define MEMBUFMAXBLKS  (MEMBUFTABSIZE / PTRSIZE)
#define MEMBUFBINBITS  8 // number of bits for bucket/size-IDs
#define MEMBINTYPEBITS 4 // number of bits for type/zone-IDs
#define MEMBUFTABSIZE  (4 * PAGESIZE)
#define MEMBUFTABOFS   (4 * PAGESIZE)

/* macros */

/* construct info-member for struct membuf */
#define membufmkinfo(type, bin)                                        \
    (((type) << _MEMBINTYPEOFS) | (bin))
/* extract buffer header info-member values */
#define _MEMBUFBINMASK  ((MEMWORD(1) << MEMBUFBINBITS) - 1)
#define _MEMBINTYPEOFS  MEMBUFBINBITS
#define _MEMBINTYPEMASK ((MEMWORD(1) << MEMBINTYPEBITS) - 1)
#define membufbin(buf)  ((buf)->info & _MEMBUFBINMASK)
#define membuftype(buf) (((buf)->info >> _MEMBINTYPEOFS) & _MEMBINTYPEMASK)
/* block size calculations */
#define memsmallblksize(buf ((membufbin(buf) + 1) * CLSIZE)
#define mempageblksize(buf) ((membufbin(buf) + 1) * PAGESIZE)
#define membigblksize(buf)  (MEMWORD(1) << membufbin(buf))
#define membufblksize(buf)                                              \
    (((membufbin(buf) == MEMSMALLBUF)                                   \
      ? (memsmallblksize(bin))                                          \
      : (((membufbin(buf) == MEMCLBUF)                                  \
          ? (memcacheblksize(buf))                                      \
          : (((membufbin(buf) == MEMPAGEBUF)                            \
              ? (mempageblksize(buf))                                   \
              : (membigblksize(buf))))))))
/* calculate address for buffer block */
#define membufblkadr(buf, ndx) (membufblksize(buf) * (ndx))
#define membufblkid(buf, ptr)  (
#define membufstk(buf)         ((MEMBLK_T *)(buf->stk[0]))
#define membuftab(buf)         ((MEMPTR_T *)((MEMPTR_T)(buf) + MEMBUFTABOFS))

/* forward-declaration */
struct membufslot;

/* allocation block buffer header */
#define MEMBUFHDRSIZE (offsetof(struct membuf, stk))
struct membuf {
    /* 8 machine words; struct membuf should be exact multiple of cacheline */
    volatile struct memtls *tls;
    struct membufbin       *bin;
    struct membuf          *prev;
    struct membuf          *next;
    MEMWORD_T               info;
    MEMPTR_T                base;
    m_atomic_t              ndx;
    volatile MEMWORD_T      nblk;
    /* end of header information */
    uint8_t                 stk[EMPTY];
    /* allocation pointer table at offset MEMBUFTABOFS from struct beginning */
};

/* global memory slot of queues */

#define MEMBUFSLOTSIZE  (WORDSIZE + 2 * PTRSIZE)
struct membufslot {
    volatile MEMWORD_T      nbuf;
    volatile struct membuf *free;
    volatile struct membuf *dead;
    uint8_t                 _pad[CLSIZE - MEMBUFSLOTSIZE];
};

#define memslotadr(type, bin)                                           \
    (((type) == MEMSMALLBUF)                                            \
     ? ((&g_mem.smalltab[(bin)][0])                                     \
        : ((type == MEMPAGEBUF)                                         \
           ? ((&g_mem.pagetab[(bin)][0]))                               \
           : (&g_mem->bigtab[(bin)][0]))))
#define memslotadrtls(tls, type, bin)                                   \
    (((type) == MEMSMALLBUF)                                            \
     ? (&(tls->smalltab[(bin)])                                         \
        : ((type == MEMPAGEBUF)                                         \
           ? (&(tls->pagetab[(bin)]))                                   \
           : (&(tls->bigtab[(bin)])))))

#define MEMTLSSIZE rounduppow2(sizeof(struct memtls), 2 * PAGESIZE)
struct memtls {
    struct membuf *smalltab[MEMSMALLSIZES];
    struct membuf *pagetab[MEMPAGESIZES];
    struct membuf *bigtab[MEMBIGSIZES];
};

struct mem {
    struct membuf *smalltab[MEMSMALLSIZES][MEMSLOTQUEUES];
    struct membuf *pagetab[MEMPAGESIZES][MEMSLOTQUEUES];
    struct membuf *bigtab[MEMBIGSIZES][MEMSLOTQUEUES];
};

static __inline__ void *
memlkbit(m_atomic_t *ptr, long bin)
{
    m_atomic_t ret = MEMLKFAIL;

    if (m_cmpsetbit((m_atomic_t *)ptr, bin)) {
        ret = *ptr;
        ret &= ~MEMADRLKBIT;
    }

    return (void *)ret;
}

/*
 * ASSUMPTIONS
 * -----------
 * - ptr is aligned to
 */
static __inline__ void *
memzeroblk(void *ptr, size_t size)
{
    MEMWORD_T *wp = ptr;
    MEMWORD_T  zero = MEMWORD(0);
    MEMWORD_T  nw = CLSIZE - ((MEMADR_T)ptr & ((MEMWORD(1) << CLSIZELOG2) - 1));
    size_t     ncl;

    size -= n * sizeof(MEMWORD_T);
    while (nw--) {
        *wp++ = zero;
    }
#if (WORDSIZE == 8)
    nw = (size & ((MEMWORD(1) << CLSIZELOG2) - 1)) >> 3;
#else
    nw = (size & ((MEMWORD(1) << CLSIZELOG2) - 1)) >> 2;
#endif
    ncl = size >> CLSIZELOG2;
    while (ncl--) {
#if (CLSIZE >= 4 * WORDSIZE)
        wp[0] = zero;
        wp[1] = zero;
        wp[2] = zero;
        wp[3] = zero;
#if (CLSIZE >= 8 * WORDSIZE)
        wp[4] = zero;
        wp[5] = zero;
        wp[6] = zero;
        wp[7] = zero;
        wp += 8;
#else
        wp += 4;
#endif
#endif
    }
    while (nw--) {
        *wp++ = zero;
    }

    return;
}

/* cache-color thread-local data */
static __inline__ void *
memgentlsadr(MEMPTR_T adr)
{
    MEMADR_T val = (MEMADR_T)adr;
    MEMADR_T ofs;

    memrandofs(ofs);
    val = CLSIZE;
    ofs &= 15;
    val *= ofs;
    adr += val;

    return adr;
}

/* cache color slot */
static __inline__ struct memslot *
memrandslot(struct MEMPTR_T adr)
{
    MEMADR_T  ofs;

    memrandofs(ofs);
    adr = (uint8_t *)slot;
    ofs &= (MEMSLOTQUEUES >> 1) - 1;
    adr += ofs;

    return slot;
}

static __inline__ void
mempushbuf(struct membuf *buf, struct membuf **queue)
{
    struct membuf *head;
    MEMADR_T       hval;

    do {
        while ((MEMADR_T)(*queue)->tail & MEMLKBIT) {
            m_waitspin();
        }
        if (!m_cmpsetbit((m_atomic_t *)queue,
                     MEMLKBITPOS)) {
            head = *queue;
            hval = (MEMADR_T)head;
            buf->prev = NULL;
            hval &= ~MEMLKBIT;
            if ((*queue)->tail == lkval) {
                ((struct membuf *)hval)->prev = buf;
                m_atomwrite((m_atomic_t *)queue,
                        (m_atomic_t)buf);

                return;
            }
        }
    } while (1);

    /* NOTREACHED */
    return;
}

static __inline__ void
mempushchain(struct membuf *first, struct membuf *last, struct membuf **queue)
{
    struct membuf *head;
    MEMADR_T       hval;

    do {
        while ((MEMADR_T)(*queue)->tail & MEMLKBIT) {
            m_waitspin();
        }
        if (!m_cmpsetbit((m_atomic_t *)queue, MEMLKBITPOS)) {
            head = *queue;
            hval = (MEMADR_T)head;
            first->prev = NULL;
            last->next = hval;
            tval &= ~MEMLKBIT;
            last->next = (struct membuf *)hval;
            m_atomwrite((m_atomic_t *)queue,
                        (m_atomic_t)first);
        }
    } while (1);

    return;
}

static __inline__ void
mempushbuftls(struct membuf *buf, struct membuf **queue)
{
    struct membuf *head = *queue;

    buf->next = head;
    if (head) {
        head->prev = buf;
    }
    *queue = buf;

    return;
}

#endif /* __ZERO_MEM_H__ +/

