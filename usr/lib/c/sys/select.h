#ifndef __SYS_SELECT_H__
#define __SYS_SELECT_H__

#include <zero/param.h>

#if (_POSIX_SOURCE)
#define NFDSET _POSIX_FD_SETSIZE
#else
#define NFDSET 65536
#endif

#define FD_CLR(fd, set)  clrbit(set, fd)
#define FD_SET(fd, set)  setbit(set, fd)
#define FD_ZERO(set)     memset(set, 0, NFDSET >> 3)

struct fdset {
    long bits[NFDSET >> (LONGSIZELOG2 + 3)];
};
typedef struct fdset fd_set;

#endif /* __SYS_SELECT_H__ */

