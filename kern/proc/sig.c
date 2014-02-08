#include <stdint.h>
#define __KERNEL__ 1
#include <signal.h>
#include <zero/types.h>
#include <kern/util.h>
#include <kern/obj.h>
//#include <kern/proc.h>
#if !defined(__arm__)
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/cpu.h>
#endif

/* cpu traps */
long signumtab[TRAPNCPU]
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
    struct proc     *proc = k_curproc;
    long             signum = signumtab[trap];
    signalhandler_t *func;

//    kprintf("trap %ld -> signal %ld\n", trap, signum);
    if (signum == SIGKILL) {
        kill(proc);
    } else if ((signum) && sigismember(&proc->sigmask, signum)) {
        func = proc->sigvec[signum];
        if (func) {
            func(signum);
        } else {
            func = sigfunctab[signum];
            if (func) {
                func(signum);
            }
        }
    }

    return;
}

