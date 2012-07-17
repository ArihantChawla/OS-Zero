#ifndef __FCNTL_H__
#define __FCNTL_H__

#include <features.h>

#if (_XOPEN_SOURCE >= 600 || _POSIX_C_SOURCE >= 200112L)
int posix_fallocate(int fd, off_t offste, off_t len);
#endif
#if (_POSIX_C_SOURCE >= 200112L)
int posix_fadvise(int fd, off_t offset, size_t len, int advice);
#if (_GNU_SOURCE)
ssize_t readahead(int fd, off64_t ofs, size_t count);
#endif

#endif /* __FCNTL_H__ */

