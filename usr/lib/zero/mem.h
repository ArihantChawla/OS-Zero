#ifndef __ZERO_MEM_H__
#define __ZERO_MEM_H__

#define MEMMINSLABSIZE (1024 * 1024)

/* internal allocator parameters */

#define MEMBUFHDRSIZE  (MEMBUFTABOFS + MEMBUFTABSIZE)
/* allocation buffer types */
#define MEMSMALLBUF    0x00 // cacheline multiples
#define MEMPAGEBUF     0x01 // page multiples
#define MEMBIGBUF      0x02 // big allocations
#define MEMBUFTYPES    8
/* numbers of buckets for allocation types */
#define MEMMAXSIZES    (MEMWORD_T(1) << MEMBUFBKTBITS)
#define MEMSMALLSIZES  (PAGESIZE / CLSIZE)
#define MEMPAGESIZES   min(MEMMINSLABSIZE / PAGESIZE, MEMMAXSIZES)
#define MEMBIGSIZES    MEMPTRBITS
/* queue parameters */
#define MEMSLOTQUEUES  16 // number of queues per slot; 2^x, x > 0
/* buffer/slab parameters */
/* maximum number of blocks in buffer */
#define MEMBUFMAXBLKS  (MEMBUFTABSIZE / PTRSIZE)
#define MEMBUFBKTBITS  8 // number of bits for bucket/size-IDs
#define MEMBUFTYPEBITS 4 // number of bits for type/zone-IDs
#define MEMBUFTABSIZE  (4 * PAGESIZE)
#define MEMBUFTABOFS   (4 * PAGESIZE)

/* macros */

/* construct info-member for struct membuf */
#define membufmkinfo(type, bkt)                                        \
    (((type) << _MEMBUFTYPEOFS) | (bkt))
/* extract buffer header info-member values */
#define _MEMBUFBKTMASK  ((MEMWORD(1) << MEMBUFBKTBITS) - 1)
#defube _MEMBUFTYPEOFS  MEMBUFBKTBITS
#define _MEMBUFTYPEMASK ((MEMWORD(1) << MEMBUFTYPEBITS) - 1)
#define membufbkt(buf)  ((buf)->info & _MEMBUFBKTMABSK)
#define membuftype(buf) (((buf)->info >> _MEMBUFTYPEOFS) & _MEMBUFTYPEMASK)
/* block size calculations */
#define memsmallblksize(bkt) (((bkt) + 1) * CLSIZE)
#define mempageblksize(bkt)  (((bkt) + 1) * PAGESIZE)
#define membigblksize(bkt)   (MEMWORD(1) << (bkt))
#define membufblksize(buf)                                              \
    (((membuftype(buf) == MEMSMALLBUF)                                  \
      ? (memsmallblksize(buf))                                          \
      : (((membuftype(buf) == MEMCLBUF)                                 \
          ? (memcacheblksize(buf))                                      \
          : (((membuftype(buf) == MEMPAGEBUF)                           \
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
    struct membufslot      *slot;
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

/* memory buffer queue header */
#define MEMBUFQUEUESIZE (2 * PTRSIZE)
struct membufqueue {
    volatile struct membuf *prev;
    volatile struct membuf *next;
};

#define MEMBUFSLOTSIZE (PTRSIZE + WORDSIZE + 2 * MEMBUFQUEUESIZE)
struct membufslot {
    volatile struct memtls *tls;
    volatile MEMWORD_T      nbuf;
    struct membufqueue      head;
    struct membufqueue      tail;
    uint8_t                 _pad[CLSIZE - MEMBUFSLOTSIZE];
};

#define membufqueue(slot, ndx) (&((slot)->qtab[(ndx)].hdr))
struct membufbkt {
    struct membufslot tab[MEMBUFSLOTS];
};

#define MEMTLSSIZE rounduppow2(sizeof(struct memtls), 2 * PAGESIZE)
struct memtls {
    struct membufslot  smalltab[MEMSMALLSIZES];
    struct membufslot  pagetab[MEMPAGESIZES];
    struct membufslot  bigtab[MEMBIGSIZES];
    struct membufslot *buftab[MEMBUFTYPES];
};

struct mem {
    struct membufbkt  smalltab[MEMSMALLSIZES];
    struct membufbkt  pagetab[MEMPAGESIZES];
    struct membufbkt  bigtab[MEMBIGSIZES];
    struct membufbkt *buftab[MEMBUFTYPES];
};

static __inline__ void *
memlkbit(m_atomic_t *ptr, long ndx)
{
    m_atomic_t ret = MEMLKFAIL;

    if (m_cmpsetbit((m_atomic_t *)ptr, ndx)) {
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

static __inline__ struct memtls *
memgentlsadr(MEMPTR_T adr)
{
    MEMADR_T val = (MEMADR_T)adr;
    MEMADR_T ofs;
    MEMADR_T rnd;

    rnd = memrandofs();
    ofs = CLSIZE;
    rnd &= 15;
    ofs *= rnd;
    adr += ofs;

    return (struct memtls *)adr;
}

static __inline__ struct memslot *
memrandslot(struct membufslot *slot)
{
    MEMADR_T rnd;

    rnd = memrandofs();
    rnd &= MEMBUFQUEUESLOTS - 1;
    slot += rnd;

    return slot;
}

static __inline__ membufslot *
memfindslot(struct membuf *buf)
{
    MEMWORD_T           bkt = membufbkt(buf);
    MEMWORD_T           type = membuftype(buf);
    struct membufslot  *tab;
    struct membufslot  *slot = &buf->buftab[type][bkt];

    return slot;
}

static __inline__ void
mempushbuf(struct membuf *buf, struct membufslot *slot, long flg)
{
    struct membuf *head = NULL;
    struct membuf *next = NULL;
    struct membuf *tail = NULL;
    struct membuf *prev = NULL;

    if (!(flg & MEMQUEUEGLOBAL)) {
        /* FASTPATH: thread local buffer */
        head = slot->head.next;
        if (head) {
            head->prev = buf;
        }
        buf->next = head;
        slot->head->next = buf;
    } else {
        buf->prev = NULL;
        head = memlkbit((m_atomic_t *)&slot->head.next, MEMLKBITPOS);
        if (head) {
            head->prev = buf;
        }
        if ((head) && (next = memlkbit((m_atomic_t *)&head->next,
                                       MEMLKBITPOS))) {
            buf->next = head;
            memunlkbit((m_atomic_t *)&head->next, MEMLKBITPOS);
        } else if (head) {
            memunlkbit((m_atomic_t *)&head->next, MEMLKBITPOS);
        } else {
            tail = memlkbit((m_atomic_t *)&slot->tail.next, MEMLKBITPOS);
            if (!tail) {
                slot->tail.prev = NULL;
                m_atomwrite((m_atomic_t *)&slot->tail.next, buf);
            } else {
                slot->tail.prev = buf;
                m_atomwrite((m_atomic_t *)&slot->tail.next, tail);
            }
        }
        m_atominc((m_atomic_t *)&slot->nbuf);
        m_atomwrite((m_atomic_t *)&slot->head.next, buf);
    }

    return;
}

static __inline__ void
memaddbuf(struct membuf *buf, struct memtls *tls)
{
    struct membufslot *slot = NULL;
    MEMWORD_T          type = membuftype(buf);
    MEMWORD_T          bkt = membufbkt(buf);
    MEMWORD_T          nbuf;
    MEMWORD_T          flg;

    flg = MEMQUEUEGLOBAL;
    if (tls) {
        slot = tls->buftab[type];
        nbuf = m_atomdec(&slot->nbuf);
        if (nbuf >= 2) {
            slot = NULL;
        } else {
            slot = &slot[bkt];
        }
    }
    if (!slot) {
        tls = t_memtls;
        slot = tls->buftab[type];
        nbuf = m_atomdec(&slot->nbuf);
        if (nbuf >= 2) {
            tls = NULL;
        } else {
            slot = &slot[bkt];
        }
    }
    if (!slot) {
        flg &= ~MEMQUEUEGLOBAL;
        slot = memfindslot(buf);
        nbuf = m_atomdec(&slot->nbuf);
    }
    memqueuebuf(buf, slot, flg);
}

static __inline__ void
mempushblk(struct membuf *buf, void *ptr)
{
    MEMWORD_T  nblk;
    MEMBLK_T   id = membufblkid(buf, ptr);
    MEMPTR_T  *tab = membuftab(buf);
    MEMBLK_T  *stk = membufstk(buf);
    MEMWORD_T  lim;
    MEMWORD_T  top;

    top = m_atomdec(&buf->ndx);
    nblk = m_atomread(&buf->nblk);
    lim = nblk;
    top--;
    lim--;
    stk[top] = id;
    if (top == 0) {
        memlkbit((m_atomic_t *)&buf->tls, MEMLKBITPOS);
        if (!m_atomread(&buf->ndx)) {
            memfreebuf(buf, tls);
        }
    } else if (top == lim) {
        memlkbit((m_atomic_t *)&buf->tls, MEMLKBITPOS);
        if (m_atomread(&buf->ndx) == lim) {
            memaddbuf(buf, tls);
        }
    }

    return;
}

#endif /* __ZERO_MEM_H__ +/

