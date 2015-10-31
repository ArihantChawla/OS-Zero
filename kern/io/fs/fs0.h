#include <stdint.h>

#define FS0NAMELEN  256
#define FS0INODELEN 512

struct fs0inode {
    unsigned char name[FS0NAMELEN];     // 256 bytes @ 0        - name
    uint32_t      uid;                  // 4 bytes @ 256        - user ID
    uint32_t      gid;                  // 4 bytes @ 260        - group ID
    uint32_t      flg;                  // 4 bytes @ 264        - perms & bits
    uint32_t      meta;                 // 4 bytes @ 272        - meta-node ID
    uint64_t      size;                 // 8 bytes @ 276        - size in bytes
    uint64_t      ofs;                  // 8 bytes @ 284        - in-node offset
    uint32_t      grp;                  // 4 bytes @ 292        - fs block-group
    uint32_t      indir1;               // 4 bytes @ 296        - indir-block #1
    uint32_t      indir2;               // 4 bytes @ 300        - indir-block #2
    uint32_t      indir3;               // 4 bytes @ 304        - indir-block #3
    uint32_t      _reserved[19];        // 76 bytes @ 308       - improvements
    uint32_t      dir[32];              // 128 bytes @ 384      - direct-blocks
};

