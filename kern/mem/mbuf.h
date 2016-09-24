#ifndef __KERN_MEM_MBUF_H__
#define __KERN_MEM_MBUF_H__

#include <kern/conf.h>
#include <stdint.h>
#include <stddef.h>
#if (SMP)
#include <zero/asm.h>
#endif
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/types.h>

/* amount of memory to use for membufs */
#define MEMNBUF              (4 * MEMNBUFBLK)

/* membuf convenience macros */
#define MBUF_SIZE          PAGESIZE                   // membuf size
#define MBUF_BLK_SHIFT     (PAGESIZELOG2 + 1)         // blk of 2 * PAGESIZE
#define MBUF_BLK_SIZE      (1 << MBUF_BLK_SHIFT)
#define MBUF_BLK_MINSIZE   (MBUF_PKT_LEN + 1)
#define MBUF_DATA_LEN      ((long)(MBUF_SIZE - sizeof(struct memhdr)))
#define MBUF_PKT_LEN       ((long)(MBUF_DATA_LEN - sizeof(struct mempkt)))
#define MBUF_BLK_MIN_SIZE  (MBUF_PKT_LEN + 1)
#define MBUF_MAX_COMPRESS  (MBUF_PKT_LEN >> 1)

/* copy parameter */
#define MBUF_COPYALL       (~0L)

/* membuf types */
#define MBUF_FREE          0  // on free-list
#define MBUF_DATA          1  // dynamic allocation
#define MBUF_EXT           2  // external storage mapped to mbuf
#define MBUF_PKTHDR        3  // packet header
#define MBUF_SOCK          4  // socket structure
#define MBUF_PCB           5  // protocol control block
#define MBUF_ROUTE_TAB     6  // routing tables
#define MBUF_HOST_TAB      7  // IMP host tables
#define MBUF_ADR_TAB       8  // address resolution tables
#define MBUF_SONAME        9  // socket name
#define MBUF_SOOPTS        10 // socket options
#define MBUF_FRAG_TAB      11 // fragment reassembly hedaer
#define MBUF_RIGHTS        12 // access rights
#define MBUF_IFADDR        13 // interface address
#define MBUF_CONTROL       14 // extra-data protocol message
#define MBUF_OOBDATA       15 // expedited data
#define MBUF_NTYPE         16
#define MBUF_NOINIT        (~0L) // for allocating non-initialised mbufs */
/* membuf flg-bits */
/* membuf flags */
#define MBUF_EXT_BIT       (1 << 0) // associated external storage
#define MBUF_PKTHDR_BIT    (1 << 1) // start of record
#define MBUF_EOR_BIT       (1 << 2) // end of record
#define MBUF_RDONLY_BIT    (1 << 3) // read-only data
#define MBUF_BROADCAST_BIT (1 << 4) // send/received as link-level broadcast
#define MBUF_MULTICAST_BIT (1 << 5) // send/received as link-level multicast
#define MBUF_PROMISC_BIT   (1 << 6) // packet not for us
#define MBUF_VLANTAG_BIT   (1 << 7) // ethernet vtag is valid
#define MBUF_UNUSED_BIT    (1 << 8) // AVAILABLE */
#define MBUF_NOFREE_BIT    (1 << 9) // mbuf embedded in blk, no separate free
#define MBUF_PROTO_BIT1    (1 << 10)// protocol-specific flags
#define MBUF_PROTO_BIT2    (1 << 11)
#define MBUF_PROTO_BIT3    (1 << 12)
#define MBUF_PROTO_BIT4    (1 << 13)
#define MBUF_PROTO_BIT5    (1 << 14)
#define MBUF_PROTO_BIT6    (1 << 15)
#define MBUF_PROTO_BIT7    (1 << 16)
#define MBUF_PROTO_BIT8    (1 << 17)
#define MBUF_PROTO_BIT9    (1 << 18)
#define MBUF_PROTO_BIT10   (1 << 19)
#define MBUF_PROTO_BIT11   (1 << 20)
#define MBUF_PROTO_BIT12   (1 << 21)
/* flags purged when crossing layers */
#define MBUF_PROTO_BITS                                                 \
    (MBUF_PROTO_BIT1 | MBUF_PROTO_BIT2 | MBUF_PROTO_BIT3                \
     | MBUF_PROTO_BIT4 | MBUF_PROTO_BIT5 | MBUF_PROTO_BIT6              \
     | MBUF_PROTO_BIT7 | MBUF_PROTO_BIT8 | MBUF_PROTO_BIT9              \
     | MBUF_PROTO_BIT10 | MBUF_PROTO_BIT11 | MBUF_PROTO_BIT12)
/* flags copied for pkthdr */
#define MBUF_PKT_COPY_BITS                                              \
    (MBUF_PKTHDR_BIT | MBUF_EOR_BIT | MBUF_RDONLY_BIT                   \
     | MBUF_BROADCAST_BIT | MBUF_MULTICAST_BIT | MBUF_PROMISC_BIT       \
     | MBUF_VLANTAG_BIT                                                 \
     | MBUF_PROTO_BITS)
#define MBUF_EXT_COPY_BITS (MBUF_EXT_BITS)

/* checksum flg-bits */
#define MBUF_CHKSUM_IP         (1 << 0) // checksum IP
#define MBUF_CHKSUM_UDP        (1 << 1) // checksum UDP
#define MBUF_CHKSUM_TCP        (1 << 2) // checksum TCP
#define MBUF_CHKSUM_STCP       (1 << 3) // checksum UDP
#define MBUF_CHKSUM_IP_TSO     (1 << 4) // TCP segmentation offload
#define MBUF_CHKSUM_ISCSI      (1 << 5) // iSCSI checksum offload
#define MBUF_CHKSUM_IP6_UDP    (1 << 9)  // more checksum offloads
#define MBUF_CHKSUM_IP6_TCP    (1 << 10)
#define MBUF_CHKSUM_IP6_SCTP   (1 << 11)
#define MBUF_CHKSUM_IP6_TSO    (1 << 12)
#define MBUF_CHKSUM_IP6_ISCSI  (1 << 13)
/* hardware-verified inbound checksum support */
#define MBUF_CHKSUM_L3_CALC    (1 << 24)
#define MBUF_CHKSUM_L3_VALID   (1 << 25)
#define MBUF_CHKSUM_L4_CALC    (1 << 26)
#define MBUF_CHKSUM_L4_VALID   (1 << 27)
#define MBUF_CHKSUM_L5_CALC    (1 << 28)
#define MBUF_CHKSUM_L5_VALID   (1 << 29)
#define MBUF_CHKSUM_MERGED     (1 << 30)
/* record/packet header in first mb of chain; MBUF_PKTHDR is set */
struct mempkt {
    struct netif  *rcvif;       // receiver interface
    long           len;         // total packet length
    uint8_t       *hdr;         // packet header
    int32_t        flg;         // checksum and other flags
    int32_t        chksum;      // checksum data
    struct membuf *aux;         // extra data buffer, e.g. IPSEC
};

/* external buffer types */
#define MBUF_EXT_NETDRV     0x01000000 // custom extbuf provide by net driver
#define MBUF_EXT_MODULE     0x02000000 // custom module-specific extbuf type
#define MBUF_EXT_DISPOSABLE 0x04000000 // may throw away with page-flipping
#define MBUF_EXT_EXTREF     0x08000000 // external reference count
#define MBUF_EXT_RDONLY     0x10000000 //
#define MBUF_EXT_RDWR       0x20000000
#define MBUF_EXT_BITS       0xff000000
#define MBUF_EXT_BLK        1          // mb block ('cluster')
#define MBUF_EXT_FILEBUF    2          // buffer for sendfile()-like things
#define MBUF_EXT_PAGE       3          // PAGESIZE-byte buffer
#define MBUF_EXT_DUAL_PAGE  4          // 2 * PAGESIZE bytes
#define MBUF_EXT_QUAD_PAGE  5          // 4 * PAGESIZE bytes
#define MBUF_EXT_PACKET     6          // mb + memblk from packet zone
#define MBUF_EXT_MBUFUF       7          // external mbuf (M_IOVEC)
struct memext {
    uint8_t     *buf;   // buffer base address
    m_atomic_t   nref;  // external reference count
    long         size;  // buffer size
    void       (*rel)(void *, void *); // optional custom free()
    void        *args;  // optional argument pointer
    long         type;  // storage type
};

struct memhdr {
    uint8_t       *data;         // data address
    long           len;         // # of bytes in membuf
    long           type;        // buffer type
    long           flg;         // flags
    struct membuf *next;        // next buffer in chain
    struct membuf *nextpkt;     // next packet in chain
};

struct membuf {
    struct memhdr             hdr;
    union {
        struct {
            struct mempkt     pkt;      // MBUF_PKTHDR is set
            union {
                struct memext ext;
                uint8_t       buf[MBUF_PKT_LEN];
            } data;
        } s;
        uint8_t               buf[MBUF_DATA_LEN];
    } u;
};

#if (SMP)
#define mbincref(mb)      m_atominc(&(mbexthdr(mb))->nref)
#define mbdecref(mb)      m_atomadd(&(mbexthdr(mb))->nref, -1)
#define mbgetref(mb, res) m_syncread(&(mbexthdr(mb))->nref, (res))
#else
#define mbincref(mb)      ((mbexthdr(mb))->nref++)
#define mbdecref(mb)      ((mbexthdr(mb))->nref--)
#define mbgetref(mb, res) ((mbexthdr(mb))->nref)
#endif

#define mbadr(mb)         ((mb)->hdr.data)
#define mblen(mb)         ((mb)->hdr.len)
#define mbtype(mb)        ((mb)->hdr.type)
#define mbflg(mb)         ((mb)->hdr.flg)
#define mbnext(mb)        ((mb)->hdr.next)
#define mbdata(mb)        ((mb)->u.buf)
#define mbpkthdr(mb)      (&((mb)->u.s.pkt))
#define mbpktbuf(mb)      ((mb)->u.s.data.buf)
#define mbpktlen(mb)      ((mb)->u.s.pkt.len)
#define mbexthdr(mb)      (&((mb)->u.s.data.ext))
#define mbextbuf(mb)      (((mb)->u.s.data.ext.buf))
#define mbextsize(mb)     (((mb)->u.s.data.ext.size))

struct mempktaux {
    long af;
    long type;
};

struct membufbkt {
    m_atomic_t     lk;
    m_atomic_t     nref;
    long           flg;
    long           nbuf;
    long           nblk;
    struct membuf *buflist;
    uint8_t        _pad[CLSIZE - 5 * sizeof(long) - 1 * sizeof(void *)];
};

#include <kern/mem/bits/mbuf.h>

#endif /* __KERN_MEM_MBUF_H__ */

