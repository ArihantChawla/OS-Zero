#ifndef __ZERO_MEM_H__
#define __ZERO_MEM_H__

/* generic memory manager definitions for libzero */

#define MEM_LK_PRIO   0                 // priority-based locklessinc.com lock
#define MEM_LK_FMTX   1                 // zero anonymous non-recursive mutex
#define MEM_LK_SPIN   2                 // zero spinlocks
#define MEM_LK_BIT    3                 // bit-locks with atomic operations

#define MEM_LK_TYPE   MEM_LK_SPIN       // type of locks to use
#define MMAP_DEV_ZERO 0                 // set to zero to mmap() with MAP_ANON

#include <limits.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>

#if (WORDSIZE == 4)
typedef int32_t   MEMWORD_T;    // machine word
#elif (WORDSIZE == 8)
typedef int64_t   MEMWORD_T;
#endif
typedef uintptr_t MEMADR_T;     // address (with possible flag/lock-bits)
typedef void *    MEMPTR_T;     // could be char * for older systems

/* macros */

/* bins shall be allocated as runs of pages of PAGESIZE */

#define MEM_BIN_MIN_SIZE   PAGESIZE     // basic heap or map allocation size
#define MEM_BIN_MAP_SIZE   CLSIZE       // bitmap size in bytes; 0-bit for free
#define MEM_BIN_MAX_BLOCKS (4 * sizeof(MEMWORD_T) * CHAR_BIT)

/* use the low-order bit of the words and pointer to lock data */
#define MEM_LK_BIT_POS     0
#define MEM_LK_BIT         (1L << MEM_LK_BIT_POS)

/*
 * bin structure for allocating runs of pages; crafted to fit in a cacheline
 * - the actual runs will be prefixed by this structure
 * - allocation shall take place with sbrk() or mmap() (see MEM_BIN_MAP_BIT)
 */

/* flag-bits for the data.info.flg-member */
#define MEM_BIN_BUSY_BIT   (1 << 0)     // zero if all blocks are free/unused
#define MEM_BIN_MAP_BIT    (1 << 1)     // mapped-memory as opposed to heap
struct membinhdr {
    MEMADR_T         adr;       // base address (PAGESIZE-aligned)
    struct membin   *prev;      // previous in-core bin (heap-trimming)
    struct membin   *next;      // next bin in chain
    /* this union should be 64 bits in size */
    union {
        MEMWORD_T    word;      // can be used for word-size access to members
        struct {
            uint8_t  blksz;     // block-size as shift-count for one
            uint8_t  npage;     // page-count as shift-count for one
            uint8_t  binsz;     // bin-size as shift-count for one
            uint8_t  flg;       // bin flag-bits
        } info;
    } data;
    MEMWORD_T        bitmap[4]; // free-bitmap; 0-bit means block is free
};

struct membinbkt {
    MEMADR_T       base;        // base address
    MEMWORD_T      slot;        // slot ID; bins are PAGESIZE << slot bytes
    MEMWORD_T      blksz;       // block-size as shift-count for one
    MEMWORD_T      nbin;        // number of bins in list
    struct membin *list;        // bi-directional list of bins in the bucket
    MEMWORD_T      nfree;       // number of free bins
    struct membin *free;        // list of free bins
    MEMWORD_T      nbuf;        // number of bins to allocate/buffer at a time
};

#endif /* __ZERO_MEM_H__ */

