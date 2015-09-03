#ifndef __SYS_UIO_H__
#define __SYS_UIO_H__

#include <sys/types.h>

#define UIO_MAXIOV 1024

struct iovec {
    void   *iov_base;
    size_t  iov_len;
};

#if !defined(__KERNEL__)

extern ssize_t readv(int fd, const struct iovec *iovec, int cnt);
extern ssize_t writev(int fd, const struct iovec *iovec, int cnt);

#endif /* !defined(__KERNEL__) */

#endif /* __SYS_UIO_H__ */

