#ifndef __KERN_IO_FS_FS0_H__
#define __KERN_IO_FS_FS0_H__

#include <stdint.h>

#define FS0DEFBLKSIZE  4096
#define FS0DEFFRAGSIZE 1024

#define FS0NAMELEN     255
#define FS0INODESIZE   512

/*
 * NOTES
 * -----
 * - all data structures use LSB byte- and bit-order
 */

/*
 * inode structure
 * ---------------
 * - inodes are fixed-size of 512 bytes
 * - for meta-blocks, id == meta - meta == type
 * - for device-blocks, ofs == (((major) << 32) | (minor))
 */
/* type-member values */
#define FS0REG      0   // file-like objects
#define FS0DIR      1   // directory
#define FS0METANODE 2   // metadata
#define FS0BLKGROUP 3   // block-group
#define FS0INODE    4   // file/node allocation info
#define FS0MOUNTPNT 5   // mountpoint
#define FS0CHRDEV   6   // character special
#define FS0BLKDEV   7   // block special
#define FS0SYMLINK  8   // symbolic link
#define FS0FIFO     9   // named pipe
#define FS0SOCK     10  // socket
#define FS0MQ       11  // message queue
#define FS0SEM      12  // semaphore
#define FS0SHM      13  // shared memory
/* flag-bits for flg */
#define INODENOBUF 0x80000000
struct fs0inode {
    unsigned char name[FS0NAMELEN + 1]; // 256 bytes @ 0        - name
    /* 32-bit fields */
    uint32_t      type;                 // 4 bytes @ 256        - node type
    uint32_t      id;                   // 4 bytes @ 260        - inode number
    uint32_t      uid;                  // 4 bytes @ 264        - user ID
    uint32_t      gid;                  // 4 bytes @ 268        - group ID
    uint32_t      flg;                  // 4 bytes @ 272        - perms & bits
    uint32_t      meta;                 // 4 bytes @ 276        - meta-node ID
    /* 64-bit fields */
    uint64_t      nblk;                 // 8 bytes @ 280        - # of blocks
    uint64_t      size;                 // 8 bytes @ 288        - size in bytes
    uint64_t      ofs;                  // 8 bytes @ 296        - in-node offset
    uint64_t      ctime;                // 8 bytes @ 304        - creation time
    uint64_t      mtime;                // 8 bytes @ 312        - mod time
    uint64_t      atime;                // 8 bytes @ 320        - access time
    /* 32-bit fields */
    uint32_t      blkgrp;               // 4 bytes @ 328        - fs block-group
    uint32_t      indir1;               // 4 bytes @ 332        - indir-block #1
    uint32_t      indir2;               // 4 bytes @ 336        - indir-block #2
    uint32_t      indir3;               // 4 bytes @ 340        - indir-block #3
    uint32_t      _reserved[11];        // 44 bytes @ 340       - for extensions
    uint32_t      dir[32];              // 128 bytes @ 384      - direct-blocks
};

/* FS flag-bits in flg */
#define FSDIRTY    0x80000000
#define FSCHECK    0x40000000
#define FSRDONLY   0x20000000
#define FSSMOUNTED 0x10000000
struct fs0supblk {
    uint32_t ver;       // filesystem version
    uint32_t uid;       // mount owner ID
    uint32_t gid;       // mount group ID
    uint32_t flg;       // filesystem flags
    uint32_t blksize;   // block-size
    uint32_t fragsize;  // fragment-size
    uint32_t readsize;  // read-ahead buffering
    uint32_t writesize; // write-buffer size (speculative allocation)
    uint64_t dev;       // physical device such as disk ID
    uint64_t nblk;      // number of blocks on FS
};

/* block groups */

struct fs0blkgrp {
    struct fs0supblk supblk;    // superblock copy
    uint32_t         ninode;    // number of inodes in group
    uint32_t         nblk;      // number of data blocks in group
    uint32_t         blk1num;   // # of first data block (in disk units)
    uint32_t         pad;       // pad to 64-bit boundary
};

#endif /* __KERN_IO_FS_FS0_H__ */

