#ifndef __SYS_SELECT_H__
#define __SYS_SELECT_H__

#include <features.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <zero/param.h>

#if (_POSIX_SOURCE)
#define FD_SETSIZE _POSIX_FD_SETSIZE
#else
#define FD_SETSIZE NPROCFD
#endif

typedef long       fd_mask;
#define NFDBITS    (sizeof(fd_mask) * CHAR_BIT)

struct fd_set {
#if (USEXOPEN)
    fd_mask fds_bits[FD_SETSIZE >> (LONGSIZELOG2 + 3)];
#else
    fd_mask __fds_bits[FD_SETSIZE >> (LONGSIZELOG2 + 3)];
#endif
};
typedef struct fd_set fd_set;

#define FD_SET(fd, set)    setbit(set.fd_bits, fd)
#define FD_CLR(fd, set)    clrbit(set.fd_bits, fd)
#define FD_ISSET(fd, set)  bitset(set.fd_bits, fd)
#define FD_ZERO(set)       memset(set.fd_bits, 0, FD_SETSIZE / CHAR_BIT)
/* FIXME: BSD macro? */
#define FD_COPY(src, dest) memcpy(dest, src, sizeof(struct fdset))

extern int select(int nfd,
                  fd_set *__restrict readfds,
                  fd_set *__restrict writefds,
                  fd_set *__restrict excfds,
                  struct timeval *__restrict timeout);
#if (USEXOPEN2K)
extern int pselect(int nfd,
                   fd_set *__restrict readfds,
                   fd_set *__restrict writefds,
                   fd_set *__restrict excfds,
                   const struct timespec *__restrict timeout,
                   const sigset_t *__restrict sigmask);
#endif

#endif /* __SYS_SELECT_H__ */

