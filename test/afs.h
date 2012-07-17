#ifndef __AFS_H__
#define __AFS_H__

#include <kern/perm.h>
#include <zero/cdecl.h>

#define AFSBLKSIZE 8192

typedef uint64_t afsdev_t;

#define afsdevid(major, minor)                                          \
    ((uint64_t)(major) << 32 | (minor))
/* zero and null */

#define AFSBLKLOG2   12
#define AFSBLKSIZE   (1U << AFS_BLKLOG2)
#define AFSFCBSIZE   512
#define AFSFCBPERGRP 64
#define AFSBLKPERGRP 256

#define AFSMAGIC     UINT64_C(0xa001a001a001a001)

/* filesystem control block */

/* type */
#define AFCBCDEV  0			// character device
#define AFCBBDEV  1			// block device
#define AFCBREGF  2			// regular file
#define AFCBDIR   3			// directory
#define AFCBFIFO  4			// FIFO
#define AFCBSOCK  5			// socket
#define AFCBSEM   6			// semaphore
#define AFCBRWLK  7			// read-write lock
#define AFCBMQ    8			// message queue
#define AFS_NTYPE 9

/* flags */
#define AFCBDLIO  (UINT64_C(1) << (PERMNBITS + 1)       // deadline I/O
#define AFCBSYNIO (UINT64_C(1) << (PERMNBITS + 3)	// synchronous I/O
#define AFCBHIDIO (UINT64_C(1) << (PERMNBITS + 4)	// human interface
#define AFCBLOLIO (UINT64_C(1) << (PERMNBITS + 5)	// low latency
#define AFCBNOBUF (UINT64_C(1) << (PERMNBITS + 6)       // disable buffer cache
#define AFCBREADA (UINT64_C(1) << (PERMNBITS + 7)       // enable read-ahead

/* file name encodings */
#define AFSASCII     UINT64_C(0)
#define AFSISO8859   UINT64_C(1)
#define AFSUTF8      UINT64_C(2)
#define AFSUCS2      UINT64_C(3)
#define AFSUCS4      UINT64_C(4)

#define AFCBSIZE     512
#define AFCBNPERBLK  (AFSBLKSIZE / AFCBSIZE)
/* file control block */
struct afcb {
    uint64_t magic;     // file system magic
    uint64_t size;
    uint64_t owner;
    uint64_t group;
    uint64_t type;
    uint64_t flags;
    uint64_t ctime;
    uint64_t atime;
    uint64_t mtime;
    uint64_t iblk1;
    uint64_t iblk2;
    uint64_t iblk3;
    uint64_t mblk;      // metadata index
    uint64_t mimetype;  // binary-encoded MIME type
    uint64_t namenc;    // UTF-8 by default
    uint8_t  name[256] ALIGNED(CLSIZE);
    uint16_t btab[16];
    uint64_t info[4];
    /* MUST be less than AFCBSIZE bytes so far */
};

struct afmeta {
    uint64_t mimetype;
};

#endif /* __AFS_H__ */

