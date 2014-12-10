#ifndef __SIGNAL_H__
#define __SIGNAL_H__

/* TODO: sysconf(): _SC_SIGRT_MIN, _SC_SIGRT_MAX */

#if !defined(PTHREAD) || defined(__KERNEL__)
#define PTHREAD 0
#endif

#include <features.h>
#include <sys/types.h>
#include <zero/param.h>
#include <unistd.h>
#include <zero/param.h>
#if (_ZERO_SOURCE)
#include <kern/signal.h>
#endif
#if (PTHREAD)
#include <pthread.h>
#endif
#include <bits/signal.h>

#if (_POSIX_SOURCE) && (USEPOSIX199309)
#include <time.h>
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
#endif /* _POSIX_SOURCE */

#if (_BSD_SOURCE) || (USEXOPENEXT)
/*
 * send signal sig to all processes in the group pgrp
 * - if pid is zero, send sig to all processes in the current one's group
 */
extern int killpg(pid_t pgrp, int sig);
#endif /* _BSD_SOURCE || USEXOPENEXT */

extern int raise(int sig);
#if (USESVID)
extern __sighandler_t ssignal(int sig, __sighandler_t func);
extern int            gsignal(int sig);
#endif /* USESVID */
extern void           psignal(int sig);
#if (FAVORBSD)
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

//typedef __sighandler_t sig_t;
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
extern int sigaction(int sig, const struct sigaction *__restrict act,
                     struct sigaction *__restrict oldact);
extern int sigpending(sigset_t *set);
extern int sigwait(const sigset_t *set, int *__restrict sig);
#if (USEPOSIX199309)
extern int sigwaitinfo(const sigset_t *__restrict set,
                       siginfo_t *__restrict info);
extern int sigtimedwait(const sigset_t *__restrict set,
                        siginfo_t *__restrict info,
                        const struct timespec *__restrict timeout);
//extern int sigqueue(pid_t pid, int sig, const union sigval val);
#endif /* USEPOSIX199309 */

#endif /* _POSIX_SOURCE */

#if (_BSD_SOURCE)
extern const char *__const _sys_siglist[_NSIG];
extern const char *__const sys_siglist[_NSIG];
#endif

#if (_BSD_SOURCE) || (USEXOPENEXT)

struct sigstack {
    char *ss_sp;
    int   ss_onstack;
};

typedef struct {
  char *ss_sp;
  int   ss_size;
  int   ss_flags;
} stack_t;

#else /* !(_BSD_SOURCE | USEXOPENEXT) */

struct sigaltstack {
    char *ss_base;
    int   ss_len;
    int   ss_onstack;
};

#endif /* _BSD_SOURCE || USEXOPENEXT */

/*
 * if intr is nonzero, make signal sig interrupt system calls (causing them
 * to fail with EINTR); if intr is zero, make system calls be restarted
 * after signal sig
 */
extern int siginterrupt(int sig, int intr);

#if (_BSD_SOURCE) || (USEXOPENEXT)
extern int sigstack(struct sigstack *stk, struct sigstack *oldstk);
extern int sigaltstack(const stack_t *stk, const stack_t *oldstk)
#else
extern int sigaltstack(const struct sigaltstack *stk,
                       struct sigaltstack *oldstk);
#endif

#if (_XOPEN_SOURCE)
#include <ucontext.h>
#endif

#if (USEXOPENEXT)
extern int            sighold(int sig);
extern int            sigrelse(int sig);
extern int            sigignore(int sig);
extern __sighandler_t sigset(int sig, __sighandler_t func);
#endif /* USEXOPENEXT */

//int sigprocmask(int how, const sigset_t *set, sigset_t *oset);

/* macros. */

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


#define MAXSIG      SIGRTMAX

#define S_SIGNAL    1
#define S_SIGSET    2
#define S_SIGACTION 3
#define S_NONE      4


#if (_POSIX_SOURCE) && (USEPOSIX199309)
typedef struct {
    int           si_signo;
    int           si_code;
    int           si_errno;
    pid_t         si_pid;
    uid_t         si_uid;
    void         *si_addr;
    int           si_status;
    long          si_band;
    union sigval  si_value;
} siginfo_t;
#endif /* _POSIX_SOURCE && USEPOSIX199309 */

#endif /* __SIGNAL_H__ */

