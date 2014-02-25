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

signalhandler_t *ksigfunctab[NSIG];

void
kill(struct proc *proc)
{
    ;
}

ASMLINK
void
sigfunc(uint32_t trap)
{
    struct proc     *proc = k_curproc;
    long             signum = signumtab[trap];
    signalhandler_t *func;

//    kprintf("trap 0x%lx -> signal 0x%lx\n", trap, signum);
    if (trap == TRAPUD) {
//        kprintf("PANIC: #UD (0x%lx)\n", errcode);
    } else if (trap == TRAPGP) {
//        kprintf("PANIC: #GP (0x%lx)\n", errcode);
    } else if (signum == SIGKILL) {
//        kprintf("trap 0x%lx -> signal 0x%lx\n", trap, signum);
        kill(proc);
    } else if ((signum) && sigismember(&proc->sigmask, signum)) {
//        kprintf("trap 0x%lx -> signal 0x%lx\n", trap, signum);
        func = proc->sigvec[signum];
        if (func) {
            func(signum);
        } else {
            func = ksigfunctab[signum];
            if (func) {
                func(signum);
            }
        }
    }

    return;
}

