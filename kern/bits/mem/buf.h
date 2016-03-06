#ifndef __KERN_BITS_MEM_BUF_H__
#define __KERN_BITS_MEM_BUF_H__

#include <stdint.h>
#include <zero/asm.h>
#include <zero/trix.h>
#include <kern/util.h>

#define membufsettype(mb, t)                                            \
    ((mb)->hdr.type = (t))
#define membufwritable(mb)                                              \
    (!((mb)->flg & MEMBUF_RDONLY)                                       \
     && (!((mb)->flg & MEMBUF_EXTBIT)                                   \
         || !memextisref(mb)))

#define memextisref(mb)  ((membufextnref) && ((*membufextnref(mb)) > 1))
#define memaddextref(mb) m_atominc(membufextnref(mb))
#define memrmextref(mb)  m_atomdec(membufextnref(mb))

/* TODO: I hope we don't need another suballocator for membufs... :) */

/* get and initialise a standard membuf */
#define _memgetbuf(mp, how, t)                                          \
    do {                                                                \
        struct membuf *_mb = memgetbuf(how);                            \
                                                                        \
        while (!_mb && (how == MEM_TRYWAIT)) {                          \
            m_waitint();                                                \
            _mb = memgetbuf(how);                                       \
        }                                                               \
        if (_mb) {                                                      \
            _mb->hdr.type = (t);                                        \
            _mb->hdr.next = NULL;                                       \
            _mb->hdr.flg = 0;                                           \
        }                                                               \
        (mp) = _mb;                                                     \
    } while (0)

#define _memgetblk(mp, how, type)                                       \
    do {                                                                \
        struct membuf *_mb = (mp);                                      \
        struct memext *_ext = membufexthdr(_mb);                        \
        void          *_buf = memgetblk(how);                           \
                                                                        \
        if (_buf) {                                                     \
            _mb->hdr->adr = _buf;                                       \
            _ext->adr = _buf;                                           \
            _ext->nref = 0;                                             \
            _mb->hdr.flg |= MEMBUF_PKTHDR_BIT;                          \
            _ext->free = NULL;                                          \
            _ext->arg = NULL;                                           \
            _ext->size = MEMBUF_BLK_SIZE;                               \
            _ext->type = MEMBUF_EXT_BLK;                                \
        }                                                               \
    } while (0)

#define _memgetpkthdr(mp, how, t)                                       \
    do {                                                                \
        struct membuf *_mb = memgetbuf(how);                            \
                                                                        \
        if (_mb) {                                                      \
            struct mempkt *_pkt = &_mb->info.pkt;                       \
                                                                        \
            _mb->hdr.type = (t);                                        \
            _pkt->next = NULL;                                          \
            _pkt->flg = MEMBUF_PKTHDR_BIT;                              \
            _pkt->rcvif = NULL;                                         \
            _pkt->flg = 0;                                              \
            _pkt->aux = NULL;                                           \
        }                                                               \
        (mp) = _mb;                                                     \
    } while (0)

#define _memaddext(mb, how, size, free, arg, flg, type)                 \
    do {                                                                \
        struct membuf *_mb = (mb);                                      \
        void          *_buf = kmalloc(size);                            \
                                                                        \
        while (!_buf && (how == MEM_TRYWAIT)) {                         \
            m_waitint();                                                \
            _buf = kmalloc(size);                                       \
        }                                                               \
        if (_buf) {                                                     \
            struct memext *_ext = &_mb->info.ext;                       \
                                                                        \
            _mb_->hdr.adr = _buf;                                       \
            _ext->adr = _buf;
            _ext->nref = NULL;                                          \
            _ext->free = kfree;                                         \
            _ext->arg = (arg);                                          \
            _ext->size = (size);                                        \
            _ext->flg |= (MEMBUF_EXTBIT | (flg));                       \
        }                                                               \
    } while (0)

#define _memfreeext(mb)                                                 \
    do {                                                                \
        struct membuf *_mb = (mb);                                      \
        struct memext *_ext = &_mb->data.hdr.mem.ext;                   \
                                                                        \
        memrmextref(_mb);                                               \
        if (m_cmpswap(_ext->nref, 0, 1)) {                              \
            if (_ext->type != MEMBUF_EXT_BLK) {                         \
            (*(_ext->free))(_ext->adr, _ext->arg);                      \
            } else {                                                    \
                memputblk(_ext->adr);                                   \
            }                                                           \
            ext->nref = NULL;                                           \
        }                                                               \
        _mb->hdr.flg &= ~MEMBUF_EXTBIT;                                 \
    } while (0)

#define _memfreebuf(mp)                                                 \
    do {                                                                \
        struct membuf *_mb = *(mp);                                     \
                                                                        \
        if (_mb->hdr.flg & MEMBUF_EXTBIT) {                             \
            memfreeext(_mb);                                            \
        }                                                               \
        memputbuf(_mb);                                                 \
        (mp) = NULL;                                                    \
    } while (0)

#define _memcpypkthdr(dest, src)                                        \
    do {                                                                \
        struct membuf *_dest = (dest);                                  \
        struct membuf *_src = (src);                                    \
                                                                        \
        _dest->hdr.adr = membufpktdata(_dest);                          \
        _dest->hdr.flg = _src->hdr.flg & MEMBUF_PKT_COPYBITS;           \
        *(membufpkthdr(_dest)) = *(membufpkthdr(_src));              \
        _src->data.hdr.pkt.aux = NULL;                                  \
    } while (0)

#define _memmovepkthdr(dest, src)                                       \
    do {                                                                \
        struct membuf *_dest = (dest);                                  \
        struct membuf *_src = (src);                                    \
                                                                        \
        _dest->info.pkt.flg = _src->info.pkt.flg & MEMBUF_PKT_COPY_BITS; \
        if (!(_dest->hdr.flg & MEMBUF_EXT_BIT)) {                       \
            kmemcpy(_dest->info.pkt.data, _src->info.pkt.data,          \
                    _src->info.pkt.len);                                \
        }                                                               \
        *(membufpkthdr(_dest)) = *(membufpkthdr(_src));                 \
    } while (0)

#define _membufsize(mb)                                                 \
    (((mb)->hdr.flg & MEMBUF_EXT_BIT)                                   \
     ? (membufextsize(mb))                                              \
     : (((mb)->hdr.flg & MEMBUF_PKTHDR_BIT)                             \
        ? MEMBUF_PKTLEN                                                 \
        : MEMBUF_LEN))

#define _memalignbuf(mb, len)                                           \
    rounddownpow2((uintptr_t)(mb)->hdr.adr + MEMBUF_LEN - (len),        \
                  sizeof(long))

#define _memalignpkt(mb, len)                                           \
    rounddownpow2((uintptr_t)(mb)->hdr.adr + MEMBUF_PTKLEN - (len),     \
                  sizeof(long))

#define _membufleadspace(mb)                                            \
    (((mb)->hdr.flg & MEMBUF_EXTBIT)                                    \
     ? 0                                                                \
     : (((mb)->hdr.flg & MEMBUF_PKTHDR_BIT)                             \
        ? (membufpktdata(mb) - (mb))                                    \
        : ((mb)->hdr.data - (mb))))

#define _membuftrailspace(mb)                                           \
    (((mb)->hdr.flg & MEMBUF_EXTBIT)                                    \
     ? (membufextadr(mb) + membufextsize(mb)                            \
        - ((mb)->hdr.adr + (mb)->hdr.len))                              \
     : (&(mb)->data.buf[MEMBUF_LEN] - ((mb)->hdr.adr + (mb)->hdr.len)))

#define _membufprepend(mb, len, how)                                    \
    do {                                                                \
        struct membuf **_mbp = &(mb);                                   \
        struct membuf  *_mb = *_mbp;                                    \
        size_t          _len = (len);                                   \
        long            _how = (how);                                   \
                                                                        \
        if (membufleadspace(mb) >= _len) {                              \
            _mb->hdr.adr -= len;                                        \
            _mb->hdr.len += len;                                        \
        } else {                                                        \
            _mb = membufprepend(_mb, _len, _how);                       \
        }                                                               \
        if ((_mb) && ((mb)->hdr.flg & MEMBUF_PKTHDR_BIT)) {             \
            struct mempkt *_pkt = membufpkthdr(_mb);                    \
                                                                        \
            _pkt->len += _len;                                          \
        }                                                               \
        *_mbp = _mb;                                                    \
    } while (0)

#endif /* __KERN_BITS_MEM_BUF_H__ */

