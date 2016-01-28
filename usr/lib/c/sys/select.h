#ifndef __SYS_SELECT_H__
#define __SYS_SELECT_H__

#include <features.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#if !defined(__time_types_defined)
#include <share/time.h>
#endif
#if !defined(__struct_timeval_defined)
#include <sys/share/time.h>
#endif
//#include <sys/types.h>
//#include <zero/param.h>
//#if !defined(_POSIX_SOURCE) && (USEBSD) && !defined(NFDBITS)
#if !defined(NFDBITS)
#include <kern/conf.h>
#endif

#if !defined(CHAR_BIT)
#include <share/limits.h>
#endif

#if !defined(__KERNEL__)

#if !defined(FD_SETSIZE)

#if (_ZERO_SOURCE)
#include <kern/conf.h>
#endif
#if defined(NPROCFD) && !defined(FD_SETSIZE)
#define FD_SETSIZE NPROCFD
#elif (_POSIX_SOURCE) && !defined(FD_SETSIZE)
#define FD_SETSIZE _POSIX_FD_SET_SIZE
#elif (USEBSD) && !defined(NFDBITS)
#include <sys/sysmacros.h>
#define FD_SETSIZE NFDBITS
#endif

typedef long       fd_mask;
#define NFDBITS    (sizeof(fd_mask) * CHAR_BIT)

struct fd_set {
#if (USEXOPEN)
    fd_mask fds_bits[FD_SETSIZE / NFDBITS];
#else
    fd_mask __fds_bits[FD_SETSIZE / NFDBITS];
#endif
};
typedef struct fd_set fd_set;

#define FD_SET(fd, set)    setbit(set->fd_bits, fd)
#define FD_CLR(fd, set)    clrbit(set->fd_bits, fd)
#define FD_ISSET(fd, set)  bitset(set->fd_bits, fd)
#define FD_ZERO(set)       memset(set->fd_bits, 0, FD_SETSIZE / CHAR_BIT)
#if (USEBSD)
#define FD_COPY(src, dest) memcpy(dest, src, sizeof(fd_set))
#endif

#endif /* !defined(FD_SETSIZE) */

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

#endif /* !defined(__KERNEL__) */

#endif /* __SYS_SELECT_H__ */

