#ifndef __SYS_STAT_H__
#define __SYS_STAT_H__

#include <unistd.h>

int stat(const char *path, struct stat *buf);

struct stat {
    dev_t     st_dev;           // device ID
    ino_t     st_ino;           // inode number;
    mode_t    st_mode;          // protection
    nlink_t   st_nlink;         // number of hard links
    uid_t     st_uid;           // user ID of owner
    gid_t     st_gid;           // group ID of owner
    dev_t     st_rdev;          // device ID for special files
    off_t     st_size;          // total size in bytes
    time_t    st_atime;         // last access time
    time_t    st_mtime;         // last modification time
    time_t    st_ctime;         // last status change time
    blksize_t st_blksize;       // block size for filesystem I/O
    blkcnt_t  st_blocks;        // number of allocated blocks
};

#endif /* __SYS_STAT_H__ */

