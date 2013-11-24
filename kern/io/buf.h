#ifndef __KERN_IO_BUF_H__
#define __KERN_IO_BUF_H__

#include <stdint.h>
#include <kern/conf.h>
#include <kern/perm.h>

#define BUFPERDEV    1

#define BUFNBYTE     (32768 * 1024)
#define BUFNBLK      (BUFNBYTE >> BUFSIZELOG2)
#define BUFNIDBIT    48

#if (BUFNIDBIT <= 48)
#define BUFNTABITEM  65536
#else
#define BUFNHASHITEM 65536
#endif

#if (BUFNIDBIT <= 48)
#define bufkey(num)                                                     \
    ((num) & UINT64_C(0xffff000000000000)                               \
    ? UINT64_C(0xffffffffffffffff)                                      \
    : ((num) >> 32))
#else
#define bufkey(num)  ((num) & (BUFNHASHITEM - 1))
#endif

/* status values */
#define BUFLOCKED    0x01       // buffer is locked
#define BUFHASDATA   0x02       // buffer has valid data
#define BUFMUSTWRITE 0x04       // the kernel must write before reassigning
#define BUFDOINGIO   0x08       // the kernel is reading or writing data
#define BUfWAIT      0x10       // a process is waiting for buffer release
struct bufblk {
#if (BUFLK)
    volatile long  lk;
#endif
    long           dev;         // device #
    uint64_t       num;         // per-device block #
    long           status;      // status flags
    long           nb;          // # of bytes
    void          *data;        // in-core block data (kernel virtual address)
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

#if (BUFPERDEV)
#define BUFNDEV 1024
struct bufdev {
#if (BUFNIDBIT <= 48)
    volatile long    lk;
    struct bufblk ***buftab;
    volatile long   *buflktab;
    struct bufdev   *prev;
    struct bufdev   *next;
#else /* BUFNIDBIT > 48 */
#endif
};
#else /* !BUFPERDEV */
#define BUFNDEV 0
#endif

#endif /* __KERN_IO_BUF_H__ */

