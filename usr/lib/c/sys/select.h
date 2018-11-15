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
#include <sys/types.h>
//#include <zero/param.h>
//#if !defined(_POSIX_SOURCE) && (USEBSD) && !defined(PROCDESCBITS)
#if !defined(PROCDESCBITS)
#include <kern/conf.h>
#endif

#if !defined(CHAR_BIT)
#include <share/limits.h>
#endif

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

