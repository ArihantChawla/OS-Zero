#ifndef __KERN_BITS_MEM_BUF_H__
#define __KERN_BITS_MEM_BUF_H__

#include <stdint.h>
#include <zero/asm.h>
#include <zero/trix.h>

#define _membufchtype(mb, t)                                            \
    ((mb)->hdr.type = (t))
#define _membufwritable(mb)                                             \
    (!((mb)->flg & MEMBUF_RDONLY)                                       \
     && (!((mb)->flg & MEMBUF_EXTBIT)                                   \
         || !memextisref(mb)))

#define _memextisref(mb)  ((membufextnref) && ((*membufextnref(mb)) > 1))
#define _memaddextref(mb) m_atominc(membufextnref(mb))
#define _memrmextref(mb)  m_atomdec(membufextnref(mb))

/* TODO: I hope we don't need another suballocator for membufs... :) */

/* get and initialise a standard membuf */
#define _memgetbuf(mp, how, type)                                       \
    do {                                                                \
        struct membuf *_mb = memgetbuf();                               \
                                                                        \
        while (!_mb && (how == MEM_TRYWAIT)) {                          \
            m_waitint();                                                \
            _mb = memgetbuf(how);                                       \
        }                                                               \
        if (_mb) {                                                      \
            _mb->hdr.type = (type);                                     \
            _mb->hdr.next = NULL;                                       \
            _mb->hdr.nextpkt = NULL;                                    \
            _mb->hdr.adr = _mb->data.buf;                               \
            _mb->hdr.flg = 0;                                           \
        }                                                               \
        (mp) = _mb;                                                     \
    } while (0)

#define _memgetblk(mp, how)                                             \
    do {                                                                \
        struct membuf *_mb = (mp);                                      \
        struct memext *_ext = membufexthdr(_mb);                        \
        void          *_ptr = memgetblk(how);                           \
                                                                        \
        if (_ptr) {                                                     \
            _ext->adr = _ptr;                                           \
            ext->nref = 0;                                              \
            _mb->hdr.adr = _ptr;                                        \
            _mb->hdr.flg |= MEMBUF_PKTHDRBIT;                           \
            _ext->free = NULL;                                          \
            _ext->arg = NULL;                                           \
            _ext->size = MEMBUF_BLK_SIZE;                               \
            _ext->type = MEMBUF_EXT_BLK;                                \
        }                                                               \
    } while (0)

#define _memgetpkthdr(mp, how, type)                                    \
    do {                                                                \
        struct membuf *_mb = kmalloc(MEMBUF_SIZE);                      \
                                                                        \
        while (!_mb && (how == MEM_TRYWAIT)) {                          \
            m_waitint();                                                \
            _mb = kmalloc(MEMBUF_SIZE);                                 \
        }                                                               \
        if (_mb) {                                                      \
            struct pkthdr *_pkt = &_mb->data.hdr.pkt;                   \
            _mb->hdr.type = (type);                                     \
            _mb->hdr.next = NULL;                                       \
            _mb->hdr.nextpkt = NULL;                                    \
            _mb->hdr.adr = membufpktdata(_mb);                          \
            _mb->hdr.flg = MEMBUF_PKTHDRBIT;                            \
            pkt->rcvif = NULL;                                          \
            pkt->flg = 0;                                               \
            pkt->aux = NULL;                                            \
        }                                                               \
        (mp) = _mb;                                                     \
    } while (0)

#define _memaddext(mb, how, size, free, arg, flg, type)                 \
    do {                                                                \
        struct membuf *_mb = (mb);                                      \
        void          *_buf = kmalloc(size);                            \
                                                                        \
        while (!buf && (how == MEM_TRYWAIT)) {                          \
            m_waitint();                                                \
            _buf = kmalloc(size);                                       \
        }                                                               \
        if (_buf) {                                                     \
            struct memext *_ext = &_mb->data.hdr.mem.ext;               \
                                                                        \
            _ext->nref = NULL;                                          \
            _ext->adr = buf;                                            \
            _mb->hdr.adr = _buf;                                        \
            _mb->hdr.flg |= (MEMBUF_EXTBIT | (flg));                    \
            _ext->free = (free);                                        \
            _ext->arg = (arg);                                          \
            _ext->size = (size);                                        \
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
                (*(_ext->free))(_ext->adr, _ext->arg);                  \
            } else {                                                    \
                memputblk(_ext->adr);                                   \
            }                                                           \
            ext->nref = NULL;                                           \
        }                                                               \
        _mb->hdr.flg &= ~MEMBUF_EXTBIT;                                 \
    } while (0)

#define _memfreebuf(mp, next)                                           \
    do {                                                                \
        struct membuf *_mb = *(mp);                                     \
                                                                        \
        (next) = _mb->next;                                             \
        if (_mb->hdr.flg & MEMBUF_EXTBIT) {                             \
            _memfreeext(_mb);                                           \
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
        _dest->hdr.flg = _src->hdr.flg & MEMBUF_COPYBITS;               \
        *(membufpkthdr(_dest)) = *(membufpkthdr(_src));                 \
        _src->data.hdr.pkt.aux = NULL;                                  \
    } while (0)

#define _memmovepkthdr(dest, src)                                       \
    do {                                                                \
        struct membuf *_dest = (dest);                                  \
        struct membuf *_src = (src);                                    \
                                                                        \
        _dest->hdr.flg = ((_src->hdr.flg & MEMBUF_COPYBITS)             \
                          | (_dest->hdr.flg & MEMBUF_EXTBIT));          \
        if (!(_dest->hdr.flg & MEMBUF_EXTBIT)) {                        \
            _dest->hdr.adr = membufpktdata(_dest);                      \
        }                                                               \
        *(membufpkthdr(_dest)) = *(membufpkthdr(_src));                 \
    } while (0)

#define __membufsize(mb)                                                \
    (((mb)->hdr.flg & MEMBUF_EXTBIT)                                    \
     ? (membufextsize(mb))                                              \
     : (((mb)->hdr.flg & MEMBUF_PKTHDRBIT)                              \
        ? MEMBUF_PKTLEN                                                 \
        : MEMBUF_LEN))

#define _memalignbuf(mb, len)                                           \
    rounddownpow2((uintptr_t)(mb)->hdr.adr + MEMBUF_LEN - (len),        \
                  sizeof(long))

#define _memalignpkt(mb, len)                                           \
    rounddownpow2((uintptr_t)(mb)->hdr.adr + MEMBUF_BKTLEN - (len),     \
                  sizeof(long))

#define _membufleadspace(mb)                                            \
    (((mb)->hdr.flg & MEMBUF_EXTBIT)                                    \
     ? 0                                                                \
     : (((mb)->hdr.flg & MEMBUF_PKTHDRBIT)                              \
        ? ((mb)->hdr.adr - membufpktdata(mb))                           \
        : ((mb)->hdr.adr - (mb)->data.buf)))

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
        if ((_mb) && ((mb)->hdr.flg & MEMBUF_PKTHDRBIT)) {              \
            struct pkthdr *_pkt = membufpkthdr(_mb);                    \
                                                                        \
            _pkt->len += _len;                                          \
        }                                                               \
        *_mbp = _mb;                                                    \
    } while (0)

#endif /* __KERN_BITS_MEM_BUF_H__ */

