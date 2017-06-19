#include <stddef.h>
#include <stdint.h>
#include <kern/util.h>
#include <kern/malloc.h>
#include <kern/mem/membuf.h>

/* convenience macro to allocate buffers */
#define __memgetbuf(buf, nb, how)                                       \
    do {                                                                \
        if ((how) & MEM_WAIT) {                                         \
            buf = kwtwalloc(nb);                                        \
        } else {                                                        \
            buf = kwalloc(nb);                                          \
        }                                                               \
        if (buf) {                                                      \
            if (how & MEM_ZERO) {                                       \
                kbzero(buf, nb);                                        \
            } else {                                                    \
                buf->hdr.len = 0;                                       \
                buf->hdr.flg = 0;                                       \
                buf->hdr.next = NULL;                                   \
                buf->hdr.nextpkt = NULL;                                \
            }                                                           \
            buf->hdr.data = mbdata(buf);                                \
        }                                                               \
    } while (0)
#define __memgetblk(buf, nb, how)                                       \
    do {                                                                \
        struct kmembuf *_mb;                                            \
        struct kmemext *_ext;                                           \
                                                                        \
        if ((how) & MEM_WAIT) {                                         \
            _mb = kwtwalloc(nb);                                        \
        } else {                                                        \
            _mb = kwalloc(nb);                                          \
        }                                                               \
        if (_mb) {                                                      \
            _ext = mbexthdr(buf);                                       \
            if ((how) & MEM_ZERO) {                                     \
                kbzero(_mb, nb);                                        \
            } else {                                                    \
                _ext->rel = NULL;                                       \
                _ext->args = NULL;                                      \
            }                                                           \
        }                                                               \
    } while (0)
#define __memputbuf(buf) kfree(buf)
#define __memputblk(ptr) kfree(ptr)

/* allocate buffer and initialise for internal data */
static __inline__ struct kmembuf *
memgetbuf(long type, long how)
{
    struct kmembuf *buf;

    __memgetbuf(buf, MEMBUF_SIZE, how);
    if (buf) {
        buf->hdr.data = mbdata(buf);
        buf->hdr.type = type;
    }

    return buf;
}

/* allocate block and refer it to buffer */
static __inline__ struct kmembuf *
memgetblk(struct kmembuf *buf, long how)
{
    struct kmembuf *mb;
    struct kmemext *ext;
    uint8_t        *adr;

    __memgetblk(mb, MEMBUF_BLK_SIZE, how);
    if (buf) {
        adr = (uint8_t *)buf;
        ext = mbexthdr(buf);
        buf->hdr.data = adr;
        buf->hdr.flg |= MEMBUF_EXT_BIT;
        ext->buf = adr;
        ext->size = MEMBUF_BLK_SIZE;
        ext->type = MEMBUF_EXT_BLK;
    }

    return buf;
}

/* allocate buffer and initialize for packet header and internal data */
static __inline__ struct kmembuf *
memgetpkt(long type, long how)
{
    struct kmembuf *buf;

    __memgetbuf(buf, MEMBUF_SIZE, how);
    if (buf) {
        buf->hdr.data = mbpktbuf(buf);
        buf->hdr.type = type;
        buf->hdr.flg = MEMBUF_PKTHDR_BIT;
    }

    return buf;
}

/* set up preallocated external storage and refer it to buffer */
static __inline__ void
memsetext(struct kmembuf *buf, void *adr, long size,
          void (*rel)(void *, void *),
          void *args, long type, long flg)
{
    struct kmemext *ext;

    ext = mbexthdr(buf);
    buf->hdr.data = adr;
    buf->hdr.flg |= MEMBUF_EXT_BIT | flg;
    ext->buf = adr;
    ext->size = size;
    ext->rel = rel;
    ext->args = args;
    ext->type = type;

    return;
}

/* release external storage if reference count is zero */
static __inline__ void
memrelext(struct kmembuf *buf)
{
    struct kmemext *ext;

    ext = mbexthdr(buf);
    mbdecref(buf);
    if (m_cmpswap(&ext->nref, 0, 1)) {
        if (ext->type != MEMBUF_EXT_BLK) {
            ext->rel(ext->buf, ext->args);
        } else {
            __memputblk(ext->buf);
        }
    }
    buf->hdr.flg &= ~MEMBUF_EXT_BIT;
}

/*
 * release buffer and possible associated external storage
 * - return next buffer in chain
 */
static __inline__ struct kmembuf *
memrelbuf(struct kmembuf *buf)
{
    struct kmembuf *next;

    next = mbnext(buf);
    if (buf->hdr.flg & MEMBUF_EXT_BIT) {
        memrelext(buf);
    }
    __memputbuf(buf);

    return next;
}

/* free chain of buffers */
static __inline__ void
memrelchain(struct kmembuf *buf)
{
    struct kmembuf *mb;
    struct kmembuf *aux;
    struct kmempkt *pkt;
    
    if (buf) {
        do {
            if (mbflg(buf) & MEMBUF_PKTHDR_BIT) {
                pkt = mbpkthdr(buf);
                aux = pkt->aux;
                memrelbuf(aux);
                pkt->aux = NULL;
            }
            buf = memrelbuf(buf);
        } while (buf);
    }

    return;
}

/*
 * allocate len-bytes's worth of buffers
 * - return pointer to top of chain;
 *   if buf is non-NULL, allocate chain and return buf
 * - in case of failure we free everything we allocated and return NULL
 */
static __inline__ struct kmembuf *
memgetchain(struct kmembuf *buf, long len, long how, long type)
{
    struct kmembuf *last = NULL;
    struct kmembuf *mb;
    struct kmembuf *top;
    struct kmembuf *tail;

    mb = memgetbuf(type, how);
    if (!mb) {

        return NULL;
    } else if (len > MEMBUF_PKT_LEN) {
        __memgetblk(mb, MEMBUF_BLK_SIZE, how);
        if (!(mbflg(mb) & MEMBUF_EXT_BIT)) {
            memrelbuf(mb);

            return NULL;
        }
    }
    len -= mbtrailspace(mb);
    if (buf) {
        for (last = buf ; (mbnext(last)) ; last = mbnext(last)) {
            ;
        }
    } else {
        buf = mb;
    }
    top = mb;
    tail = mb;
    while (len > 0) {
        mb = memgetbuf(type, how);
        if (!mb) {
            memrelchain(top);

            return NULL;
        }
        tail->hdr.next = mb;
        if (len > MEMBUF_PKT_LEN) {
            memgetblk(mb, how);
            if (!(mbflg(mb) & MEMBUF_EXT_BIT)) {
                memrelchain(top);
                
                return NULL;
            }
        }
        tail = mb;
        len -= mbtrailspace(mb);
    }
    if (last != NULL) {
        last->hdr.next = top;
    }

    return buf;
}

/*
 * copy packet header from src to dest
 * - src->hdr.flg must have MEMBUF_PKTHDR_BIT set, and dest must be empty
 */
static __inline__ void
memcpypkthdr(struct kmembuf *src, struct kmembuf *dest)
{
    struct kmempkt *spkt;
    struct kmempkt *dpkt;

    spkt = mbpkthdr(src);
    dpkt = mbpkthdr(dest);
    dest->hdr.data = mbpktbuf(src);
    dest->hdr.flg = spkt->flg & MEMBUF_PKT_COPY_BITS;
    *dpkt = *spkt;
    spkt->aux = NULL;

    return;
}

/* allocate new buffer to prepend to chain, copy data along */
static __inline__ struct kmembuf *
_memprepend(struct kmembuf *buf, long nb, long how)
{
    struct kmembuf *mb;

    mb = memgetbuf(mbtype(buf), how);
    if (!mb) {
        memrelchain(buf);

        return NULL;
    }
    if (mbflg(buf) & MEMBUF_PKTHDR_BIT) {
        memcpypkthdr(buf, mb);
        buf->hdr.flg &= ~MEMBUF_PKTHDR_BIT;
    }
    mb->hdr.next = buf;
    if (nb < MEMBUF_PKT_LEN) {
        mbalignpkt(mb, nb);
    }
    mb->hdr.len = nb;

    return mb;
}

/* prepare buf for prepending len bytes of data */
static __inline__ struct kmembuf *
memprepend(struct kmembuf *buf, long nb, long how)
{
    struct kmempkt *pkt;
    
    if (mbleadspace(buf) >= nb) {
        buf->hdr.data -= nb;
        buf->hdr.len += nb;
    } else {
        buf = _memprepend(buf, nb, how);
    }
    if ((buf) && (mbflg(buf) & MEMBUF_PKTHDR_BIT)) {
        pkt = mbpkthdr(buf);
        pkt->len += nb;
    }

    return buf;
}

/*
 * - copy nb bytes from membuf chain starting from ofs0 bytes from the beginning
 * - if nb is MEMBUF_COPY_ALL, copy to end of buf
 * - how can be 0 or MEM_WAIT
 * - copy is read-only; blocks are not copied, just reference counts incremented
 */
static __inline__ struct kmembuf *
memcpychain(struct kmembuf *buf, int ofs0, long nb, long how)
{
    long             ofs = ofs0;
    long             cpyhdr = 0;
    long             len;
    struct kmembuf  *mb;
    struct kmembuf **hi;
    struct kmembuf  *top;
    struct kmempkt  *pkt;
    struct kmemext  *ext1;
    struct kmemext  *ext2;

    if (!ofs && (mbflg(buf) & MEMBUF_PKTHDR_BIT)) {
        cpyhdr = 1;
    }
    while (ofs > 0) {
        len = mblen(buf);
        if (ofs < len) {

            break;
        }
        ofs -= len;
        buf = mbnext(buf);
    }
    hi = &top;
    top = NULL;
    while (nb > 0) {
        if (!buf) {

            break;
        }
        mb = memgetbuf(mbtype(buf), how);
        len = mblen(buf);
        *hi = mb;
        if (!mb) {
            memrelchain(top);

            return NULL;
        }
        if (cpyhdr) {
            pkt = mbpkthdr(mb);
            memcpypkthdr(buf, mb);
            if (nb == MEMBUF_COPYALL) {
                pkt->len -= ofs0;
            } else {
                pkt->len = nb;
            }
            cpyhdr = 0;
        }
        mb->hdr.len = min(nb, len - ofs);
        if (mbflg(buf) & MEMBUF_EXT_BIT) {
            ext1 = mbexthdr(buf);
            ext2 = mbexthdr(mb);
            mb->hdr.data = buf->hdr.data + ofs;
            *ext2 = *ext1;
            mb->hdr.flg |= MEMBUF_EXT_BIT;
            mbincref(buf);
        } else {
            kbcopy(mtod(buf, uint8_t *) + ofs,
                   mtod(mb, uint8_t *),
                   (uintptr_t)mblen(mb));
        }
        if (nb != MEMBUF_COPYALL) {
            nb -= mblen(mb);
        }
        ofs = 0;
        buf = mbnext(buf);
        hi = &mbnext(mb);
    }

    return top;
}

/*
 * - copy whole packet including header (which must be present)
 * - optimisation of memcpychain(buf, 0, M_COPYALL, how)
 * - how can be 0 or MEM_WAIT
 * - copy is read-only; blocks are not copied, just reference counts incremented
 * - preserve alignment of first membuf, e.g. for protocol headers
 */
static __inline__ struct kmembuf *
memcpypkt(struct kmembuf *buf, long how)
{
    struct kmembuf *mb1;
    struct kmembuf *mb2;
    struct kmembuf *top;
    struct kmemext *ext1;
    struct kmemext *ext2;
    uint8_t        *buf1;
    uint8_t        *buf2;

    mb1 = memgetbuf(mbtype(buf), how);
    top = mb1;
    if (!mb1) {
        memrelchain(top);

        return NULL;
    }
    memcpypkthdr(buf, mb1);
    mb1->hdr.len = mblen(buf);
    if (mbflg(buf) & MEMBUF_EXT_BIT) {
        ext1 = mbexthdr(mb1);
        ext2 = mbexthdr(buf);
        mb1->hdr.data = mbadr(buf);
        *ext1 = *ext2;
        mb1->hdr.flg |= MEMBUF_EXT_BIT;
        mbincref(buf);
    } else {
        buf1 = mbpktbuf(mb1);
        buf2 = mbpktbuf(buf);
        mb1->hdr.data = buf1 + (buf->hdr.data - buf2);
        kbcopy(mtod(buf, uint8_t *),
               mtod(mb1, uint8_t *),
               (uintptr_t)mblen(mb1));
    }
    buf = mbnext(buf);
    while (buf) {
        mb2 = memgetbuf(mbtype(buf), how);
        if (!mb2) {
            memrelchain(top);
            
            return NULL;
        }
        mb1->hdr.next = mb2;
        mb2->hdr.len = mblen(buf);
        mb1 = mb2;
        if (mbflg(buf) & MEMBUF_EXT_BIT) {
            ext1 = mbexthdr(mb1);
            ext2 = mbexthdr(buf);
            mb1->hdr.data = mbadr(buf);;
            mb1->hdr.flg |= MEMBUF_EXT_BIT;
            *ext1 = *ext2;
            mbincref(buf);
        } else {
            kbcopy(mtod(buf, uint8_t *),
                   mtod(mb1, uint8_t *),
                   (uintptr_t)mblen(mb1));
        }
        buf = mbnext(buf);
    }

    return top;
}

/*
 * copy data from buffer-chain starting at ofs bytes from the beginning into
 * the indicated buffer
 */
static __inline__ void
memcpydata(struct kmembuf *buf, long ofs, long nb, uint8_t *dest)
{
    unsigned long cnt;
    long          len;

    len = mblen(buf);
    while (ofs > 0) {
        if (ofs < len) {

            break;
        }
        ofs -= len;
        buf = mbnext(buf);
        len = mblen(buf);
    }
    while (nb > 0) {
        cnt = min(len - ofs, nb);
        kbcopy(mtod(buf, uint8_t *), dest, cnt);
        len -= cnt;
        buf = mbnext(buf);
        dest += cnt;
        ofs = 0;
        len = mblen(buf);
    }

    return;
}

/*
 * - copy whole packet including header into a new buffer-chain
 * - blocks are copied along
 * - like memcpypkt(), but produces a writable copy
 */
static __inline__ struct kmembuf *
mduppkt(struct kmembuf *buf, long how)
{
    struct kmembuf  *mb;
    struct kmembuf **hi;
    struct kmembuf  *top;
    long            len;
    long            ofs;
    long            nb;
    long            sz;
    long            csz;
    long            nleft;

    if (!buf) {

        return NULL;
    }
    nleft = mbpktlen(buf);
    ofs = 0;
    hi = &top;
    while (nleft > 0 || !top) {
        mb = memgetbuf(mbtype(buf), how);
        if (!mb) {
            memrelchain(top);

            return NULL;
        }
        if (!top) {
            memcpypkthdr(buf, mb);
            len = MEMBUF_PKT_LEN;
        } else {
            len = MEMBUF_DATA_LEN;
        }
        if (nleft > MEMBUF_PKT_LEN) {
            memgetblk(mb, how);
            if (!(mbflg(mb) & MEMBUF_EXT_BIT)) {
                __memputbuf(mb);
                memrelchain(top);
                
                return NULL;
            }
            len = MEMBUF_BLK_SIZE;
        }
        *hi = mb;
        mb->hdr.len = 0;
        nb = 0;
        hi = &mbnext(mb);
        while (nb < len && (buf)) {
            sz = mblen(buf);
            csz = min(len - nb, sz - ofs);
            kbcopy(mbdata(buf), mbdata(mb) + nb, csz);
            ofs += csz;
            nb += csz;
            nleft -= csz;
            if (ofs == sz) {
                buf = mbnext(buf);
                ofs = 0;
            }
        }
        mb->hdr.len = nb;
    }

    return top;
}

static __inline__ void
memcatchain(struct kmembuf *src, struct kmembuf *dest)
{
    uint8_t *sptr;
    uint8_t *dptr;
    uint8_t *lim;
    long     join;
    long     slen;
    long     dlen;
    
    while (mbnext(dest)) {
        dest = mbnext(dest);
    }
    join = mbflg(dest);
    dlen = mblen(dest);
    while (src) {
        dptr = mbadr(dest);
        slen = mblen(src);
        lim = &dptr[MEMBUF_DATA_LEN];
        sptr = mbadr(src);
        if ((join)
            || dptr + dlen + slen >= lim) {
            dest->hdr.next = src;

            return;
        }
        kbcopy(sptr, dptr + dlen, (uintptr_t)slen);
        dlen += slen;
        src = memrelbuf(src);
    }
    dest->hdr.len = dlen;

    return;
}

