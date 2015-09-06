#include <signal.h>
#include <sys/zero/syscall.h>

void *
signal(int sig, void (*func)(int))
{
    void             *ret;
    struct syssigarg  arg;

    arg.pid = SIG_SELF;
    arg.func = func;
    ret = (void *)_syscall(SYS_SIGOP, SIG_SETFUNC, sig, (sysreg_t)&arg);

    return ret;
}

