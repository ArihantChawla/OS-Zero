#ifndef __KERN_MEM_BUF_H__
#define __KERN_MEM_BUF_H__

#include <kern/types.h>

#define MEMBUF_SIZE 256                 // membuf size
#define MEMCL_SHIFT (PAGESIZELOG2 - 1)
#define MEMCL_SIZE  (1 << MEMCL_SHIFT)  // cluster smaller than PAGESIZE

//struct membuf;

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
    int32_t        chksum;      // checksum data (for IP4)
    struct membuf *aux;         // extra data buffer, e.g. IPSEC
    uint8_t        _pad[__STRUCT_PKTHDR_PAD];
};

#define __STRUCT_MEMEXT_SIZE                                            \
    (sizeof(long) + 3 * sizeof(void *) + sizeof(size_t))
#define __STRUCT_MEMEXT_PAD                                             \
    (rounduppow2(__STRUCT_MEMEXT_SIZE, CLSIZE) - __STRUCT_MEMEXT_SIZE)
struct memext {
    volatile long   nref;
    void           *buf;
    void          (*free)(void *, void *);
    void           *args;
    size_t          size;
    long            type;
    uint8_t         _pad[__STRUCT_MEMEXT_PAD];
};

#define MEMBUF_HDRSIZE    roundup(offsetof(struct membuf, data), CLSIZE)
#define MEMBUF_PKTHDRSIZE 
#define MEMBUF_LEN        (MEMBUF_SIZE - MEMBUF_HDRSIZE)
#define MEMBUF_PKTLEN     (MEMBUF_SIZE - MEMBUF_PKTHDRSIZE)
/* membuf types */
#define MEMBUF_FREE       0     // on free-list
#define MEMBUF_EXT        1     // external storage mapped to mbuf
#define MEMBUF_DATA       2     // dynamic allocation
#define MEMBUF_PKTHDR     3     // packet header
#define MEMBUF_SOCK       4     // socket structure
#define MEMBUF_PCB        5     // protocol control block
#define MEMBUF_RTAB       6     // routing tables
#define MEMBUF_HTAB       7     // IMP host tables
#define MEMBUF_ATAB       8     // address resolution tables
#define MEMBUF_SONAME     9     // socket name
#define MEMBUF_SOOPTS     10    // socket options
#define MEMBUF_FTAB       11    // fragment reassembly header
#define MEMBUF_RIGHTS     12    // access rights
#define MEMBUF_IFADDR     13    // interface address
#define MEMBUF_CONTROL    14    // extra-data protocol message
#define MEMBUF_OOBDATA    15    // expedited data
#define membufpkthdr(mb) ((mb)->data.hdr.pkt)
#define membufexthdr(mb) ((mb)->data.hdr.ext.hdr)
#define membufpktadr(mb) ((mb)->data.buf)
#define membufextadr(mb) ((mb)->data.hdr.ext.hdr)
#define __STRUCT_MEMBUFHDR_SIZE                                         \
    (3 * sizeof(long) + 4 * sizeof(void *) + sizeof(size_t))
#define __STRUCT_MEMBUFHDR_PAD                                          \
    (rounduppow2(__STRUCT_MEMBUFHDR_SIZE, CLSIZE) - __STRUCT_MEMBUFHDR_SIZE)
struct membufhdr {
    void          *adr;         // data address
    volatile long  nref;        // # of references
    size_t         size;        // # of bytes in membuf
    long           type;        // type of data
    long           flg;         // flags
    struct membuf *prev;        // previous buffer in chain
    struct membuf *next;        // next buffer in chain
    struct membuf *nextbuf;     // next chain in queue/record
};

struct membuf {
    union {
        struct {
            struct pkthdr      pkt;     // MEMBUF_PKTHDR is set
            union {
                struct memext  hdr;     // MEMBUF_EXT is set
                char           buf[0];
            } ext;
        } hdr;
        char                   buf[0];
    } data;
};

#endif /* __KERN_MEM_BUF_H__ */

