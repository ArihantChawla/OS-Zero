#ifndef __FCNTL_H__
#define __FCNTL_H__

#include <features.h>
#if (_XOPEN_SOURCE)
#include <sys/stat.h>
#endif
#if (_XOPEN_SOURCE)
#include <kern/io.h>
#endif

#if !defined(R_OK)
/* these constants are in <unistd.h> as well */
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0
#endif

#if !defined(F_LOCK) && (USEXOPENEXT) && !(_POSIX_SOURCE)
/* these constants also appear in <unistd.h> */
#define F_ULOCK 0
#define F_LOCK  1
#define F_TLOCK 2
#define F_TEST  3

extern int lockf(int fd, int cmd, off_t len);
#endif

#if (_GNU_SOURCE)
#define AT_FDCWD 100
#define AT_SYMLINK_NOFOLLOW 0x100
#define AT_REMOVEDIR        0x200
#define AT_SYMLINK_FOLLOW   0x400
#define AT_EACCESS          0x200
#endif

extern int creat(const char *file, mode_t mode);
extern int fcntl(int fd, int cmd, ...);
#if (_XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 200112L || (USEXOPEN2K))
extern int posix_fallocate(int fd, off_t offste, off_t len);
#endif
#if (_POSIX_C_SOURCE >= 200112L) || (USEXOPEN2K)
extern int posix_fadvise(int fd, off_t offset, size_t len, int advice);
#endif
#if (_GNU_SOURCE)
typedef int64_t off64_t;
extern ssize_t readahead(int fd, off64_t ofs, size_t count);
#endif

#if (_GNU_SOURCE)
extern int openat(int fd, const char *file, int flg);
#endif

#endif /* __FCNTL_H__ */

