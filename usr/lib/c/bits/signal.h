#ifndef __BITS_SIGNAL_H__
#define __BITS_SIGNAL_H__

#include <features.h>
#include <stdint.h>
#if (_ZERO_SOURCE)
#include <kern/signal.h>
#endif

/* internal. */
#define _sigvalid(sig)  ((sig) && (!((sig) & ~SIGMASK)))
#define _sigrt(sig)     ((sig) && (!((sig) & ~SIGRTMASK)))
#define _signorm(sig)   ((sig) && (!((sig) & SIGRTMASK)))

#if defined(__x86_64__) || defined(__amd64__) || defined(___alpha__)
#define SIG32BIT 0
#else
#define SIG32BIT 1
#endif

#define SIG_ERR      ((void (*)(int))(uintptr_t)(-1L))
#define SIG_DFL      ((void (*)(int))(uintptr_t)(0L))
#define SIG_IGN      ((void (*)(int))(uintptr_t)(1L))
#define SIG_HOLD     ((void (*)(int))(uintptr_t)(2L))

/* flags for sigprocmask() */
#define SIG_BLOCK    1
#define SIG_UNBLOCK  2
#define SIG_SETMASK  3

typedef volatile long    sig_atomic_t;
#if (SIG32BIT)
struct _sigset {
    uint32_t norm;
    uint32_t rt;
};
typedef struct _sigset   sigset_t;
#elif (LONGSIZE == 8)
typedef long             sigset_t;
#else
typedef long long        sigset_t;
#endif
typedef void           (*__sighandler_t)(int);
//typedef __sighandler_t   sighandler_t;
#if (_GNU_SOURCE)
typedef void             sighandler_t(int);
#endif

#if (_BSD_SOURCE)
typedef void           (*sig_t)(int);
#endif

/* sigaction() definitions */ 
#define SA_NOCLDSTOP SIG_NOCLDSTOP
#define SA_NOCLDWAIT SIG_NOCLDWAIT
#define SA_NODEFER   SIG_NODEFER
#define SA_RESETHAND SIG_RESETHAND
#define SA_SIGINFO   SIG_SIGINFO
/* non-POSIX */
#define SA_ONSTACK   SIG_ONSTACK
#define SA_RESTART   SIG_RESTART

#define MINSIGSTKSZ  PAGESIZE
#define SIGSTKSZ     (4 * PAGESIZE)
#define SS_ONSTACK   0x01
#define SS_DISABLE   0x02

/* special values. */
#if 0
#define SIG_ERR      ((sighandler_t)-1L)
#define SIG_DFL      ((sighandler_t)0L)
#define SIG_IGN      ((sighandler_t)1L)
#define SIG_HOLD     ((sighandler_t)2L)
#endif

struct sigaction {
    void     (*sa_handler)(int);
    void     (*sa_sigaction)(int, siginfo_t, void *); // sa_flags & SA_SIGINFO
    void     (*sa_restorer)(void);
    sigset_t  sa_mask;
    int       sa_flags;
};

#endif /* __BITS_SIGNAL_H__ */

