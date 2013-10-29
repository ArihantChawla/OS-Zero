#ifndef __KERN_IO_BUF_H__
#define __KERN_IO_BUF_H__

#include <stdint.h>
#include <kern/conf.h>
#include <kern/perm.h>

#define BUFNBYTE     (32768 * 1024)
#define BUFNBLK      (BUFNBYTE >> BUFSIZELOG2)

#define BUFNHASHITEM 65536

#define bufkey(num)  ((num) & (BUFNHASHITEM - 1))

/* status values */
#define BUFLOCKED    0x01       // buffer is locked
#define BUFHASDATA   0x02       // buffer has valid data
#define BUFMUSTWRITE 0x04       // the kernel must write before reassigning
#define BUFDOINGIO   0x08       // the kernel is reading or writing data
#define BUfWAIT      0x10       // a process is waiting for buffer release
struct bufblk {
    long           dev;         // device #
    long           num;         // per-device block #
    long           status;      // status flags
//    long           nb;          // # of bytes
    void          *data;        // in-core block data (kernel virtual address)
    struct bufblk *hashprev;    // previous block on hash chain
    struct bufblk *hashnext;    // next block on hash chain
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

