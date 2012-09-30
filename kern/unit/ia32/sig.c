#include <stdint.h>
#define __KERNEL__ 1
#include <kern/signal.h>
#include <zero/types.h>
#include <kern/util.h>
#include <kern/proc.h>
#include <kern/unit/ia32/cpu.h>

/* cpu traps */
long signumtab[]
= {
    SIGFPE, 
    0,
    0,
    SIGTRAP,
    0,
    SIGBUS,
    SIGILL,
    SIGILL,
    0,
    0,
    0,
    SIGSEGV,
    SIGSTKFLT,
    SIGSEGV,
    0,
    0,
    SIGFPE,
    SIGBUS,
    SIGABRT,
    SIGFPE
};

signalhandler_t *sigfunctab[NSIG];

void
kill(struct proc *proc)
{
    ;
}

void
sigfunc(uint32_t trap)
{
    struct proc     *proc = curproc;
    long             signum = signumtab[trap];
    signalhandler_t *sigfunc;

//    kprintf("trap %ld -> signal %ld\n", trap, signum);
    if (signum == SIGKILL) {
        kill(proc);
    }
    if ((signum) && sigismember(&proc->sigmask, signum)) {
        sigfunc = proc->sigvec[signum];
        if (sigfunc) {
            sigfunc(signum);
        } else {
            sigfunc = sigfunctab[signum];
            if (sigfunc) {
                sigfunc(signum);
            }
        }
    }
}

