#ifndef __FCNTL_H__
#define __FCNTL_H__

#include <features.h>
#include <sys/types.h>
#if (_XOPEN_SOURCE)
#include <sys/stat.h>
#endif
#include <sys/uio.h>
#if (_XOPEN_SOURCE)
#include <kern/io.h>
#endif

#if !defined(F_LOCK) && (USEXOPENEXT) && !(_POSIX_SOURCE)
extern int     lockf(int fd, int cmd, off_t len);
#endif

extern int     creat(const char *file, mode_t mode);
extern int     fcntl(int fd, int cmd, ...);
#if (_XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 200112L || (USEXOPEN2K))
extern int     posix_fallocate(int fd, off_t ofs, off_t len);
#endif
#if (_POSIX_C_SOURCE >= 200112L) || (USEXOPEN2K)
extern int     posix_fadvise(int fd, off_t ofs, size_t len, int advice);
#endif
#if defined(_GNU_SOURCE)
//typedef int64_t off64_t;
extern ssize_t readahead(int fd, off64_t ofs, size_t count);
extern int     sync_file_range(int fd, off64_t from, off64_t to,
                               unsigned int flags);
extern int     vmsplice(int outfd, const struct iovec *iov, size_t cnt,
                        unsigned int flags);
extern int     splice(int infd, off64_t inofs, int outfd, off64_t outofs,
                      size_t len, unsigned int flags);
extern int     tee(int infd, int outfd, size_t len, unsigned int flags);
#endif

#if defined(_GNU_SOURCE)
extern int     openat(int fd, const char *file, int flg);
#endif

#endif /* __FCNTL_H__ */

