#ifndef __ERRNO_H__
#define __ERRNO_H__

#include <features.h>

#if !(__KERNEL__)
extern int errno;
#endif

#define ENOSYS       1
#define EINTR        2
#define ENOMEM       3
#define EAGAIN       4
#if (_BSD_SOURCE)
#define EWOULDBLOCK  EAGAIN
#endif
#define EACCES       5
#define EIO          6
#define ELOOP        7
#define ENAMETOOLONG 8
#define ENOENT       9
#define ENOTDIR      10
#define EOVERFLOW    11
#define EBADF        12
#define EFBIG        13
#define EINVAL       14
#define ENODEV       15
#define ENOSPC       16
#define ESPIPE       17
#define EILSEQ       18
#define EDOM         19
#define ERANGE       20

#endif /* __ERRNO_H__ */

