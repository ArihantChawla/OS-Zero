#include <kern/conf.h>
#include <stdint.h>
#include <signal.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/types.h>
#include <kern/util.h>
#include <kern/cpu.h>
//#include <kern/obj.h>
#include <kern/proc/proc.h>
#if !defined(__arm__)
#include <kern/unit/x86/trap.h>
#endif

extern struct task tasktab[NTASK];
extern long        trapsigmap[TRAPNCPU];

__sighandler_t     sigfunctab[NSIG] ALIGNED(PAGESIZE);
long               traperrbits = TRAPERRBITS;

void
prockill(struct proc *proc)
{
    ;
}

FASTCALL
void
sigfunc(unsigned long pid, int32_t trap, long err, void *frame)
{
    struct task    *task = &tasktab[pid];
    long            sig = trapsigmap[trap];
    struct proc    *proc = task->proc;
    __sighandler_t  func;

    if (pid < TASKNPREDEF) {
        panic(trap, err);
    }
//    kprintf("trap 0x%lx -> signal 0x%lx\n", trap, sig);
    if (trap == TRAPUD) {
//        kprintf("PANIC: #UD (0x%lx)\n", errcode);
    } else if (trap == TRAPGP) {
//        kprintf("PANIC: #GP (0x%lx)\n", errcode);
    } else if (sig == SIGKILL) {
//        kprintf("trap 0x%lx -> signal 0x%lx\n", trap, sig);
        prockill(proc);
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

