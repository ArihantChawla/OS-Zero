#ifndef __SYS_FILE_H__
#define __SYS_FILE_H__

#if !defined(__KERNEL__)

#include <fcntl.h>

extern int flock(int fd, int op);

#endif /* __KERNEL__ */

#endif /* __SYS_FILE_H__ */

