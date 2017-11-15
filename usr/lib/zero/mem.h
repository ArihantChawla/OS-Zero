#ifndef __ZERO_BITS_MEM_H__
#define __ZERO_BITS_MEM_H__

#define MEMMINSLABSIZE   (1024 * 1024)

/* internal allocator parameters */

/* header size with block bitmap */
#define MEMBITMAPHDRSIZE (sizeof(struct membuf))
/* header size with block stack */
#define MEMSTKHDRSIZE    (4 * PAGESIZE)
/* allocation buffer types */
#define MEMSMALLBUF      0x00 // cacheline multiples
#define MEMPAGEBUF       0x01 // page multiples
#define MEMBIGBUF        0x02 // big allocations
/* numbers of buckets for allocation types */
#define MEMMAXBKTS       (MEMWORD_T(1) << MEMBUFBKTBITS)
#define MEMSMALLBKTS     (PAGESIZE / CLSIZE)
#define MEMPAGEBKTS      min(MEMALLOCMIN / PAGESIZE, MEMMAXBKTS)
#define MEMBIGBKTS       MEMPTRBITS
/* queue parameters */
#define MEMBUFSLOTQUEUES 16 // number of queues per slot
/* buffer/slab parameters */
/* maximum number of blocks in buffer */
#define MEMBUFMAXBLKS    ((MEMSTKSHDRSIZE - MEMBUFPTRTABOFS) / PTRSIZE)
#define MEMBUFBKTBITS    8
#define MEMBUFTYPEBITS   4
#define MEMBUFPTRTABOFS  2 * PAGESIZE)

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
#define membufstk(buf)         ((MEMBLK_T *)(&buf->tab[0]))
#define membufptrtab(buf)      ((MEMWORD_T *)((MEMPTR_T)(buf)           \
                                             + MEMBUFPTRTABOFS))

/* forward-declaration */
struct membufslot;

/* allocation block buffer header */
struct membuf {
    struct memtls     *tls;
    struct membufslot *slot;
    struct membuf     *prev;
    struct membuf     *next;
    MEMWORD_T          info;
    MEMPTR_T           base;
    m_atomic_t         ndx;
    MEMWORD_T          nblk;
    uint8_t            tab[EMPTY];
    /* allocation pointer table at offset MEMBUFTABOFS from struct beginning */
};

/* memory buffer slot header */

struct membufqueue {
    m_atomic_t              nref;
    m_atomic_t              nblk;
    volatile struct membuf *head;
    volatile struct membuf *tail;
};

#define membufqueue(slot, ndx) (&((slot)->qtab[(ndx)].hdr))
struct membufslot {
    union {
        struct membufqueue hdr;
        uint8_t            _pad[CLSIZE];
    } qtab[MEMBUFSLOTQUEUES];
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

static __inline__ MEMPTR_T
memgenadr(MEMPTR_T adr, MEMADR_T pos, MEMADR_T nmax, MEMADR_T scale)
{
    MEMADR_T val = (MEMADR_T)adr;
    MEMADR_T rnd;

    rnd = memrandofs();
    val = fastumod8(rnd, nmax);
    val <<= scale;
    adr += val;

    return adr;
}

static __inline__ MEMPTR_T
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

    return adr;
}

static __inline__ membufqueue *
memfindqueue(struct membuf *buf)
{
    struct memtls     *tls = memlkbit((m_atomic_t *)&buf->tls, MEMLKBITPOS);
    MEMWORD_T          bkt = membufbkt(buf);
    MEMWORD_T          type = membuftype(buf);
    struct membufslot *slot;

    if ((tls) && tls == t_memtls) {
        /* FASTPATH */
        tab = tls->bufqtab[type];
        queue = &tab[bkt];

        return slot;
    } else if (!tls) {
        /* global */
    } else {
        ;
    }

    return queue;
}

static __inline__ void
mempushbuf(struct membuf *buf)
{
    struct membufslot *slot = memfindqueue(buf);
    struct membuf     *head = NULL;
    struct membuf     *tail = NULL;
    struct membuf     *bptr;

    buf->prev = NULL;
    buf->next = NULL;
    head = memlkbit(&slot->head, MEMLKBITPOS);
    tail = m_atomread(&slot->tail);
    if (!tail && !m_cmpswapptr((m_atomic_t **)&slot->tail, NULL, buf)) {
        memlkbit((m_atomic_t *)&slot->tail, MEMLKBITPOS);
    } else if (!m_atomread(&slot->nblk)) {
        tail = memlkbit((m_atomic_t *)&slot->tail, MEMLKBITPOS);
        if (!tail) {
            m_atomwrite(&slot->tail, buf);
        } else {
            memrellkbit(&slot->tail);
        }
    }
    buf->next = head;
    if (head) {
        head->prev = buf;
    }
    m_atominc((m_atomic_t *)&slot->nblk);
    m_atomwrite((m_atomic_t *)&slot->head, (m_atomic_t)buf);

    return;
}

static __inline__ void
memaddblk(struct membuf *buf, vpid *ptr)
{
    MEMWORD_T nblk = buf->nblk;
    MEMWORD_T ndx = memgetblkid(buf, ptr);

    ndx = m_fetchadd(&buf->nblk, 1);
    if (!ndx) {
        mempushbuf(buf);
    } else if (ndx == nblk) {
        memslot(buf);
    }

    return;
}

#endif /* __ZERO_BITS_MEM_H__ +/

