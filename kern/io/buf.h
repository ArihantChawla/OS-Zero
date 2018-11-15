#ifndef __KERN_IO_BUF_H__
#define __KERN_IO_BUF_H__

#include <kern/conf.h>
#include <stddef.h>
#include <stdint.h>
#include <mach/param.h>
#include <mach/asm.h>
#include <zero/trix.h>
#include <kern/perm.h>
#if (BUFTKTLK)
#define BUF_LK_T zerotktlk
#include <mt/tktlk.h>
#endif
#include <zero/hash.h>

#define bufchkparm(dev, num)   ((dev) < DEVSMAX && (num) < DEVMAXBLKS)
#define bufsetid(dev, num)     (((uint64_t)(dev) << DEVBLKBITS) | ((num) & DEVBLKMASK))
#define bufgetdevid(id)        ((id) >> DEVBLKBITS)
#define bufgetblknum(id)       ((id) & DEVBLKMASK)
#define bufsetkblknum(id, num) ((id) | ((num) & DEVBLKMASK))
typedef uint64_t               bufid_t;

/* for locking hash chains */
#define BUF_LK_BIT_POS 1
#define BUF_LK_BIT     (1UL << BUF_LK_BIT_POS)

/* hash table parameters */
#define BUFHASHBITS    16
#define BUFHASHITEMS   (1UL << BUFHASHBITS)

/* buffer cache parameters */
#define BUFBLKS         (BUFCACHESIZE / BUFBLKSIZE)
#define BUFBLKSIZE      (1U << BUFBLKSHIFT)
#define BUFBLKSHIFT     16
/* size of buffer cache */
#define BUFCACHESIZE    (BUFMEGS * 1024 * 1024)
#define BUFMINCACHESIZE (BUFCACHESIZE >> 3)
/* max # of cached blocks */
#define DEVMAXBLKS      (UINT64_C(1) << DEVBLKBITS)
#define DEVBLKBITS      (64 - DEVBITS)
#define DEVBLKMASK      ((UINT64_C(1) << DEVBLKBITS) - 1)
#define DEVSMAX         (1U << BUFDEVBITS)
#define DEVBITS         16
#define DEVMASK         (BUFDEVS - 1)

#define bufclr(blk)                                                     \
    do {                                                                \
        long  _val = 0;                                                 \
        void *_ptr = NULL;                                              \
                                                                        \
        (blk)->nref = _val;                                             \
        (blk)->flg = _val;                                              \
        (blk)->id = _val;                                               \
        (blk)->chksum = _val;                                           \
        (blk)->prev = _ptr;                                             \
        (blk)->next = _ptr;                                             \
        (blk)->xlist = _val;                                            \
    } while (0)
/* contents for the flg-member */
#define BUFPAGEBITS     16        // 16-bit # of pages allocated
#define BUFHASDATA      (1 << 31) // buffer has valid data
#define BUFDIRTY        (1 << 30) // kernel must write before reassigning
#define BUFDOINGIO      (1 << 29) // kernel is reading or writing data
/* this structure has been carefully crafted to fit in 8 machine words */
#define BUFBLKHDRSIZE   rounduppow2(sizeof(struct bufblk), CLSIZE)
#define bufaddref(blk)  m_fetchadd(&blk->nref, 1)
#define bufdropref(blk) m_fetchadd(&blk->nref, -1)
struct bufblk {
    m_atomic_t     nref;        // # of references
    const void    *data;        // buffer address (+ perhaps flags in low bits)
    bufid_t        id;          // 16-bit device + 48-bit (low) buffer ID
    int32_t        flg;         // shift count for size + flags as above
    int32_t        chksum;      // checksum such as IPv4
    uintptr_t      xlist;       // prev ^ next for table chain
    struct bufblk *prev;        // previous block on free-list or LRU
    struct bufblk *next;        // next block on free-list or LRU
};

struct bufdev {
    BUF_LK_T lk;                // lock
    long     id;                // system descriptor
    long     flg;               // flags such as DEVIOSEQ, DEVCANSEEK(?), ...
    long     type;              // DISK, NET, OPT, TAPE, ...
    long     prio;              // device priority for I/O scheduling
    long     timelim;           // time-limit (e.g. to wait before seek)
};

long            bufinit(void);
struct bufblk * bufget(void);
void            bufaddblk(struct bufblk *blk);
struct bufblk * buffindblk(long dev, bufid_t num, long rel);

#endif /* __KERN_IO_BUF_H__ */

