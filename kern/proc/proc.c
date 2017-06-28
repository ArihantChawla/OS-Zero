#include <stddef.h>
#include <stdint.h>
#include <signal.h>
#include <zero/param.h>
#include <kern/conf.h>
#include <kern/util.h>
#include <kern/obj.h>
#include <kern/malloc.h>
#include <kern/sched.h>
#include <kern/cpu.h>
#include <kern/mem/vm.h>
#include <kern/mem/page.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
//#include <kern/obj.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/link.h>
#include <kern/unit/ia32/task.h>

extern pde_t        kernpagedir[NPDE];
extern struct task  tasktab[NTASK];
struct proc         proctab[NTASK] ALIGNED(PAGESIZE);
struct proc        *proczombietab[NTASK];

long
procinit(long id, long sched)
{
    struct cpu     *cpu;
    struct proc    *proc;
    struct task    *task;
    long            prio;
    long            val;
    struct taskstk *stk;
    void           *ptr;
    uint8_t        *u8ptr;

    if (id < TASKNPREDEF) {
        cpu = &cputab[0];
        proc = &proctab[id];
        task = &tasktab[id];
        prio = SCHEDSYSPRIOMIN;
        task->sched = SCHEDSYSTEM;
        task->prio = prio;
        proc->pagedir = (pde_t *)kernpagedir;
        proc->pagetab = (pte_t *)&_pagetab;
        task->state = TASKREADY;
        if (cpu->info.flg & CPUHASFXSR) {
            task->flg |= CPUHASFXSR;
        }
        k_curcpu = cpu;
        k_curunit = 0;
        k_curtask = task;
        k_curpid = id;

        return id;
    } else {
        cpu = &cputab[0];
        id = taskgetid();
        proc = &proctab[id];
        task = &tasktab[id];
        task->state = TASKNEW;
        proc->pid = id;
        proc->nice = 0;
        proc->task = task;
        k_curtask = task;
        task->proc = proc;
        val = 0;
        if (cpu->flg & CPUHASFXSR) {
            val = CPUHASFXSR;
            task->flg |= val;
        }
        val = 0;
        task->flg = val;
        task->score = val;
        task->slice = val;
        task->runtime = val;
        task->slptime = val;
        task->ntick = val;
        val = cpu->ntick;
        task->lastrun = val;
        task->firstrun = val;
        task->lasttick = val;
        if (sched == SCHEDNOCLASS) {
            prio = SCHEDUSERPRIOMIN;
            task->sched = SCHEDNORMAL;
            task->prio = prio;
        } else {
            prio = schedclassminprio(sched);
            task->sched = sched;
            task->prio = prio;
        }
        if (task->state == TASKNEW) {
            /* initialise page directory */
            ptr = kwalloc(NPDE * sizeof(pde_t));
            if (ptr) {
                kbzero(ptr, NPDE * sizeof(pde_t));
                proc->pagedir = ptr;
            } else {
                kfree(proc);
                
                return -1;
            }
#if (VMFLATPHYSTAB)
            /* initialise page tables */
            ptr = kwalloc(PAGETABSIZE);
            if (ptr) {
                kbzero(ptr, PAGETABSIZE);
                proc->pagetab = ptr;
            } else {
                kfree(proc->pagedir);
                kfree(proc);
                
                return -1;
            }
#endif
            /* initialise descriptor table */
            ptr = kmalloc(NPROCFD * sizeof(struct desc));
            if (ptr) {
                kbzero(ptr, NPROCFD * sizeof(struct desc));
                proc->desctab = ptr;
                proc->ndesctab = NPROCFD;
            } else {
                if (id >= TASKNPREDEF) {
                    kfree(proc->pagetab);
                    kfree(proc->pagedir);
                    kfree(proc);
                }
            }
        }
    }
    
    return id;
}

/* see <kern/proc.h> for definitions of scheduler classes */
struct proc *
newproc(int argc, char *argv[], char *envp[], long sched)
{
    long         id = taskgetid();
    struct proc *proc = (id >= 0) ? &proctab[id] : NULL;
    struct task *task = (id >= 0) ? &tasktab[id] : NULL;

    if (proc) {
        procinit(id, sched);
        proc->argc = argc;
        proc->argv = argv;
        proc->envp = envp;
    }

    return proc;
}

