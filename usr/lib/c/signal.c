#include <features.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <sys/zero/syscall.h>

/* NOTE: SIG_DFL is 0 */
THREADLOCAL    _sigfunctab[_NSIG] ALIGNED(CLSIZE) = { 0 };
const uint64_t _sigforcebits = _SIGFORCEBITS;

#define _sigforced(sig) (_sigforcebits & (UINT64_C(1) << (sig)))

#if (USEBSD) || (USEGNU)
#undef SYSV_SIGNAL
#else /* UNIX / System V semantics; reset to SIG_DFL, do not block */
#define SYSV_SIGNAL
#endif

int
sigaction(int sig, const struct sigaction *act, struct sigaction *oldact)
{
    
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

    sa.sa_handler = func;
    if (sigemptyset(&sa.sa_mask) < 0
        || sigaction(sig, &sa, &oldsa) < 0) {

        return SIG_ERR;
    }
    oldfunc = oldsa.sa_handler;

    return oldfunc;
}

__sighandler_t
__sysv_signal(int sig, __sighandler_t func)
{
    struct sigaction sa;
    struct sigaction oldsa;
    __sighandler_t   oldfunc;

    if (!_sigvalid(sig)) {
        errno = EINVAL;

        return SIG_ERR;
    }
    sa.sa_handler = func;
    if (sigemptyset(&sa.sa_mask) < 0) {

        return SIG_ERR;
    }
    sa.sa_flags = SA_ONESHOT | SA_NOMASK | SA_INTERRUPT;
    if (sigaction(sig, &sa, &oldsa) < 0) {

        return SIG_ERR;
    }
    oldfunc = oldsa.sa_handler;

    return oldfunc;
}

#if (USEBSD)

__sighandler_t
bsd_signal(int sig, __sighandler_t func)
{
    struct sigaction sa;
    struct sigaction oldsa;
    sigset_t         set;
    __sighandler_t   oldfunc;

    if (!_sigvalid(sig)) {

        return (__sighandler_t)SIG_IGN;
    }
    if (sigemptyset(&set) < 0) {

        return (__sighandler_t)SIG_IGN;
    }
    sa.sa_handler = func;
    sa.sa_mask = set;
    sa.sa_flags = 0;
    if (sigaction(sig, &sa, &oldsa) < 0) {

        return (__sighandler_t)SIG_IGN;
    }
    oldfunc = oldsa.sa_handler;

    return oldfunc;
}

#endif

