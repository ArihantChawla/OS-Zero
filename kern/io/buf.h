#ifndef __KERN_IO_BUF_H__
#define __KERN_IO_BUF_H__

#include <kern/conf.h>
#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/perm.h>

#if (!BUFMULTITAB)
#if (BUFNEWHASH)
#define BUFNHASHBIT 16
#define bufmkhashkey(dev, num)                                          \
    (((int64_t)dev << (64 - BUFNDEVBIT))                                \
     | ((num) & ((INT64_C(1) << (64 - BUFNDEVBIT)) - 1)))
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
#define BUFMINSIZELOG2 13
#define BUFNOFSBIT     BUFMINSIZELOG2
#define BUFSIZE        (1UL << BUFSIZELOG2)
#define BUFSIZELOG2    16
/* size of buffer cache */
#define BUFNBYTE       (BUFNMEG * 1024 * 1024)
/* max # of cached blocks */
#define BUFNBLK        (BUFNBYTE >> BUFMINSIZELOG2)
#define BUFNIDBIT      (64 - BUFMINSIZELOG2)
#define BUFNDEV        256
#define BUFNDEVBIT     8
#define BUFNDEVBITMAX  16
#define BUFDEVMASK     (BUFNDEV - 1)
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

#define bufkey(num) (((num) >> BUFNOFSBIT) & ((UINT64_C(1) << BUFNIDBIT) - 1))
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
#define __STRUCT_BUFBLK_SIZE                                            \
    (sizeof(long) + 5 * sizeof(void *)                                  \
     + sizeof(int64_t) + sizeof(int32_t) + 2 * sizeof(int16_t))
#define __STRUCT_BUFBLK_PAD                                             \
    (roundup(__STRUCT_BUFBLK_SIZE, CLSIZE) - __STRUCT_BUFBLK_SIZE)
struct bufblk {
    const void    *data;        // buffer address + flags in low bits
    long           flg;         // shift count for size + flags as above
    int64_t        num;         // per-device block ID
    int32_t        nref;        // # of references
    int16_t        chksum;      // checksum such as IPv4
    int16_t        dev;         // buffer-subsystem device ID
    struct bufblk *prev;        // previous block on free-list or LRU
    struct bufblk *next;        // next block on free-list or LRU
    struct bufblk *tabprev;     // previous block in table chain
    struct bufblk *tabnext;     // next block in table chain
    uint8_t        _pad[__STRUCT_BUFBLK_PAD];
};
#else
struct bufblk {
    int64_t        dev;         // device #
    int64_t        num;         // per-device block #
    int64_t        chksum;      // checksum such as IPv4 or IPv6
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

#define __STRUCT_BUFDEV_SIZE                                            \
    (6 * sizeof(long))
#define __STRUCT_BUFDEV_PAD                                             \
    (roundup(__STRUCT_BUFDEV_SIZE, CLSIZE) - __STRUCT_BUFDEV_SIZE)
struct bufdev {
    volatile long lk;
    long          id;           // system descriptor
    long          flg;          // flags such as DEVIOSEQ, DEVCANSEEK(?), ...
    long          type;         // DISK, NET, OPT, TAPE, ...
    long          prio;         // device priority for I/O scheduling
    long          timelim;      // time-limit (e.g. to wait before seek)
    uint8_t       _pad[__STRUCT_BUFDEV_PAD];
};

#define __STRUCT_BUFCHAIN_SIZE                                          \
    (2 * sizeof(long) + sizeof(void *))
#define __STRUCT_BUFCHAIN_PAD                                           \
    (roundup(__STRUCT_BUFCHAIN_SIZE, CLSIZE) - __STRUCT_BUFCHAIN_SIZE)
struct bufchain {
    volatile long  lk;
    long           nitem;
    struct bufblk *list;
    uint8_t        _pad[__STRUCT_BUFCHAIN_PAD];
};

#define __STRUCT_BUFBLKQUEUE_SIZE                                       \
    (sizeof(long) + sizeof(void *))
#define __STRUCT_BUFBLKQUEUE_PAD                                        \
    (roundup(__STRUCT_BUFBLKQUEUE_SIZE, CLSIZE) - __STRUCT_BUFBLKQUEUE_SIZE)
struct bufblkqueue {
    volatile long  lk;
    struct bufblk *head;
    uint8_t        _pad[__STRUCT_BUFBLKQUEUE_PAD];
};

long            bufinit(void);
struct bufblk * bufalloc(void);
void            bufaddblk(struct bufblk *blk);
struct bufblk * buffindblk(long dev, off_t num, long rel);

#endif /* __KERN_IO_BUF_H__ */

