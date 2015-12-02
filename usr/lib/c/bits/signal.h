#ifndef __BITS_SIGNAL_H__
#define __BITS_SIGNAL_H__

#undef PTHREAD
#if !defined(PTHREAD) || defined(__KERNEL__)
#define PTHREAD 0
#endif

#include <features.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
//#include <zero/param.h>
#include <zero/types.h>
#if (PTHREAD)
#include <pthread.h>
#endif
#if (_ZERO_SOURCE) && 0
#include <kern/signal.h>
#endif
#if defined(__x86_64__) || defined(__amd64__)
#include <x86-64/signal.h>
#elif (defined(__i386__) || defined(__i486__)                           \
       || defined(__i586__) || defined(__i686__))
#include <ia32/signal.h>
#elif defined(__arm__)
#include <arm/signal.h>
#endif

/* internal. */
#define _sigvalid(sig) ((sig) && (!((sig) & ~_SIGMASK)))
#define _sigrt(sig)    ((sig) && ((sig) & _SIGRTBIT))
#define _signorm(sig)  ((sig) && (!((sig) & _SIGRTBIT)))
#define _SIGNOCATCH    ((UINT64_C(1) << SIGKILL) | (UINT64_C(1) << SIGSTOP))

#if (defined(__x86_64__) || defined(__amd64__) || defined(___alpha__)   \
     || defined(__i386__) || defined(__i486__)                          \
     || defined(__i586__) || defined(__i686__))
#define SIG32BIT 0
#else
#define SIG32BIT 1
#endif

/* special values; standard ones */
#define SIG_ERR      (__sighandler_t)-1)
#define SIG_DFL      (__sighandler_t)0)
#define SIG_IGN      (__sighandler_t)1)
#define SIG_HOLD     (__sighandler_t)2)
/* private values for signal actions */
#define _SIG_TERM    (__sighandler_t)3)
#define _SIG_CORE    (__sighandler_t)4)
#define _SIG_STOP    (__sighandler_t)5)
#define _SIG_CONT    (__sighandler_t)6)

/* commands for sigprocmask() */
#define SIG_BLOCK    0
#define SIG_UNBLOCK  1
#define SIG_SETMASK  2

#define SIGNO_MASK   0x000000ff
#define SIGDEFER     0x00000100
#define SIGHOLD      0x00000200
#define SIGRELSE     0x00000400
#define SIGIGNORE    0x00000800
#define SIGPAUSE     0x00001000

#if (SIG32BIT)
typedef struct {
    int32_t norm;
    int32_t rt;
} sigset_t;
#else
typedef int64_t sigset_t;
#endif
#if !defined(__pid_t_defined)
typedef long             pid_t;          // process ID
#define __pid_t_defined
#endif

typedef void           (*__sighandler_t)(int);

#if (USEBSD) && (!USEPOSIX)
typedef void __bsdsig_t(int sig, int code, struct sigcontext *ctx, char *adr);
#endif
#if (_BSD_SOURCE) || (_GNU_SOURCE)
typedef __sighandler_t sig_t;
#endif
#if (_GNU_SOURCE)
typedef __sighandler_t sighandler_t;
#endif

/* sigaction() definitions */ 
#define SA_NOCLDSTOP SIG_NOCLDSTOP
#define SA_NOCLDWAIT SIG_NOCLDWAIT
#define SA_NODEFER   SIG_NODEFER
#define SA_NOMASK    SA_NODEFER
#define SA_RESETHAND SIG_RESETHAND
#define SA_ONESHOT   SA_RESETHAND
#define SA_SIGINFO   SIG_SIGINFO
/* non-POSIX */
#if (!USEPOSIX)
#define SA_ONSTACK   SIG_ONSTACK
#define SA_RESTART   SIG_RESTART
#define SA_INTERRUPT SIG_FASTINTR
#endif

#if (!_ZERO_SOURCE) && (USEPOSIX)
struct sigaction {
    void     (*sa_handler)(int);
    void     (*sa_sigaction)(int, siginfo_t *, void *); // sa_flags & SA_SIGINFO
    void     (*sa_restorer)(void);
    sigset_t  sa_mask;
    int       sa_flags;
};
#endif

union sigval {
    void *sival_ptr;
    int   sival_int;
};

/* values for sigev_notify */
#define SIGEV_NONE   0
#define SIGEV_SIGNAL 1
#define SIGEV_THREAD 2
struct sigevent {
    int              sigev_notify;
    int              sigev_signo;
    union sigval     sigev_value;
    void           (*sigev_notify_function)(union sigval);
#if (PTHREAD)
    pthread_attr_t  *sigev_notify_attributes;
#endif
};

#if (_POSIX_SOURCE) && (USEPOSIX199309)

/* si_code-member values */
/* SIGILL */
#define ILL_ILLOPC    1 // illegal opcode
#define ILL_ILLOPN    2 // illegal operand
#define ILL_ILLADR    3 // illegal addressing mode
#define ILL_ILLTRP    4 // illegal trap
#define ILL_PRVOPC    5 // privileged opcode
#define ILL_PRVREG    6 // privileged register
#define ILL_COPROC    7 // coprocessor error
#define ILL_BADSTK    8 // internal stack error
/* SIGFPE */
#define FPE_INTDIV    1 // integer division by zero
#define FPE_INTOVF    2 // integer overflow
#define FPE_FLTDIV    3 // floating-point division by zero
#define FPE_FLTOVF    4 // floating-point overflow
#define FPE_FLTUND    5 // floating-point underflow
#define FPE_FLTRES    6 // floating-point inexact result
#define FPE_FLTINV    7 // invalid floating-point operation
#define FPE_FLTSUB    8 // subscript out of range
/* SIGSEGV */
#define SEGV_MAPERR   1 // address not mapped to object
#define SEGV_ACCERR   2 // invalid permissions for mapped object
/* SIGBUS */
#define BUS_ADRALN    1 // invalid address alignment
#define BUS_ADRERR    2 // nonexistent physical address
#define BUS_OBJERR    3 // object-specific hardware error
/* SIGTRAP */
#define TRAP_BRKPT    1 // process breakpoint
#define TRAP_TRACE    2 // process trace trap
/* SIGCHLD */
#define CLD_EXITED    1 // child has exited
#define CLD_KILLED    2 // terminated abnormally, didn't dump core
#define CLD_DUMPED    3 // terminated abnormally, dumped core
#define CLD_TRAPPED   4 // traced child has trapped
#define CLD_STOPPED   5 // child has stopped
#define CLD_CONTINUED 6 // stopped child has continued
/* SIGPOLL */
#define POLL_IN       1 // data input available
#define POLL_OUT      2 // output buffers available
#define POLL_MSG      3 // input message available
#define POLL_ERR      4 // I/O error
#define POLL_PRI      5 // high priority input available
#define POLL_HUP      6 // device disconnected
/* any signal */
#define SI_USER       1 // signal sent by kill()
#define SI_QUEUE      2 // signal sent by sigqueue()
#define SI_TIMER      3 // timer set by timer_settime() expired
#define SI_ASYNCIO    4 // asynchronous I/O request completed
#define SI_MESGQ      5 // message arrived on empty message queue
typedef struct {
    int           si_signo;
    int           si_code;
    int           si_errno;
    int           si_status;    // exit value or signal
    pid_t         si_pid;       // SIGCHLD
    uid_t         si_uid;       // real user ID of signal sender process
    void         *si_addr;      // SIGILL, SIGFPE, SIGSEGV, SIGBUS
    long          si_band;      // band event for POLL_IN, POLL_OUT, POLL_MSG
    union sigval  si_value;
    ctid_t        si_ctid;
    zoneid_t      si_zoneid;
} siginfo_t;

#endif /* _POSIX_SOURCE && USEPOSIX199309 */

#if (_BSD_SOURCE) || (USEXOPENEXT)

struct sigstack {
    char *ss_sp;
    int   ss_onstack;
};

#define SIGSTKSZ     (4 * PAGESIZE)
#define MINSIGSTKSZ  PAGESIZE
/* ss_flags bits */
#define SS_ONSTACK   0x00000001
#define SS_DISABLE   0x00000002
typedef struct {
  void   *ss_sp;
  size_t  ss_size;
  int     ss_flags;
} stack_t;

#elif (FAVORBSD)

struct sigaltstack {
    void *ss_base;
    int   ss_len;
    int   ss_onstack;
};

#elif (!USEPOSIX)

struct sigaltstack {
    void   *ss_sp;
    size_t  ss_size;
    int     ss_flags;
};

#endif

#if (USEBSD) && (!USEPOSIX)

/* bits for sv_flags */
#define SV_INTERRUPT SIG_RESTART        // opposite sense
#define SV_RESETHAND SIG_RESETHAND
#define SV_ONSTACK   SIG_ONSTACK
#define sv_onstack   sv_flags           // compatibility name
struct sigvec {
    void (*sv_handler)(int);            // signal disposition
    long   sv_mask;                     // signals to block while executing
    int    sv_flags;
};

#endif /* USEBSD && !USEPOSIX */

#if (_POSIX_SOURCE)

#if (SIG32BIT)

//#define _sigptr(sp) ((struct sigset *)(sp))

/* POSIX */
#define sigemptyset(sp)                                                 \
    ((sp)->norm = (sp)->rt = 0)
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
#define __sigisemptyset(sp) (!(sp)->norm | !(sp)->rt)

#else /* !SIG32BIT */

/* POSIX */
#define sigemptyset(sp)                                                 \
    (*(sp) = 0)
#define sigfillset(sp)                                                  \
    (*(sp) = ~0L)
#define sigaddset(sp, sig)                                              \
    ((!_sigvalid(sig)                                                   \
      ? (errno = EINVAL, -1L)                                           \
      : ((sp) |= (1UL << (sig)),                                        \
         0)))
#define sigdelset(sp, sig)                                              \
    ((!_sigvalid(sig)                                                   \
      ? (errno = EINVAL, -1L)                                           \
      : ((sp) &= ~(1UL << (sig)),                                       \
         0)))
#define sigismember(sp, sig)                                            \
    ((!_sigvalid(sig)                                                   \
      ? (errno = EINVAL, -1)                                            \
      : ((*(sp) & (1UL << (sig)))                                       \
         ? 1                                                            \
         : 0)))
#if (_GNU_SOURCE)
#define __sigisemptyset(sp) (!*(sp))
#endif

#endif /* SIG32BIT */

#endif /* _POSIX_SOURCE */

#if (_GNU_SOURCE)
#define sigisemptyset(sp) __sigisemptyset(sp)
#endif

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

