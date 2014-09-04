#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#include <features.h>
#include <sys/types.h>
#include <zero/param.h>
#if (_ZERO_SOURCE)
/* kernel signal interface */
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

#if (_POSIX_SOURCE)

typedef volatile long   sig_atomic_t;
#if (SIG32BIT)
struct _sigset {
    uint32_t norm;
    uint32_t rt;
};
typedef struct _sigset   sigset_t;
#else
typedef long             sigset_t;
#endif
typedef void           (*__sighandler_t)(int);
typedef __sighandler_t  sighandler_t;
#if (_GNU_SOURCE)
typedef void            signalhandler_t(int);
#endif

#endif /* _POSIX_SOURCE */

#if (_POSIX_SOURCE) && defined(USEPOSIX199309)
//#include <time.h>
#endif

#if (_BSD_SOURCE)
/* set handler for signal sig; returns old handler */
extern __sighandler_t signal(int sig, __sighandler_t func);
#endif
extern __sighandler_t __sysv_signal(int sig, __sighandler_t func);
#if (_GNU_SOURCE)
extern __sighandler_t sysv_signal(int sig, __sighandler_t func);
#endif
#if (_XOPEN_SOURCE)
extern __sighandler_t bsd_signal(int sig, __sighandler_t func);
#endif
#if (_POSIX_SOURCE)
/*
 * send signal sig to process or group described by pid
 * - if pid is zero, send sig to all processes in the current one's process group
 * - if pid < -1, send sig to all prccesses in the process group -pid
 */
extern int kill(pid_t pid, int sig);
#endif
#if (_BSD_SOURCE) || (USEXOPENEXT)
/*
 * send signal sig to all processes in the group pgrp
 * - if pid is zero, send sig to all processes in the current one's group
 */
extern int killpg(pid_t pgrp, int sig);
#endif

extern int raise(int sig);
#if (USESVID)
extern __sighandler_t ssignal(int sig, __sighandler_t func);
extern int            gsignal(int sig);
#endif
extern void           psignal(int sig);
#if (USEOLDBSD)
/* set mask to blocked signals, wait for signal, restore the mask */
extern int            sigpause(int mask);
#elif (_XOPEN_SOURCE)
/* remove sig from signal mask and suspend the process */
#if defined(__GNUC__)
extern int            sigpause(int sig) __asm__ ("__xpg_sigpause\n");
#else
//#define               sigpause(sig) __sigpause(sig, 1);
#endif
#endif

#if (_BSD_SOURCE)

typedef __sighandler_t sig_t;
/* none of these functions should be used any longer */
#define sigmask(sig)   (1L << (sig))
/* block signals in mask, return old mask */
extern int             sigblock(int mask); 
/* set mask of blocked signals, return old mask */
extern int             sigsetmask(int mask);
/* return current signal mask */
extern int             siggetmask(void);

struct sigvec {
    void (*sv_handler)(int);
    int    sv_mask;
    int    sv_flags;
};

int sigvec(int sig, const struct sigvec *vec, struct sigvec *oldvec);
#if 0
int sigmask(int sig);
int sigblock(int mask);
int sigsetmask(int mask);
int siggetmask(void);
#endif

#define SV_INTERRUPT 0x00000001
#define SV_RESETHAND 0x00000002
#define SV_ONSTACK   0x00000004

// extern int sigreturn(struct sigcontext *scp);

#endif /* BSD_SOURCE */

#if (_POSIX_SOURCE)
/* get and/or change set of blocked signals */
extern int sigprocmask(int how, const sigset_t *__restrict set,
                       sigset_t *__restrict oldset);
/* change blocked signals to set, wait for a signal, restore the set */
extern int sigsuspend(const sigset_t *set);
#if 0 /* TODO: struct sigaction */
extern int sigaction(int sig, const struct sigaction *__restrict act,
                     struct sigaction *__restrict oldact);
#endif
extern int sigpending(sigset_t *set);
extern int sigwait(const sigset_t *set, int *__restrict sig);
#if (USEPOSIX199309) && 0 /* TODO: siginfo_t */
extern int sigwaitinfo(const sigset_t *__restrict set,
                       siginfo_t *__restrict info);
extern int sigtimedwait(const sigset_t *__restrict set,
                        siginfo_t *__restrict info,
                        const struct timespec *__restrict timeout);
//extern int sigqueue(pid_t pid, int sig, const union sigval val);
#endif
#endif

#if (_BSD_SOURCE)
extern const char *__const _sys_siglist[_NSIG];
extern const char *__const sys_siglist[_NSIG];
#endif

#if (_BSD_SOURCE) || (USEXOPENEXT)

/*
 * if intr is nonzero, make signal sig interrupt system calls (causing them
 * to fail with EINTR); if intr is zero, make system calls be restarted
 * after signal sig
 */
extern int siginterrupt(int sig, int intr);

// extern int sigstack(struct sigstack *stk, struct sigstack *oldstk);
// extern int sigaltstack(const struct sigaltstack *__restrict stk,
//                        struct sigaltstack *__restrict oldstk);

#if (_XOPEN_SOURCE) && 0
#include <sys/ucontext.h>
#endif

#endif /* _BSD_SOURCE || USEXOPENEXT */

#if (USEXOPENEXT)
extern int            sighold(int sig);
extern int            sigrelse(int sig);
extern int            sigignore(int sig);
extern __sighandler_t sigset(int sig, __sighandler_t func);
#endif /* USEXOPENEXT */

//int sigprocmask(int how, const sigset_t *set, sigset_t *oset);

/* macros. */

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
#define sigisemptyset(sp) ((sp)->norm | (sp)->rt)
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
      : ((sp)->norm |= (1UL << (sig)),                                  \
         0)))
#define sigdelsetset(sp, sig)                                           \
    ((!_sigvalid(sig)                                                   \
      ? (-1L)                                                           \
      : ((sp)->norm &= ~(1UL << (sig)),                                 \
         0)))
#define sigismember(sp, sig)                                            \
    ((!_sigvalid(sig)                                                   \
      ? (-1)                                                            \
      : (*(sp) & (1L << (sig)),                                         \
         0)))
#if (_GNU_SOURCE)
#define sigisemptyset(sp) (sp)
#endif

#endif /* SIG32BIT */

#endif /* _POSIX_SOURCE */

/* sigaction() definitions */ 
#define SA_NOCLDSTOP SIG_NOCLDSTOP
#define SA_NOCLDWAIT SIG_NOCLDWAIT
#define SA_NODEFER   SIG_NODEFER
#define SA_RESETHAND SIG_RESETHAND
#define SA_SIGINFO   SIG_SIGINFO
/* non-POSIX */
#define SA_ONSTACK   SIG_ONSTACK
#define SA_RESTART   SIG_RESTART

#define SIGSTKSZ     NBPG
#define SS_DISABLE   (1 << 0)
#define SS_ONSTACK   (1 << 1)

/* special values. */
#define SIG_ERR      ((sighandler_t)-1L)
#define SIG_IGN      ((sighandler_t)0L)
#define SIG_DFL      ((sighandler_t)1L)
#define SIG_HOLD     ((sighandler_t)2L)

#endif /* __SIGNAL_H__ */

