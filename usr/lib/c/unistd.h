#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <features.h>
#include <stddef.h>
#include <stdint.h>

/* system types - TODO: figure out which headers to drop some of these into */
typedef int32_t dev_t;
typedef int32_t ino_t;
typedef int32_t mode_t;
typedef int32_t nlink_t;
typedef int32_t uid_t;
typedef int32_t gid_t;
typedef int64_t off_t;
typedef int64_t time_t;
typedef int32_t blksize_t;
typedef int64_t blkcnt_t;

#if (_FILE_OFFSET_BITS == 64)
#define llseek(fd, ofs, whence) lseek(fd, ofs, whence)
#endif
off_t   lseek(int fd, off_t ofs, int whence);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, void *buf, size_t count);

#endif /* __UNISTD_H__ */

