#ifndef __BITS_SIGNAL_H__
#define __BITS_SIGNAL_H__

#undef PTHREAD
#if !defined(PTHREAD) || defined(__KERNEL__)
#define PTHREAD 0
#endif

#include <features.h>
#include <stdint.h>
#include <sys/types.h>
#if (PTHREAD)
#include <pthread.h>
#endif
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

#define SIGNO_MASK  0x000000ff
#define SIGDEFER    0x00000100
#define SIGHOLD     0x00000200
#define SIGRELSE    0x00000400
#define SIGIGNORE   0x00000800
#define SIGPAUSE    0x00001000

typedef volatile long    sig_atomic_t;
#if (SIG32BIT)
typedef struct {
    long norm;
    long rt;
}                        sigset_t;
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
#define SS_ONSTACK   0x00000001
#define SS_DISABLE   0x00000002

/* special values. */
#if 0
#define SIG_ERR      ((sighandler_t)-1L)
#define SIG_DFL      ((sighandler_t)0L)
#define SIG_IGN      ((sighandler_t)1L)
#define SIG_HOLD     ((sighandler_t)2L)
#endif

union sigval {
    int   sival_int;
    void *sival_ptr;
};

struct sigevent {
    int             sigev_notify;
    int             sigev_signo;
    union sigval    sigev_value;
    void           (*sigev_notify_function)(union sigval);
#if (PTHREAD)
    pthread_attr_t *sigev_notify_attributes;
#endif
};

#if (_POSIX_SOURCE) && (USEPOSIX199309)
typedef struct {
    int           si_signo;
    int           si_code;
    int           si_errno;
    int           si_status;
    void         *si_addr;
    pid_t         si_pid;
    uid_t         si_uid;
    ctid_t        si_ctid;
    zoneid_t      si_zoneid;
    long          si_band;
    union sigval  si_value;
} siginfo_t;
#endif /* _POSIX_SOURCE && USEPOSIX199309 */

struct sigaction {
    void     (*sa_handler)(int);
    void     (*sa_sigaction)(int, siginfo_t, void *); // sa_flags & SA_SIGINFO
    void     (*sa_restorer)(void);
    sigset_t  sa_mask;
    int       sa_flags;
};

#if (_POSIX_SOURCE)

#if (SIG32BIT)

//#define _sigptr(sp) ((struct sigset *)(sp))

/* POSIX */
#define sigemptyset(sp)                                                 \
    (!(sp)->norm && !(sp)->rt)
#define sigfillset(sp)                                                  \
    (((sp)->norm = (sp)->rt = ~UINT32_C(0)), 0)
#define sigaddset(sp, sig)                                              \
    ((!_sigvalid(sig)                                                   \
      ? (-1)                                                            \
      : (_signorm(sig)                                                  \
         ? ((sp)->norm |= (1UL << (sig)))                               \
         : ((sp)->rt |= (1UL << ((sig) - SIGRTMIN))))),                 \
     0)
#define sigdelset(sp, sig)                                              \
    ((!_sigvalid(sig)                                                   \
      ? (-1)                                                            \
      : (_signorm(sig)                                                  \
         ? ((sp)->norm &= ~(1UL << (sig)))                              \
         : ((sp)->rt &= ~(1UL << ((sig) - SIGRTMIN))))),                \
     0)
#define sigismember(sp, sig)                                            \
    ((!_sigvalid(sig)                                                   \
      ? (-1)                                                            \
      : (_signorm(sig)                                                  \
         ? (((sp)->norm >> (sig)) & 0x01)                               \
         : (((sp)->rt >> (sig - SIGRTMIN)) & 0x01))))
#if (_GNU_SOURCE)
#define sigisemptyset(sp) (!(sp)->norm | !(sp)->rt)
#endif

#else /* !SIG32BIT */

/* POSIX */
#define sigemptyset(sp)                                                 \
    (!*(sp))
#define sigfillset(sp)                                                  \
    (*(sp) = ~0L)
#define sigaddset(sp, sig)                                              \
    ((!_sigvalid(sig)                                                   \
      ? (-1L)                                                           \
      : ((sp) |= (1UL << (sig)),                                        \
         0)))
#define sigdelset(sp, sig)                                              \
    ((!_sigvalid(sig)                                                   \
      ? (-1L)                                                           \
      : ((sp) &= ~(1UL << (sig)),                                       \
         0)))
#define sigismember(sp, sig)                                            \
    ((!_sigvalid(sig)                                                   \
      ? (-1)                                                            \
      : (*(sp) & (1UL << (sig)),                                        \
         0)))
#if (_GNU_SOURCE)
#define sigisemptyset(sp) (!(sp))
#endif

#endif /* SIG32BIT */

#endif /* _POSIX_SOURCE */

#if (_POSIX_SOURCE)

#define MAXSIG      SIGRTMAX

#define S_SIGNAL    1
#define S_SIGSET    2
#define S_SIGACTION 3
#define S_NONE      4

#endif /* _POSIX_SOURCE */

#if (_BSD_SOURCE)
#define BADSIG      SIG_ERR
#endif

#define MAXSIG      SIGRTMAX

#define S_SIGNAL    1
#define S_SIGSET    2
#define S_SIGACTION 3
#define S_NONE      4

#endif /* __BITS_SIGNAL_H__ */

