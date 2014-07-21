#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#include <features.h>
#if (__KERNEL__)
#include <kern/signal.h>
#endif

#if 0
#if (_ZERO_SOURCE)
/* Zero kernel-user and user-user signaling */
#define SIGHID SIGRTMIN
#define SIGAUD (SIGRTMIN + 1)
#define SIGVID (SIGRTMIN + 2)
#define sigishid(sp)    sigismember(sp, SIGHID)
#define sigisaud(sp)    sigismember(sp, SIGAUD)
#define sigisvid(sp)    sigismember(sp, SIGVID)
#endif
#endif

typedef void signalhandler_t(int);
/* internal. */
#define _sigvalid(sig)  ((sig) && (!((sig) & ~SIGMASK)))
#define _sigrt(sig)     ((sig) && (!((sig) & ~SIGRTMASK)))
#define _signorm(sig)   ((sig) && (!((sig) & SIGRTMASK)))

#if (_POSIX_SOURCE)

#if defined(__i386__) || defined(__arm__)
#define SIG32BIT 1
#else
#define SIG32BIT 0
#endif

typedef volatile long sig_atomic_t;
#if (!SIG32BIT)
typedef uint64_t sigset_t;
#elif (LONGSIZE == 4)
typedef struct sigset { uint32_t norm; uint32_t rt; } sigset_t;
#elif (LONGSIZE == 8)
typedef struct sigset { uint64_t mask; } sigset_t;
#endif
typedef void     sighandler_t(int);

//int sigprocmask(int how, const sigset_t *set, sigset_t *oset);

/* macros. */

#if (SIG32BIT)

#define _sigptr(sp)     ((struct sigset *)(sp))

/* POSIX. */

#if (LONGSIZE == 4)

#define sigemptyset(sp)                                                 \
    (!_sigptr(sp)->norm && !_sigptr(sp)->rt)
#define sigfillset(sp)                                                  \
    ((_sigptr(sp)->norm = _sigptr(sp)->rt = ~UINT32_C(0)), 0)

#define sigaddset(sp, sig)                                              \
    ((!_sigvalid(sig)                                                   \
      ? (-1)                                                            \
      : (_signorm(sig)                                                  \
         ? (_sigptr(sp)->norm |= (1UL << (sig)))                        \
         : (_sigptr(sp)->rt |= (1UL << ((sig) - SIGRTMIN))))),          \
     0)
#define sigdelset(sp, sig)                                              \
    ((!_sigvalid(sig)                                                   \
      ? (-1)                                                            \
      : (_signorm(sig)                                                  \
         ? (_sigptr(sp)->norm &= ~(1UL << (sig)))                       \
         : (_sigptr(sp)->rt &= ~(1UL << ((sig) - SIGRTMIN))))),         \
     0)
#define sigismember(sp, sig)                                            \
    ((!_sigvalid(sig)                                                   \
      ? (-1)                                                            \
      : (_signorm(sig)                                                  \
         ? ((_sigptr(sp)->norm >> (sig)) & 0x01)                        \
         : ((_sigptr(sp)->rt >> (sig - SIGRTMIN)) & 0x01))))

#elif (LONGSIZE == 8)

#define sigemptyset(sp) (!_sigptr(sp)->mask)
#define sigfillset(sp)  ((_sigptr(sp)->mask = ~UINT64_C(0)), 0)

#define sigaddset(sp, sig)                                              \
    ((!_sigvalid(sig)                                                   \
      ? (-1)                                                            \
      : (_sigptr(sp)->mask |= (1UL << ((sig))))),                       \
     0)
#define sigdelset(sp, sig)                                              \
    ((!_sigvalid(sig)                                                   \
      ? (-1)                                                            \
      : (_sigptr(sp)->mask &= ~(1UL << ((sig))))),                      \
     0)
#define sigismember(sp, sig)                                            \
    (!_sigvalid(sig)                                                    \
     ? (-1)                                                             \
     : (_sigptr(sp)->mask & (1UL << ((sig)))))

#endif /* LONGSIZE == 8 */

#else /* !SIG32BIT */

/* POSIX. */
#define sigemptyset(sp) (*(s) = 0)
#define sigfillset(sp)  (*(sp) = ~UINT64_C(0), 0)

#define sigaddset(sp, sig)                                              \
    (!_sigvalid(sig)                                                    \
     ? (-1)                                                             \
     : ((*(sp) |= (UINT64_C(0x01) << (sig))), 0))
#define sigdelset(sp, sig)                                              \
    (!_sigvalid(sig)                                                    \
     ? (-1)                                                             \
     : ((*(sp) &= ~(UINT64_C(0x01) << (sig))), 0))
#define sigismember(sp, sig)                                            \
    (!_sigvalid(sig)                                                    \
     ? (-1)                                                             \
     : ((*(sp) >> (sig)) & UINT64_C(0x01)))

#endif /* LONGSIZE */

#define SA_NOCLDSTOP SIG_NOCLDSTOP
#define SA_NOCLDWAIT SIG_NOCLDWAIT
#define SA_NODEFER   SIG_NODEFER
#define SA_RESETHAND SIG_RESETHAND
#define SA_SIGINFO   SIG_SIGINFO
/* non-posix */
#define SA_ONSTACK   SIG_ONSTACK
#define SA_RESTART   SIG_RESTART

#define SIGSTKSZ     NBPG
#define SS_DISABLE   (1 << 0)
#define SS_ONSTACK   (1 << 1)

#endif /* _POSIX_SOURCE */

/* special values. */
#define SIG_ERR      ((sighandler_t)-1L)
#define SIG_IGN      ((sighandler_t)0L)
#define SIG_DFL      ((sighandler_t)1L)
#define SIG_HOLD     ((sighandler_t)2L)

#if (_BSD_SOURCE)

struct sigvec {
    void (*sv_handler)(int);
    int    sv_mask;
    int    sv_flags;
};

int sigvec(int sig, struct sigvec *vec, struct sigvec *oldvec);
int sigmask(int sig);
int sigblock(int mask);
int sigsetmask(int mask);
int siggetmask(void);

#define SV_INTERRUPT 0x00000001
#define SV_RESETHAND 0x00000002
#define SV_ONSTACK   0x00000004

#endif /* _BSD_SOURCE */

#endif /* __SIGNAL_H__ */

