#include <stdint.h>

#define FS0NAMELEN  256
#define FS0INODELEN 512

/*
 * NOTES
 * -----
 * - for meta-blocks, id == meta - ofs == type
 * - for device-blocks, ofs == (((major) << 32) | (minor))
 */
/* type-member values */
#define FSMETANODE 0x00000000
#define FSINODE    0x00000001
struct fs0inode {
    unsigned char name[FS0NAMELEN];     // 256 bytes @ 0        - name
    uint32_t      type;                 // 4 bytes @ 256        - node type
    uint32_t      num;                  // 4 bytes @ 260        - inode number
    uint32_t      uid;                  // 4 bytes @ 264        - user ID
    uint32_t      gid;                  // 4 bytes @ 268        - group ID
    uint32_t      flg;                  // 4 bytes @ 272        - perms & bits
    uint32_t      meta;                 // 4 bytes @ 280        - meta-node ID
    uint64_t      nblk;                 // 8 bytes @ 288        - # of blocks
    uint64_t      size;                 // 8 bytes @ 296        - size in bytes
    uint64_t      ofs;                  // 8 bytes @ 304        - in-node offset
    uint32_t      grp;                  // 4 bytes @ 308        - fs block-group
    uint32_t      indir1;               // 4 bytes @ 312        - indir-block #1
    uint32_t      indir2;               // 4 bytes @ 316        - indir-block #2
    uint32_t      indir3;               // 4 bytes @ 320        - indir-block #3
    uint32_t      _reserved[16];        // 64 bytes @ 320       - improvements
    uint32_t      dir[32];              // 128 bytes @ 384      - direct-blocks
};

