#ifndef __KERN_IO_BUF_H__
#define __KERN_IO_BUF_H__

#include <stdint.h>
#include <kern/conf.h>
#include <kern/perm.h>

#define BUFNBYTE     (65536 * 1024)
#define BUFNBLK      (BUFNBYTE >> BUFSIZELOG2)
#define BUFNIDBIT    48
#define BUFNDEV      256
#define BUFNTABITEM  65536

#define BUFNL1ITEM (1UL << 10)
#define BUFNL2ITEM (1UL << 10)
#define BUFNL3ITEM (1UL << 12)

#define bufkey(num)                                                     \
    ((num) & UINT64_C(0xffff000000000000)                               \
     ? INT64_C(-1)                                                      \
     : ((num) >> 32))

/* status values */
#define BUFLOCKED    0x01       // buffer is locked
#define BUFHASDATA   0x02       // buffer has valid data
#define BUFMUSTWRITE 0x04       // the kernel must write before reassigning
#define BUFDOINGIO   0x08       // the kernel is reading or writing data
#define BUFWAIT      0x10       // a process is waiting for buffer release
struct bufblk {
    int64_t        dev;         // device #
    int64_t        num;         // per-device block #
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

