#ifndef __KERN_IO_BUF_H__
#define __KERN_IO_BUF_H__

#define BUFNBLKBIT 64

#include <stdint.h>
#include <kern/perm.h>

typedef uint64_t blkid_t;

#define BUFSIZE     (1UL << BUFSIZELOG2)
#define BUFSIZELOG2 16

#define BUFNEVICT   8
#define BUFNBYTE    (32768 * 1024)
#define BUFNBLK     (BUFNBYTE >> BUFSIZELOG2)

#define BUFNET  (1U << (PERMNBIT + 1))  // create header, precalculate checksum
#define BUFNOLK (1U << (PERMNBIT + 2))  // don't lock on access

/* API */
void *bufalloc(void);

#define devgetblk(buf, blk) devfindblk(buf, blk, 0)

#if (BUFNBLKBIT == 64)
#define NLVL0BIT 16
#define NLVL1BIT 16
#define NLVL2BIT 16
#define NLVL3BIT 16
#define NLVL0BLK (1UL << NLVL0BIT)
#define NLVL1BLK (1UL << NLVL1BIT)
#define NLVL2BLK (1UL << NLVL2BIT)
#define NLVL3BLK (1UL << NLVL3BIT)
#define BUFNKEY  4
#endif

/* table item */
struct buftab {
    void *ptr;  // pointer to next table or data
    long  nref; // # of references to table items
};

struct devbuf {
    long           lk;  // mutex
    struct buftab  tab;
};

#define bufkey0(blk)                                                    \
    (((blk) >> (NLVL1BIT + NLVL2BIT + NLVL3BIT)) & ((1UL << NLVL0BIT) - 1))
#define bufkey1(blk)                                                    \
    (((blk) >> (NLVL2BIT + NLVL3BIT)) & ((1UL << NLVL1BIT) - 1))
#define bufkey2(blk)                                                    \
    (((blk) >> NLVL3BIT) & ((1UL << NLVL2BIT) - 1))
#define bufkey3(blk)                                                    \
    ((blk) & ((1UL << NLVL3BIT) - 1))

#endif /* __KERN_IO_BUF_H__ */

