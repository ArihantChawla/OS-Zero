#ifndef __KERN_MEM_BUF_H__
#define __KERN_MEM_BUF_H__

#include <kern/conf.h>

#define MEMNBUF (4 * NMEMBUFBLK)

#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <kern/types.h>
#include <kern/bits/mem.h>

/* allocation flags */
#define MEM_TRYWAIT       0
#define MEM_DONTWAIT      1

/* membuf convenience macros */
#define MEMBUF_SIZE        256                     // membuf size
#define MEMBUF_BLK_SHIFT   (PAGESIZELOG2 - 1)      // blk half of PAGESIZE
#define MEMBUF_BLK_SIZE    (1 << MEMBUF_BLK_SHIFT) // blk < PAGESIZE
#define MEMBUF_BLK_MINSIZE (MEMBUF_PKTLEN + 1)
#define MEMBUF_HDRSIZE     sizeof(struct memhdr)
#define MEMBUF_PKTHDRSIZE  sizeof(struct pkthdr)
#define MEMBUF_LEN         (MEMBUF_SIZE - MEMBUF_HDRSIZE)
#define MEMBUF_PKTLEN      (MEMBUF_LEN - sizeof(struct pkthdr))

//struct membuf;

struct mempktaux {
    long af;
    long type;
};

/* checksum flg-bits */
#define MEMBUF_CHKSUM_IP         (1 << 0) // checksum IP
#define MEMBUF_CHKSUM_UDP        (1 << 1) // checksum UDP
#define MEMBUF_CHKSUM_TCP        (1 << 2) // checksum TCP
#define MEMBUF_CHKSUM_STCP       (1 << 3) // checksum UDP
#define MEMBUF_CHKSUM_IP_TSO     (1 << 4) // TCP segmentation offload
#define MEMBUF_CHKSUM_ISCSI      (1 << 5) // iSCSI checksum offload
#define MEMBUF_CHKSUM_IP6_UDP    (1 << 9)  // more checksum offloads
#define MEMBUF_CHKSUM_IP6_TCP    (1 << 10)
#define MEMBUF_CHKSUM_IP6_SCTP   (1 << 11)
#define MEMBUF_CHKSUM_IP6_TSO    (1 << 12)
#define MEMBUF_CHKSUM_IP6_ISCSI  (1 << 13)
/* hardware-verified inbound checksum support */
#define MEMBUF_CHKSUM_L3_CALC    (1 << 24)
#define MEMBUF_CHKSUM_L3_VALID   (1 << 25)
#define MEMBUF_CHKSUM_L4_CALC    (1 << 26)
#define MEMBUF_CHKSUM_L4_VALID   (1 << 27)
#define MEMBUF_CHKSUM_L5_CALC    (1 << 28)
#define MEMBUF_CHKSUM_L5_VALID   (1 << 29)
#define MEMBUF_CHKSUM_MERGED     (1 << 30)
/* record/packet header in first membuf of chain; MEMBUF_PKTHDR is set */
#define __STRUCT_PKTHDR_SIZE                                            \
    (3 * sizeof(void *) + 2 * sizeof(int32_t) + sizeof(size_t))
#define __STRUCT_PKTHDR_PAD                                             \
    (rounduppow2(__STRUCT_PKTHDR_SIZE, CLSIZE) - __STRUCT_PKTHDR_SIZE)
struct pkthdr {
    struct ifnet  *rcvif;       // rcv interface
    size_t         len;         // total packet length
    void          *hdr;         // packet header
    int32_t        flg;         // checksum and other flags
    int32_t        chksum;      // checksum data
    struct membuf *aux;         // extra data buffer, e.g. IPSEC
//    uint8_t        _pad[__STRUCT_PKTHDR_PAD];
};

/* external buffer types */
#define MEMBUF_EXT_NETDRV     0x01000000 // custom extbuf provide by net driver
#define MEMBUF_EXT_MODULE     0x02000000 // custom module-specific extbuf type
#define MEMBUF_EXT_DISPOSABLE 0x04000000 // may throw away with page-flipping
#define MEMBUF_EXT_EXTREF     0x08000000 // external reference count
#define MEMBUF_EXT_RDONLY     0x10000000 //
#define MEMBUF_EXT_RDWR       0x20000000
#define MEMBUF_EXT_BITS       0xff000000
#define MEMBUF_EXT_BLK        1          // membuf block ('cluster')
#define MEMBUF_EXT_FILEBUF    2          // buffer for sendfile()-like things
#define MEMBUF_EXT_PAGE       3          // PAGESIZE-byte buffer
#define MEMBUF_EXT_DUAL_PAGE  4          // 2 * PAGESIZE bytes
#define MEMBUF_EXT_QUAD_PAGE  5          // 4 * PAGESIZE bytes
#define MEMBUF_EXT_PACKET     6          // membuf + memblk from packet zone
#define MEMBUF_EXT_MBUF       7          // external mbuf (M_IOVEC)
#define __STRUCT_MEMEXT_SIZE                                            \
    (2 * sizeof(long) + 5 * sizeof(void *) + sizeof(size_t))
#define __STRUCT_MEMEXT_PAD                                             \
    (rounduppow2(__STRUCT_MEMEXT_SIZE, CLSIZE) - __STRUCT_MEMEXT_SIZE)
struct memext {
    union {
        volatile long  val;     // local reference count
        volatile long *ptr;     // external reference count
    } nref;
    uint8_t        *adr;        // buffer base address
    void          (*free)(void *, void *, void *); // optional custom free()
    void           *arg1;
    void           *arg2;
    size_t          size;
    long            type;
    long            flg;
//    uint8_t         _pad[__STRUCT_MEMEXT_PAD];
};

/* membuf types */
#define MEMBUF_FREE         0   // on free-list
#define MEMBUF_DATA         1   // dynamic allocation
#define MEMBUF_EXT          2   // external storage mapped to mbuf
#define MEMBUF_PKTHDR       3   // packet header
#define MEMBUF_SOCK         4   // socket structure
#define MEMBUF_PCB          5   // protocol control block
#define MEMBUF_ROUTE_TAB    6   // routing tables
#define MEMBUF_HOST_TAB     7   // IMP host tables
#define MEMBUF_ADR_TAB      8   // address resolution tables
#define MEMBUF_SONAME       9   // socket name
#define MEMBUF_SOOPTS       10  // socket options
#define MEMBUF_FRAG_TAB     11  // fragment reassembly hedaer
#define MEMBUF_RIGHTS       12  // access rights
#define MEMBUF_IFADDR       13  // interface address
#define MEMBUF_CONTROL      14  // extra-data protocol message
#define MEMBUF_OOBDATA      15  // expedited data
#define MEMBUF_NTYPE        16
#define MEMBUF_NOINIT       (~0L) // for allocating non-initialised mbufs */
/* membuf flg-bits */
/* membuf flags */
#define MEMBUF_EXT_BIT       (1 << 0) // associated external storage
#define MEMBUF_PKTHDR_BIT    (1 << 1) // start of record
#define MEMBUF_EOR_BIT       (1 << 2) // end of record
#define MEMBUF_RDONLY        (1 << 3) // read-only data
#define MEMBUF_BROADCAST_BIT (1 << 4) // send/received as link-level broadcast
#define MEMBUF_MULTICAST_BIT (1 << 5) // send/received as link-level multicast
#define MEMBUF_PROMISC_BIT   (1 << 6) // packet not for us
#define MEMBUF_VLANTAG_BIT   (1 << 7) // ethernet vtag is valid
#define MEMBUF_UNUSED_BIT    (1 << 8) // AVAILABLE */
#define MEMBUF_NOFREE        (1 << 9) // mbuf embedded in blk, no separate free
#define MEMBUF_PROTO_BIT1    (1 << 12)// protocol-specific flags
#define MEMBUF_PROTO_BIT2    (1 << 13)
#define MEMBUF_PROTO_BIT3    (1 << 14)
#define MEMBUF_PROTO_BIT4    (1 << 15)
#define MEMBUF_PROTO_BIT5    (1 << 16)
#define MEMBUF_PROTO_BIT6    (1 << 17)
#define MEMBUF_PROTO_BIT7    (1 << 18)
#define MEMBUF_PROTO_BIT8    (1 << 19)
#define MEMBUF_PROTO_BIT9    (1 << 20)
#define MEMBUF_PROTO_BIT10   (1 << 21)
#define MEMBUF_PROTO_BIT11   (1 << 22)
#define MEMBUF_PROTO_BIT12   (1 << 23)
/* flags purged when crossing layers */
#define MEMBUF_PROTO_BITS                                               \
    (MEMBUF_PROTO_BIT1 | MEMBUF_PROTO_BIT2 | MEMBUF_PROTO_BIT3          \
     | MEMBUF_PROTO_BIT4 | MEMBUF_PROTO_BIT5 | MEMBUF_PROTO_BIT6        \
     | MEMBUF_PROTO_BIT7 | MEMBUF_PROTO_BIT8 | MEMBUF_PROTO_BIT9        \
     | MEMBUF_PROTO_BIT10 | MEMBUF_PROTO_BIT11 | MEMBUF_PROTO_BIT12)
/* flags copied for pkthdr */
#define MEMBUF_COPYBITS                                                 \
    (MEMBUF_PKTHDRBIT | MEMBUF_EORBIT | MEMBUF_RDONLY                   \
     | MEMBUF_BROADCAST_BIT | MEMBUF_MULTICAST_BIT | MEMBUF_PROMISC_BIT \
     | MEMBUF_VLANTAG_BIT                                               \
     | MEMBUF_PROTO_BITS)
#define MEMBUF_EXT_COPYBITS                                             \
    (MEMBUF_EXT_BIT | MEMBUF_EXT_BITS)
#define __STRUCT_MEMHDR_SIZE                                            \
    (3 * sizeof(long) + 3 * sizeof(void *) + sizeof(size_t))
#define __STRUCT_MEMHDR_PAD                                             \
    (rounduppow2(__STRUCT_MEMHDR_SIZE, CLSIZE) - __STRUCT_MEMHDR_SIZE)
struct memhdr {
    uint8_t       *adr;         // data address
    volatile long  nref;        // # of references
    size_t         len;         // # of bytes in membuf
    long           type;        // type of data
    long           flg;         // flags
    struct membuf *next;        // next buffer in chain
    struct membuf *nextpkt;     // next chain in queue/record
};

#define membuftodata(mb, type) ((type)((mb)->hdr.adr))
#define memdatatobuf(ptr)      (rounddownpow2((uintptr_t ptr), MEMBUFSIZE))
#define membufexthdr(mb)       (&((mb)->data.hdr.mem.ext))
#define membufextadr(mb)       ((mb)->data.hdr.ext.adr)
#define membufextnref(mb)                                               \
    (!(((mb)->data.hdr.ext.flg & MEMBUF_EXT_EXTREF))                    \
     ? (&((mb)->data.hdr.mem.ext.nref.val))                             \
     : ((mb)->data.hdr.mem.ext.nref.ptr))
#define membufextsize(mb)      ((mb)->data.hdr.mem.ext.size)
#define membufpkthdr(mb)       (&((mb)->data.hdr.pkt))
#define membufpktlen(mb)       ((mb)->data.hdr.pkt.len)
#define membufpktdata(mb)      ((mb)->data.hdr.mem.buf)
struct membuf {
    struct memhdr              hdr;
    union {
        struct {
            struct pkthdr      pkt;     // MEMBUF_PKTHDR is set
            union {
                struct memext  ext;     // MEMBUF_EXT is set
                char           buf[MEMBUF_PKTLEN];
            } mem;
        } hdr;
        char                   buf[MEMBUF_LEN];
    } data;
};

struct memblk {
    uint8_t       *ptr;
    struct memblk *next;
    uint8_t        data[MEMBUF_BLK_SIZE - 2 * sizeof(void *)];
};

#define __STRUCT_MEMBUFBKT_SIZE                                         \
    (4 * sizeof(long) + 2 * sizeof(void *))
#define __STRUCT_MEMBUFBKT_PAD                                          \
    (CLSIZE - __STRUCT_MEMBUFBKT_SIZE)
struct membufbkt {
    volatile long      lk;
    long               flg;
    long               nbuf;
    struct membuf     *buflist;
    long               nblk;
    struct memcluster *blklist;
    uint8_t            _pad[__STRUCT_MEMBUFBKT_PAD];
};

#endif /* __KERN_MEM_BUF_H__ */

