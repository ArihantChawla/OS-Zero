#ifndef __KERN_MEM_MB_H__
#define __KERN_MEM_MB_H__

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
#define MB_SIZE          PAGESIZE                   // membuf size
#define MB_BLK_SHIFT     (PAGESIZELOG2 + 1)         // blk of 2 * PAGESIZE
#define MB_BLK_SIZE      (1 << MB_BLK_SHIFT)
#define MB_BLK_MINSIZE   (MB_PKT_LEN + 1)
#define MB_DATA_LEN      ((long)(MB_SIZE - sizeof(struct memhdr)))
#define MB_PKT_LEN       ((long)(MB_DATA_LEN - sizeof(struct mempkt)))
#define MB_BLK_MIN_SIZE  (MB_PKT_LEN + 1)
#define MB_MAX_COMPRESS  (MB_PKT_LEN >> 1)

/* copy parameter */
#define MB_COPYALL       (~0L)

/* membuf types */
#define MB_FREE          0  // on free-list
#define MB_DATA          1  // dynamic allocation
#define MB_EXT           2  // external storage mapped to mbuf
#define MB_PKTHDR        3  // packet header
#define MB_SOCK          4  // socket structure
#define MB_PCB           5  // protocol control block
#define MB_ROUTE_TAB     6  // routing tables
#define MB_HOST_TAB      7  // IMP host tables
#define MB_ADR_TAB       8  // address resolution tables
#define MB_SONAME        9  // socket name
#define MB_SOOPTS        10 // socket options
#define MB_FRAG_TAB      11 // fragment reassembly hedaer
#define MB_RIGHTS        12 // access rights
#define MB_IFADDR        13 // interface address
#define MB_CONTROL       14 // extra-data protocol message
#define MB_OOBDATA       15 // expedited data
#define MB_NTYPE         16
#define MB_NOINIT        (~0L) // for allocating non-initialised mbufs */
/* membuf flg-bits */
/* membuf flags */
#define MB_EXT_BIT       (1 << 0) // associated external storage
#define MB_PKTHDR_BIT    (1 << 1) // start of record
#define MB_EOR_BIT       (1 << 2) // end of record
#define MB_RDONLY_BIT    (1 << 3) // read-only data
#define MB_BROADCAST_BIT (1 << 4) // send/received as link-level broadcast
#define MB_MULTICAST_BIT (1 << 5) // send/received as link-level multicast
#define MB_PROMISC_BIT   (1 << 6) // packet not for us
#define MB_VLANTAG_BIT   (1 << 7) // ethernet vtag is valid
#define MB_UNUSED_BIT    (1 << 8) // AVAILABLE */
#define MB_NOFREE_BIT    (1 << 9) // mbuf embedded in blk, no separate free
#define MB_PROTO_BIT1    (1 << 10)// protocol-specific flags
#define MB_PROTO_BIT2    (1 << 11)
#define MB_PROTO_BIT3    (1 << 12)
#define MB_PROTO_BIT4    (1 << 13)
#define MB_PROTO_BIT5    (1 << 14)
#define MB_PROTO_BIT6    (1 << 15)
#define MB_PROTO_BIT7    (1 << 16)
#define MB_PROTO_BIT8    (1 << 17)
#define MB_PROTO_BIT9    (1 << 18)
#define MB_PROTO_BIT10   (1 << 19)
#define MB_PROTO_BIT11   (1 << 20)
#define MB_PROTO_BIT12   (1 << 21)
/* flags purged when crossing layers */
#define MB_PROTO_BITS                                                   \
    (MB_PROTO_BIT1 | MB_PROTO_BIT2 | MB_PROTO_BIT3                      \
     | MB_PROTO_BIT4 | MB_PROTO_BIT5 | MB_PROTO_BIT6                    \
     | MB_PROTO_BIT7 | MB_PROTO_BIT8 | MB_PROTO_BIT9                    \
     | MB_PROTO_BIT10 | MB_PROTO_BIT11 | MB_PROTO_BIT12)
/* flags copied for pkthdr */
#define MB_PKT_COPY_BITS                                                \
    (MB_PKTHDR_BIT | MB_EOR_BIT | MB_RDONLY_BIT                         \
     | MB_BROADCAST_BIT | MB_MULTICAST_BIT | MB_PROMISC_BIT             \
     | MB_VLANTAG_BIT                                                   \
     | MB_PROTO_BITS)
#define MB_EXT_COPY_BITS (MB_EXT_BITS)

/* checksum flg-bits */
#define MB_CHKSUM_IP         (1 << 0) // checksum IP
#define MB_CHKSUM_UDP        (1 << 1) // checksum UDP
#define MB_CHKSUM_TCP        (1 << 2) // checksum TCP
#define MB_CHKSUM_STCP       (1 << 3) // checksum UDP
#define MB_CHKSUM_IP_TSO     (1 << 4) // TCP segmentation offload
#define MB_CHKSUM_ISCSI      (1 << 5) // iSCSI checksum offload
#define MB_CHKSUM_IP6_UDP    (1 << 9)  // more checksum offloads
#define MB_CHKSUM_IP6_TCP    (1 << 10)
#define MB_CHKSUM_IP6_SCTP   (1 << 11)
#define MB_CHKSUM_IP6_TSO    (1 << 12)
#define MB_CHKSUM_IP6_ISCSI  (1 << 13)
/* hardware-verified inbound checksum support */
#define MB_CHKSUM_L3_CALC    (1 << 24)
#define MB_CHKSUM_L3_VALID   (1 << 25)
#define MB_CHKSUM_L4_CALC    (1 << 26)
#define MB_CHKSUM_L4_VALID   (1 << 27)
#define MB_CHKSUM_L5_CALC    (1 << 28)
#define MB_CHKSUM_L5_VALID   (1 << 29)
#define MB_CHKSUM_MERGED     (1 << 30)
/* record/packet header in first mb of chain; MB_PKTHDR is set */
struct mempkt {
    struct netif  *rcvif;       // receiver interface
    long           len;         // total packet length
    uint8_t       *hdr;         // packet header
    int32_t        flg;         // checksum and other flags
    int32_t        chksum;      // checksum data
    struct membuf *aux;         // extra data buffer, e.g. IPSEC
};

/* external buffer types */
#define MB_EXT_NETDRV     0x01000000 // custom extbuf provide by net driver
#define MB_EXT_MODULE     0x02000000 // custom module-specific extbuf type
#define MB_EXT_DISPOSABLE 0x04000000 // may throw away with page-flipping
#define MB_EXT_EXTREF     0x08000000 // external reference count
#define MB_EXT_RDONLY     0x10000000 //
#define MB_EXT_RDWR       0x20000000
#define MB_EXT_BITS       0xff000000
#define MB_EXT_BLK        1          // mb block ('cluster')
#define MB_EXT_FILEBUF    2          // buffer for sendfile()-like things
#define MB_EXT_PAGE       3          // PAGESIZE-byte buffer
#define MB_EXT_DUAL_PAGE  4          // 2 * PAGESIZE bytes
#define MB_EXT_QUAD_PAGE  5          // 4 * PAGESIZE bytes
#define MB_EXT_PACKET     6          // mb + memblk from packet zone
#define MB_EXT_MBUF       7          // external mbuf (M_IOVEC)
struct memext {
    uint8_t        *buf;        // buffer base address
    volatile long   nref;       // external reference count
    long            size;       // buffer size
    void          (*rel)(void *, void *); // optional custom free()
    void           *args;       // optional argument pointer
    long            type;       // storage type
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
            struct mempkt     pkt;      // MB_PKTHDR is set
            union {
                struct memext ext;
                uint8_t       buf[MB_PKT_LEN];
            } data;
        } s;
        uint8_t               buf[MB_DATA_LEN];
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
    volatile long  lk;
    volatile long  nref;
    long           flg;
    long           nbuf;
    long           nblk;
    struct membuf *buflist;
    uint8_t        _pad[CLSIZE - 5 * sizeof(long) - 1 * sizeof(void *)];
};

#include <kern/mem/bits/mb.h>

#endif /* __KERN_MEM_MB_H__ */

