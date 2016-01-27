#ifndef __KERN_MEM_MEM_H__
#define __KERN_MEM_MEM_H__

#include <stddef.h>
#include <stdint.h>
//#include <net/if.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/types.h>
#include <kern/perm.h>

/* allocation flags */
#if 0
#define MEMFREE     0x01L
#define MEMWIRE     0x02L
#define MEMZERO     0x04L
#define MEMFLGBITS  0x07L
#define MEMNFLGBIT  3
#endif
#define MEMFREE     0x00000001UL
#define MEMZERO     0x00000002UL
#define MEMWIRE     0x00000004UL
#define MEMFLGBITS  (MEMFREE | MEMZERO | MEMWIRE)
#define MEMNFLGBIT  4

#define MEMBUF_SIZE 256                 // membuf size
#define MEMCL_SHIFT (PAGESIZELOG2 - 1)
#define MEMCL_SIZE  (1 << MEMCL_SHIFT)  // cluster smaller than PAGESIZE

//#define MEM_AVOID_CACHELINE_SHARE  1
#define MEM_CONST_SIZE_TRICK 1
#if (MEM_CONST_SIZE_TRICK)
#define memgetbkt(sz) memfastbkt(sz)
#else
#define memgetbkt(sz) memcalcbkt(sz)
#endif

struct membuf;

/* record/packet header in first membuf of chain; MEMBUF_PKTHDR is set */
#define __STRUCT_PKTHDR_SIZE                                            \
    (3 * sizeof(void *) + 2 * sizeof(int32_t) + sizeof(size_t))
#define __STRUCT_PKTHDR_PAD                                             \
    (rounduppow2(__STRUCT_PKTHDR_SIZE, CLSIZE) - __STRUCT_PKTHDR_SIZE)
struct pkthdr {
    struct ifnet  *rcvif;       // rcv interface
    size_t         len;         // total packet length
    void          *hdr;         // packet header
    int32_t        chksumflg;   // checksum flags
    int32_t        chksum;      // checksum data
    struct membuf *aux;         // extra data buffer, e.g. IPSEC
    uint8_t        _pad[__STRUCT_PKTHDR_PAD];
};

#define __STRUCT_MEMEXT_SIZE                                            \
    (sizeof(long) + 3 * sizeof(void *) + sizeof(size_t) + sizeof(m_ureg_t))
#define __STRUCT_MEMEXT_PAD                                             \
    (rounduppow2(__STRUCT_MEMEXT_SIZE, CLSIZE) - __STRUCT_MEMEXT_SIZE)
struct memext {
    m_ureg_t   refcnt;
    void      *extbuf;
    void     (*extfree)(void *, void *);
    void      *extargs;
    size_t     size;
    long       type;
    uint8_t    _res[__STRUCT_MEMEXT_PAD];
};

#define MEMBUF_HDRSIZE    offsetof(struct membuf, data)
#define MEMBUF_PKTHDRSIZE offsetof(struct membuf, pktdata)
#define MEMBUF_LEN        (MEMBUF_SIZE - MEMBUF_HDRSIZE)
#define MEMBUF_PKTLEN     (MEMBUF_SIZE - MEMBUF_PKTHDRSIZE)
/* membuf types */
#define MEMBUF_FREE    0        // on free-list
#define MEMBUF_EXT     1        // external storage mapped to mbuf
#define MEMBUF_DATA    2        // dynamic allocation
#define MEMBUF_PKTHDR  3        // packet header
#define MEMBUF_SOCK    4        // socket structure
#define MEMBUF_PCB     5        // protocol control block
#define MEMBUF_RTAB    6        // routing tables
#define MEMBUF_HTAB    7        // IMP host tables
#define MEMBUF_ATAB    8        // address resolution tables
#define MEMBUF_SONAME  9        // socket name
#define MEMBUF_SOOPTS  10        // socket options
#define MEMBUF_FTAB    11       // fragment reassembly header
#define MEMBUF_RIGHTS  12       // access rights
#define MEMBUF_IFADDR  13       // interface address
#define MEMBUF_CONTROL 14       // extra-data protocol message
#define MEMBUF_OOBDATA 15       // expedited data
struct membuf {
    void                      *adr;    // data address
    size_t                     size;    // # of bytes in membuf
    long                       type;    // type of data
    long                       flg;     // flags
    struct membuf             *prev;    // previous buffer in chain
    struct membuf             *next;    // next buffer in chain
    struct membuf             *nextpkt; // next chain in queue/record
    union {
        struct {
            struct pkthdr      pkthdr;  // MEMBUF_PKTHDR is set
            union {
                struct memext  ext;     // MEMBUF_EXT is set
                char           pktbuf[0];
            } u;
        } s;
        char                   buf[0];
    } data ALIGNED(CLSIZE);
};

#define __STRUCT_MEMHDR_SIZE                                            \
    (sizeof(long) + sizeof(void *))
#define __STRUCT_MEMHDR_PAD                                             \
    (roundup(__STRUCT_MEMHDR_SIZE, CLSIZE) - __STRUCT_MEMHDR_SIZE)
struct memhdr {
    volatile long  lk;
    void          *list;
    uint8_t        _pad[__STRUCT_MEMHDR_PAD];
};

struct memzone {
    struct memhdr  tab[PTRBITS];
    uintptr_t      base;
    unsigned long  nhdr;
    void          *hdrtab;
};

static __inline__ unsigned long
memcalcbkt(unsigned long size)
{
    unsigned long bkt = PTRBITS;
    unsigned long nlz;

    if (size) {
        nlz = lzerol(size);
        bkt -= nlz;
        if (powerof2(size)) {
            bkt--;
        }
    }

    return bkt;
}

/* use compiler optimizations to evaluate bucket for constant allocation size */
#if (MEM_CONST_SIZE_TRICK)
#if (PTRBITS <= 32)
#define memfastbkt(sz)                                                  \
    ((!isconst(sz)                                                      \
      ? memcalcbkt(sz)                                                  \
      : (((sz) <= MAGMIN)                                               \
         ? MAGMINLOG2                                                   \
         : (((sz) <= (1UL << 4))                                        \
            ? 4                                                         \
            : (((sz) <= (1UL << 5))                                     \
               ? 5                                                      \
               : (((sz) <= (1UL << 6))                                  \
                  ? 6                                                   \
                  : (((sz) <= (1UL << 7))                               \
                     ? 7                                                \
                     : (((sz) <= (1UL << 8))                            \
                        ? 8                                             \
                        : (((sz) <= (1UL << 9))                         \
                           ? 9                                          \
                           : (((sz) <= (1UL << 10))                     \
                              ? 10                                      \
                              : (((sz) <= (1UL << 11))                  \
                                 ? 11                                   \
                                 : (((sz) <= (1UL << 12))               \
                                    ? 12                                \
                                    : (((sz) <= (1UL << 13))            \
                                       ? 13                             \
                                       : (((sz) <= (1UL << 14))         \
                                          ? 14                          \
                                          : (((sz) <= (1UL << 15))      \
                                             ? 15                       \
                                             : (((sz) <= (1UL << 16))   \
                                                ? 16                    \
                                                : (((sz) <= (1UL << 17)) \
                                                   ? 17                 \
                                                   : (((sz) <= (1UL << 18)) \
                                                      ? 18              \
                                                      : (((sz) <= (1UL << 19) \
                                                          ? 19          \
                                                          : (((sz) <= (1UL << 20)) \
                                                             ? 20       \
                                                             : (((sz) <= (1UL << 21)) \
                                                                ? 21    \
                                                                : (((sz) <= (1UL << 22)) \
                                                                   ? 22 \
                                                                   : (((sz) <= (1UL << 23)) \
                                                                      ? 23 \
                                                                      : (((sz) <= (1UL << 24)) \
                                                                         ? 24 \
                                                                         : (((sz) <= (1UL << 25)) \
                                                                            ? 25 \
                                                                            : (((sz) <= (1UL << 26)) \
                                                                               ? 26 \
                                                                               : (((sz) <= (1UL << 27)) \
                                                                                  ? 27 \
                                                                                  : (((sz) <= (1UL << 28)) \
                                                                                     ? 28 \
                                                                                     : (((sz) <= (1UL << 29)) \
                                                                                        ? 29 \
                                                                                        : (((sz) <= (1UL << 30)) \
                                                                                           ? 30 \
                                                                                           : (((sz) <= (1UL << 31)) \
                                                                                              ? 31 \
                                                                                              : 0xff))))))))))))))))))))))))))))))))
#else
#define memfastbkt(sz)                                                  \
    ((!isconst(sz)                                                      \
      ? memcalcbkt(sz)                                                  \
      : (((sz) <= MAGMIN)                                               \
         ? MAGMINLOG2                                                   \
         : (((sz) <= (UINT64_C(1) << 4))                                        \
            ? 4                                                         \
            : (((sz) <= (UINT64_C(1) << 5))                                     \
               ? 5                                                      \
               : (((sz) <= (UINT64_C(1) << 6))                                  \
                  ? 6                                                   \
                  : (((sz) <= (UINT64_C(1) << 7))                               \
                     ? 7                                                \
                     : (((sz) <= (UINT64_C(1) << 8))                            \
                        ? 8                                             \
                        : (((sz) <= (UINT64_C(1) << 9))                         \
                           ? 9                                          \
                           : (((sz) <= (UINT64_C(1) << 10))                     \
                              ? 10                                      \
                              : (((sz) <= (UINT64_C(1) << 11))                  \
                                 ? 11                                   \
                                 : (((sz) <= (UINT64_C(1) << 12))               \
                                    ? 12                                \
                                    : (((sz) <= (UINT64_C(1) << 13))            \
                                       ? 13                             \
                                       : (((sz) <= (UINT64_C(1) << 14))         \
                                          ? 14                          \
                                          : (((sz) <= (UINT64_C(1) << 15))      \
                                             ? 15                       \
                                             : (((sz) <= (UINT64_C(1) << 16))   \
                                                ? 16                    \
                                                : (((sz) <= (UINT64_C(1) << 17)) \
                                                   ? 17                 \
                                                   : (((sz) <= (UINT64_C(1) << 18)) \
                                                      ? 18              \
                                                      : (((sz) <= (UINT64_C(1) << 19) \
                                                          ? 19          \
                                                          : (((sz) <= (UINT64_C(1) << 20)) \
                                                             ? 20       \
                                                             : (((sz) <= (UINT64_C(1) << 21)) \
                                                                ? 21    \
                                                                : (((sz) <= (UINT64_C(1) << 22)) \
                                                                   ? 22 \
                                                                   : (((sz) <= (UINT64_C(1) << 23)) \
                                                                      ? 23 \
                                                                      : (((sz) <= (UINT64_C(1) << 24)) \
                                                                         ? 24 \
                                                                         : (((sz) <= (UINT64_C(1) << 25)) \
                                                                            ? 25 \
                                                                            : (((sz) <= (UINT64_C(1) << 26)) \
                                                                               ? 26 \
                                                                               : (((sz) <= (UINT64_C(1) << 27)) \
                                                                                  ? 27 \
                                                                                  : (((sz) <= (UINT64_C(1) << 28)) \
                                                                                     ? 28 \
                                                                                     : (((sz) <= (UINT64_C(1) << 29)) \
                                                                                        ? 29 \
                                                                                        : (((sz) <= (UINT64_C(1) << 30)) \
                                                                                           ? 30 \
                                                                                           : (((sz) <= (UINT64_C(1) << 31)) \
                                                                                              ? 31 \
                                                                                              : (((sz) <= (UINT64_C(1) << 32)) \
                                                                                                 ? 32 \
                                                                                                 : (((sz) <= (UINT64_C(1) << 33)) \
                                                                                                    ? 33 \
                                                                                                    : (((sz) <= (UINT64_C(1) << 34)) \
                                                                                                       ? 34 \
                                                                                                       : (((sz) <= (UINT64_C(1) << 35)) \
                                                                                                          ? 35 \
                                                                                                          :(((sz) <= (UINT64_C(1) << 36)) \
                                                                                                            ? 36 \
                                                                                                            : (((sz) <= (UINT64_C(1) << 37)) \
                                                                                                               ? 37 \
                                                                                                               : (((sz) <= (UINT64_C(1) << 38)) \
                                                                                                                  ? 38 \
                                                                                                                  : (((sz) <= (UINT64_C(1) << 39)) \
                                                                                                                     ? 39 \
                                                                                                                     : (((sz) <= (UINT64_C(1) << 40)) \
                                                                                                                        ? 40 \
                                                                                                                        : (((sz) <= (UINT64_C(1) << 41)) \
                                                                                                                           ? 41 \
                                                                                                                           : (((sz) <= (UINT64_C(1) << 42)) \
                                                                                                                              ? 42 \
                                                                                                                              : (((sz) <= (UINT64_C(1) << 43)) \
                                                                                                                                 ? 43 \
                                                                                                                                 : (((sz) <= (UINT64_C(1) << 44)) \
                                                                                                                                    ? 44 \
                                                                                                                                    : (((sz) <= (UINT64_C(1) << 45)) \
                                                                                                                                       ? 45 \
                                                                                                                                       : (((sz) <= (UINT64_C(1) << 46)) \
                                                                                                                                          ? 46 \
                                                                                                                                          : (((sz) <= (UINT64_C(1) << 47)) \
                                                                                                                                             ? 47 \
                                                                                                                                             : (((sz) <= (UINT64_C(1) << 48)) \
                                                                                                                                                ? 48 \
                                                                                                                                                : (((sz) <= (UINT64_C(1) << 49)) \
                                                                                                                                                   ? 49 \
                                                                                                                                                   : (((sz) <= (UINT64_C(1) << 50)) \
                                                                                                                                                      ? 50 \
                                                                                                                                                      : (((sz) <= (UINT64_C(1) << 51)) \
                                                                                                                                                         ? 51 \
                                                                                                                                                         : (((sz) <= (UINT64_C(1) << 52)) \
                                                                                                                                                            ? 52 \
                                                                                                                                                            : (((sz) <= (UINT64_C(1) << 53)) \
                                                                                                                                                               ? 53 \
                                                                                                                                                               : (((sz) <= (UINT64_C(1) << 54)) \
                                                                                                                                                                  ? 54 \
                                                                                                                                                                  : (((sz) <= (UINT64_C(1) << 55)) \
                                                                                                                                                                     ? 55 \
                                                                                                                                                                     : (((sz) <= (UINT64_C(1) << 56)) \
                                                                                                                                                                        ? 56 \
                                                                                                                                                                        : (((sz) <= (UINT64_C(1) << 57)) \
                                                                                                                                                                           ? 57 \
                                                                                                                                                                           : (((sz) <= (UINT64_C(1) << 58)) \
                                                                                                                                                                              ? 58 \
                                                                                                                                                                              : (((sz) <= (UINT64_C(1) << 59)) \
                                                                                                                                                                                 ? 59 \
                                                                                                                                                                                 : (((sz) <= (UINT64_C(1) << 60)) \
                                                                                                                                                                                    ? 60 \
                                                                                                                                                                                    : (((sz) <= (UINT64_C(1) << 61)) \
                                                                                                                                                                                       ? 61 \
                                                                                                                                                                                       : (((sz) <= (UINT64_C(1) << 62)) \
                                                                                                                                                                                          ? 62 \
                                                                                                                                                                                          : (((sz) <= (UINT64_C(1) << 63)) \
                                                                                                                                                                                             ? 63 \
                                                                                                                                                                                             : 0xff))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))
#endif
#endif

#endif /* __KERN_MEM_MEM_H__ */

