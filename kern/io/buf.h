#ifndef __KERN_IO_BUF_H__
#define __KERN_IO_BUF_H__

#include <stdint.h>
#include <kern/conf.h>
#include <kern/perm.h>

/*
 * 64-bit off_t
 * 48 significant off_t bits for buffers
 * 2^16-byte buffer size
 */

/* buffer size */
#define BUFSIZE     (1UL << BUFSIZELOG2)
#define BUFSIZELOG2 16
/* size of buffer cache */
#define BUFNBYTE    (65536 * 1024)
/* max # of cached blocks */
#define BUFNBLK     (BUFNBYTE >> BUFSIZELOG2)
#define BUFNIDBIT   48
#define BUFNDEV     256
#define BUFDEVMASK  (BUFNDEV - 1)
#define BUFNOFSBIT  16
#define BUFNL1BIT   10
#define BUFNL2BIT   10
#define BUFNL3BIT   12
#define BUFNL1ITEM  (1UL << BUFNL1BIT)
#define BUFNL2ITEM  (1UL << BUFNL2BIT)
#define BUFNL3ITEM  (1UL << BUFNL3BIT)
#define BUFL1SHIFT  (BUFNL2BIT + BUFNL3BIT + BUFNOFSBIT)
#define BUFL2SHIFT  (BUFNL3BIT + BUFNOFSBIT)
#define BUFL3SHIFT  BUFNOFSBIT
#define BUFL1MASK   (BUFNL1ITEM - 1)
#define BUFL2MASK   (BUFNL2ITEM - 1)
#define BUFL3MASK   (BUFNL3ITEM - 1)

#define bufkey(num) (((num) >> BUFNOFSBIT) & ((UINT64_C(1) << BUFNIDBIT) - 1))

/* status values */
#define BUFLOCKED    0x01       // buffer is locked
#define BUFHASDATA   0x02       // buffer has valid data
#define BUFMUSTWRITE 0x04       // the kernel must write before reassigning
#define BUFDOINGIO   0x08       // the kernel is reading or writing data
#define BUFWAIT      0x10       // a process is waiting for buffer release
struct bufblk {
    int64_t        dev;         // device #
    int64_t        num;         // per-device block #
    int64_t        chksum;      // checksum such as IPv4 or IPv6
    long           status;      // status flags
    long           nb;          // # of bytes
    void          *data;        // in-core block data (kernel virtual address)
    long           nref;        // # of items in subtables
    struct bufblk *tabprev;     // previous block on hash chain
    struct bufblk *tabnext;     // next block on hash chain
    struct bufblk *listprev;    // previous block on free list or LRU
    struct bufblk *listnext;    // next block on free list or LRU
};

struct bufblkq {
    volatile long  lk;
    struct bufblk *head;
    struct bufblk *tail;
    long           pad;
};

#endif /* __KERN_IO_BUF_H__ */

