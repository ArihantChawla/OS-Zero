#ifndef __SYS_UIO_H__
#define __SYS_UIO_H__

#include <sys/types.h>

#define UIO_MAXIOV 1024

struct iovec {
    void   *iov_base;
    size_t  iov_len;
};

extern ssize_t readv(int fd, const struct iovec *iovec, int cnt);
extern ssize_t write(int fd, const struct iovec *iovec, int cnt);

#endif __SYS_UIO_H__

