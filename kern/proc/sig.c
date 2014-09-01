#include <stdint.h>
#define __KERNEL__ 1
#include <signal.h>
#include <zero/types.h>
#include <kern/util.h>
//#include <kern/obj.h>
#include <kern/proc/proc.h>
#if !defined(__arm__)
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/cpu.h>
#endif

extern long      trapsigmap[TRAPNCPU];

signalhandler_t *ksigfunctab[NSIG];

void
kill(struct proc *proc)
{
    ;
}

#if (FASTINTR)
FASTCALL
#endif
void
sigfunc(unsigned long pid, uint32_t trap, long errnum)
{
    struct proc     *proc = k_curproc;
    long             signum = trapsigmap[trap];
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

