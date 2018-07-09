#ifndef __KERN_IO_BUF_H__
#define __KERN_IO_BUF_H__

#include <kern/conf.h>
#include <stddef.h>
#include <stdint.h>
#include <mach/param.h>
#include <zero/trix.h>
#include <kern/perm.h>
#if (!BUFMULTITAB) && (BUFNEWHASH)
#include <zero/hash.h>
#endif
#if (BUFTKTLK)
#include <zero/tktlk.h>
#endif

#if (PTRSIZE == 8)
#define BUFVAL(x) INT64_C(x)
#define bufhash(val)                                                    \
    (tmhash64((uint64_t)val) & ((1UL << BUFNHASHBIT) - 1))
#define bufhashblk(dev, num)                                            \
    (tmhash64((uint64_t)bufmkhashkey(dev, num)) & ((1UL << BUFNHASHBIT) - 1))
typedef int64_t bufval_t;
#elif (PTRSIZE == 4)
#define BUFVAL(x) INT32_C(x)
#define bufhash(val)                                                    \
    (tmhash32((uint32_t)val) & ((1U << BUFNHASHBIT) - 1))
#define bufhashblk(dev, num)                                            \
    (tmhash32((uint32_t)bufmkhashkey(dev, num)) & ((1UL << BUFNHASHBIT) - 1))
typedef int32_t bufval_t;
#endif

#if (BUFTKTLK)
#define buflk(lp)   tktlk(lp)
#define bufunlk(lp) tktunlk(lp)
#endif

#if (!BUFMULTITAB)
#if (BUFNEWHASH)
#define BUFNHASHBIT 16
#define bufmkhashkey(dev, num)                                          \
    (((bufval_t)dev << (PTRBITS - BUFNDEVBIT))                          \
     | ((num) & ((BUFVAL(1) << (PTRBITS - BUFNDEVBIT)) - 1)))
#else
#define BUFNHASHBIT 16
#endif
#define BUFNHASH    (1UL << BUFNHASHBIT)
#endif

/*
 * 64-bit off_t
 * 64 - BUFMINSIZELOG2 (51) significant off_t bits for buffers
 * 2^16-byte i.e. 64KB default buffer size optimized for TCP/IP v4
 */

#define BUFMINSIZE     (1UL << BUFMINSIZELOG2)
#define BUFMINSIZELOG2 PAGESIZELOG2
#define BUFNOFSBIT     BUFMINSIZELOG2
#define BUFMAXSIZE     (1UL << BUFMAXSIZELOG2)
#define BUFMAXSIZELOG2 16
/* size of buffer cache */
#define BUFNBYTE       (BUFNMEG * 1024 * 1024)
/* max # of cached blocks */
#define BUFNBLK        (BUFNBYTE >> BUFMINSIZELOG2)
#define BUFNIDBIT      (64 - BUFMINSIZELOG2)
#define BUFNDEV        256
#define BUFNDEVBIT     8
#define BUFNDEVBITMAX  16
#define BUFDEVMASK     (BUFNDEV - 1)
#if (BUFMULTITAB)
#define BUFNL1BIT      (BUFNIDBIT - BUFNL2BIT - BUFNL3BIT)
#define BUFNL2BIT      16
#define BUFNL3BIT      16
#define BUFNL1ITEM     (1UL << BUFNL1BIT)
#define BUFNL2ITEM     (1UL << BUFNL2BIT)
#define BUFNL3ITEM     (1UL << BUFNL3BIT)
#define BUFL1SHIFT     (BUFMINSIZELOG2 + BUFNL2BIT + BUFNL3BIT)
#define BUFL2SHIFT     (BUFMINSIZELOG2 + BUFNL3BIT)
#define BUFL3SHIFT     (BUFMINSIZELOG2)
#define BUFL1MASK      (BUFNL1ITEM - 1)
#define BUFL2MASK      (BUFNL2ITEM - 1)
#define BUFL3MASK      (BUFNL3ITEM - 1)
#define BUFL4MASK      (BUFNL4ITEM - 1)
#endif /* BUFMULTITAB */

#define bufmkkey(num) (((num) >> BUFNOFSBIT) & ((UINT64_C(1) << BUFNIDBIT) - 1))
#define bufclr(blk)                                                     \
    do {                                                                \
        long  _val = 0;                                                 \
        void *_ptr = NULL;                                              \
                                                                        \
        (blk)->flg = _val;                                              \
        (blk)->num = _val;                                              \
        (blk)->nref = _val;                                             \
        (blk)->chksum = _val;                                           \
        (blk)->dev = _val;                                              \
        (blk)->prev = _ptr;                                             \
        (blk)->next = _ptr;                                             \
        (blk)->tabprev = _ptr;                                          \
        (blk)->tabnext = _ptr;                                          \
    } while (0)
/* contents for the flg-member */
#define BUFHASDATA   (1 << 31)   // buffer has valid data
#define BUFDIRTY     (1 << 30)   // kernel must write before reassigning
#define BUFDOINGIO   (1 << 29)   // kernel is reading or writing data
#if (NEWBUFBLK)
/* this structure has been carefully crafted to fit a cacheline or two */
struct bufblk {
    const void    *data;        // buffer address + flags in low bits
    long           flg;         // shift count for size + flags as above
    m_atomic_t     nref;        // # of references
    bufval_t       num;         // per-device block ID
    int16_t        chksum;      // checksum such as IPv4
    int16_t        dev;         // buffer-subsystem device ID
    struct bufblk *prev;        // previous block on free-list or LRU
    struct bufblk *next;        // next block on free-list or LRU
    struct bufblk *tabprev;     // previous block in table chain
    struct bufblk *tabnext;     // next block in table chain
};
#else
struct bufblk {
    bufval_t       dev;         // device #
    bufval_t       num;         // per-device block #
    bufval_t       chksum;      // checksum such as IPv4 or IPv6
    long           status;      // status flags
    long           nb;          // # of bytes
    long           nref;        // # of items in subtables
    void          *data;        // in-core block data (kernel virtual address)
    struct bufblk *prev;        // previous block on free list or LRU
    struct bufblk *next;        // next block on free list or LRU
    struct bufblk *tabprev;     // previous block in table chain
    struct bufblk *tabnext;     // next block in table chain
};
#endif

struct bufdev {
    m_atomic_t lk;              // lock
    long       id;              // system descriptor
    long       flg;             // flags such as DEVIOSEQ, DEVCANSEEK(?), ...
    long       type;            // DISK, NET, OPT, TAPE, ...
    long       prio;            // device priority for I/O scheduling
    long       timelim;         // time-limit (e.g. to wait before seek)
};

struct bufchain {
    m_atomic_t     lk;
    long           nitem;
    struct bufblk *list;
};

struct bufblkqueue {
#if (BUFTKTLK)
    union zerotktlk  lk;
#else
    m_atomic_t       lk;
#endif
    struct bufblk   *head;
};

long            bufinit(void);
struct bufblk * bufalloc(void);
void            bufaddblk(struct bufblk *blk);
struct bufblk * buffindblk(long dev, off_t num, long rel);

#endif /* __KERN_IO_BUF_H__ */

