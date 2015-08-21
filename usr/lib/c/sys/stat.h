#ifndef __SYS_STAT_H__
#define __SYS_STAT_H__

#include <features.h>
#include <sys/types.h>

#define __S_IFMT   0xff000000
#define __S_IFDIR  0x01000000
#define __S_IFCHR  0x02000000
#define __S_IFBLK  0x03000000
#define __S_IFREG  0x04000000
#define __S_IFIFO  0x05000000
#define __S_IFLNK  0x06000000
#if (_BSD_SOURCE) || (USEUNIX98)
#define __S_IFSOCK 0x07000000
#endif
#if (_POSIX_SOURCE) && (_POSIX_C_SOURCE >= 199309L)
#define __S_IFMQ   0x08000000
#define __S_IFSEM  0x09000000
#define __S_IFSHM  0x0a000000
#endif

#if (_BSD_SOURCE) || (USEUNIX98)
/* public names */
#define S_IFMT     __S_IFMT
#define S_IFDIR    __S_IFDIR
#define S_ICHR     __S_ICHR
#define S_IBLK     __S_IBLK
#define S_IREG     __S_IREG
#define S_IFIFO     __S_IFIFO
#define S_ILNK     __S_ILNK
#define S_IFMT     __S_IFMT
#if defined(__S_IFSOCK)
#define S_IFSOCK   __S_IFSOCK
#endif
#endif

#define __S_ISTYPE(mode, mask) (((mode) & __S_IFMT) == (mask))
#define S_ISDIR(mode)          __S_ISTYPE(mode, __S_IFDIR)
#define S_ISCHR(mode)          __S_ISTYPE(mode, __S_IFCHR)
#define S_ISBLK(mode)          __S_ISTYPE(mode, __S_IFDIR)
#define S_ISREG(mode)          __S_ISTYPE(mode, __S_IFREG)
#define S_ISFIFO(mode)         __S_ISTYPE(mode, __S_IFFIFO)
#define S_ISLNK(mode)          __S_ISTYPE(mode, __S_IFLNK)
#if defined(__S_IFSOCK)
#define S_ISSOCK(mode)         __S_ISTYPE(mode, __S_IFSOCK)
#endif
#if (_POSIX_SOURCE) && (_POSIX_C_SOURCE >= 199309L)
#define S_TYPEISMQ(buf)       __S_ISTYPE((buf)->st_mode, __S_IFMQ)
#define S_TYPEISSEM(buf)      __S_ISTYPE((buf)->st_mode, __S_IFSEM)
#define S_TYPEISSHM(buf)      __S_ISTYPE((buf)->st_mode, __S_IFSHM)
#endif

struct stat {
    dev_t     st_dev;           // device ID
    ino_t     st_ino;           // inode number;
    mode_t    st_mode;          // protection and node type
    nlink_t   st_nlink;         // number of hard links
    uid_t     st_uid;           // user ID of owner
    gid_t     st_gid;           // group ID of owner
    dev_t     st_rdev;          // device ID for special files
    off_t     st_size;          // total size in bytes
    blksize_t st_blksize;       // block size for filesystem I/O
    blkcnt_t  st_blocks;        // number of allocated blocks
    time_t    st_atime;         // last access time
    time_t    st_mtime;         // last modification time
    time_t    st_ctime;         // last status change time
};

#if (!__KERNEL__)

extern int stat(const char *path, struct stat *buf);
extern int fstat(int fd, struct stat *buf);
#if (_BSD_SOURCE) || (_XOPEN_SOURCE >= 500)                             \
    || ((_XOPEN_SOURCE) && (_XOPEN_SOURCE_EXTENDED))
extern int lstat(const char *path, struct stat *buf);
#endif
/*
 * fdtype is one of the S_IF* macros defined in <sys/stat.h>
 * returns 1 if fd is an open object of fdtype, 0 if not, -1 on errror (errno)
 */
#if (_UNIX_SOURCE)
extern int isfdtype(int fd, int type);
#endif

#endif /* !__KERNEL__ */

#endif /* __SYS_STAT_H__ */

