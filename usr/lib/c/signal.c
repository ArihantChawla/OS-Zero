#include <features.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#if (_ZERO_SOURCE)
#include <kern/conf.h>
#include <sys/zero/syscall.h>
#endif

/* NOTE: SIG_DFL is 0 */
static THREADLOCAL sigset_t sigmasktab[NPROCTASK] ALIGNED(PAGESIZE);
static __sighandler_t       sigfunctab[_NSIG] = { 0 };
static const uint64_t       signocatchbits = _SIGNOCATCHBITS;

#define _signocatch(sig) (signocatchbits & (UINT64_C(1) << (sig)))

#if (USEBSD) || (USEGNU)
#undef SYSV_SIGNAL
#else /* UNIX / System V semantics; reset to SIG_DFL, do not block */
#define SYSV_SIGNAL
#endif

int
sigaction(int sig, const struct sigaction *act, struct sigaction *oldact)
{
    if (!_sigvalid(sig)
        || sig == SIGSTOP
        || sig == SIGKILL) {
        errno = EINVAL;

        return -1;
    }
    /* FIXME: finish this function */
}

__sighandler_t
signal(int sig, __sighandler_t func)
{
    struct sigaction sa;
    struct sigaction oldsa;
    __sighandler_t   oldfunc;
#if defined(SYSV_SIGNAL)
    int              flg = SA_RESETHAND | SA_NODEFER;
#else /* BSD semantics */
    int              flg = SA_RESTART;
#endif

    if (!_sigvalid(sig)
        || sig == SIGSTOP
        || sig == SIGKILL) {
        errno = EINVAL;

        return SIG_ERR;
    }
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, sig);
    sa.sa_handler = func;
    sa.sa_flags = flg;
    if (sigaction(sig, &sa, &oldsa) < 0) {

        return SIG_ERR;
    }
    oldfunc = oldsa.sa_handler;

    return oldfunc;
}

#if defined(_GNU_SOURCE)

/*
 * unreliable semantics
 * --------------------
 * - disposition of sig is reset to the default
 * - delivery of further instances of sig is not blocked
 * - blocking system calls are not restarted
 */
sighandler_t
sysv_signal(int sig, __sighandler_t func)
{
    struct sigaction sa;
    struct sigaction oldsa;
    __sighandler_t   oldfunc;
    int              flg = SA_ONESHOT | SA_NOMASK | SA_INTERRUPT;

    if (!_sigvalid(sig)
        || sig == SIGSTOP
        || sig == SIGKILL) {
        errno = EINVAL;

        return SIG_ERR;
    }
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, sig);
    sa.sa_handler = func;
    sa.sa_flags = flg;
    if (sigaction(sig, &sa, &oldsa) < 0) {

        return SIG_ERR;
    }
    oldfunc = oldsa.sa_handler;

    return oldfunc;
}

#endif /* defined(_GNU_SOURCE) */

#if defined(_XOPEN_SOURCE) || (USEBSD)

/*
 * reliable semantics
 * ------------------
 * - disposition of sig is not reset to the default
 * - delivery of further instances of sig is blocked
 * - blocking system calls are restarted
 */
__sighandler_t
bsd_signal(int sig, __sighandler_t func)
{
    struct sigaction sa;
    struct sigaction oldsa;
    __sighandler_t   oldfunc;

    if (!_sigvalid(sig)
        || sig == SIGSTOP
        || sig == SIGKILL) {
        errno = EINVAL;

        return SIG_ERR;
    }
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, sig);
    sa.sa_handler = func;
    sa.sa_flags = 0;
    if (sigaction(sig, &sa, &oldsa) < 0) {

        return SIG_ERR;
    }
    oldfunc = oldsa.sa_handler;

    return oldfunc;
}

#endif /* defined(_XOPEN_SOURCE) || (USEBSD) */

