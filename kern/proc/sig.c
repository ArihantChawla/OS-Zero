#include <kern/conf.h>
#include <stdint.h>
#define __KERNEL__ 1
#include <signal.h>
#include <zero/cdecl.h>
#include <zero/types.h>
#include <kern/util.h>
//#include <kern/obj.h>
#include <kern/proc/proc.h>
#if !defined(__arm__)
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/cpu.h>
#endif

extern struct task  tasktab[NTASK];
extern long         trapsigmap[TRAPNCPU];

signalhandler_t    *sigfunctab[NSIG] ALIGNED(PAGESIZE);

void
killproc(volatile struct proc *proc)
{
    ;
}

#if (FASTINTR)
FASTCALL
#endif
void
sigfunc(unsigned long pid, int32_t trap, long err)
{
    volatile struct task *task = &tasktab[pid];
    long                  sig = trapsigmap[trap];
    volatile struct proc *proc = task->proc;
    signalhandler_t      *func;

    if (pid < TASKNPREDEF) {
        panic(pid, trap, err);
    }
//    kprintf("trap 0x%lx -> signal 0x%lx\n", trap, sig);
    if (trap == TRAPUD) {
//        kprintf("PANIC: #UD (0x%lx)\n", errcode);
    } else if (trap == TRAPGP) {
//        kprintf("PANIC: #GP (0x%lx)\n", errcode);
    } else if (sig == SIGKILL) {
//        kprintf("trap 0x%lx -> signal 0x%lx\n", trap, sig);
        killproc(proc);
    } else if ((sig) && sigismember(&task->sigmask, sig)) {
//        kprintf("trap 0x%lx -> signal 0x%lx\n", trap, sig);
        func = proc->sigvec[sig];
        if (func) {
            func(sig);
        } else {
            func = sigfunctab[sig];
            if (func) {
                func(sig);
            }
        }
    }

    return;
}

