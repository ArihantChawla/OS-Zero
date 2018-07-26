#include <kern/conf.h>
#include <stdint.h>
#include <signal.h>
#include <zero/cdefs.h>
#include <mach/param.h>
//#include <mach/types.h>
#include <kern/util.h>
//#include <kern/cpu.h>
//#include <kern/obj.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/task.h>

extern long        k_trapsigmap[NCPU];

__sighandler_t     k_sigfunctab[NSIG] ALIGNED(PAGESIZE);
long               k_traperrbits = TRAPERRBITS;

void
prockill(pid_t pid, long sig)
{
    kprintf("PROC %ld received signal %ld\n", (long)pid, sig);
}

FASTCALL
void
sigfunc(unsigned long pid, int32_t trap, long err, void *frame)
{
    struct task    *task = &k_tasktab[pid];
    long            sig = k_trapsigmap[trap];
    struct proc    *proc = task->proc;
    __sighandler_t  func;

    if (pid < TASKNPREDEF) {
        panic(trap, err, frame);
    }
    if (trap == TRAPUD
        || trap == TRAPGP
        || sig == SIGKILL) {
        prockill(pid, sig);
    } else {
        func = proc->sigvec[sig];
        if (func) {
            func(sig);
        } else {
            func = k_sigfunctab[sig];
            if (func) {
                func(sig);
            }
        }
    }

    return;
}

