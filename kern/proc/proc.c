#include <stddef.h>
#include <stdint.h>
#include <signal.h>
#include <mach/param.h>
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
#include <kern/unit/x86/asm.h>
#include <kern/unit/x86/link.h>

extern uintptr_t      k_pagedir[PDESMAX];
struct proc          *k_proctab[TASKSMAX] ALIGNED(PAGESIZE);
volatile struct proc *k_proczombietab[TASKSMAX];

long
procinit(long unit, long id, long sched)
{
    struct task         *task = NULL;
    volatile struct cpu *cpu;
    struct proc         *proc;
    long                 prio;
    struct taskstk      *stk;
    void                *ptr;
    uint8_t             *u8ptr;

    k_introff();
    cpu = &k_cputab[unit];
    if (id == PROCNEW) {
        id = taskgetid();
        task = &k_tasktab[id];
        kbzero(task, sizeof(struct task));
        proc = k_proctab[id];
        kbzero(proc, sizeof(struct proc));
        proc->pid = id;
        proc->task = task;
        task->proc = proc;
        if (sched == SCHEDNOCLASS) {
            prio = SCHEDUSERPRIOMIN;
            task->sched = SCHEDNORMAL;
            task->prio = prio;
        } else {
            prio = schedclassminprio(sched);
            task->sched = sched;
            task->prio = prio;
        }
        task->flg |= CPUHASFXSR;
        if (task->state == TASKNEW) {
            /* initialise page directory */
            ptr = kwalloc(PDESMAX * sizeof(uintptr_t));
            if (ptr) {
                kbzero(ptr, PDESMAX * sizeof(uintptr_t));
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
            ptr = kmalloc(PROCDESCS * sizeof(struct desc));
            if (ptr) {
                kbzero(ptr, PROCDESCS * sizeof(struct desc));
                proc->desctab = ptr;
                proc->ndesctab = PROCDESCS;
            } else {
                if (id >= TASKPREDEFS) {
                    kfree(proc->pagetab);
                    kfree(proc->pagedir);
                    kfree(proc);
                }
            }
        }
    } else if (id < TASKPREDEFS) {
        task = &k_tasktab[id];
        kbzero(task, sizeof(struct task));
        proc = k_proctab[id];
        kbzero(proc, sizeof(struct proc));
        proc->pid = id;
        prio = SCHEDSYSPRIOMIN;
        task->sched = SCHEDSYSTEM;
        task->flg |= CPUHASFXSR;
        task->prio = prio;
        proc->pagedir = (uintptr_t *)k_pagedir;
        proc->pagetab = (uintptr_t *)&_pagetab;
    }
    k_setcurcpu(cpu);
    k_setcurunit(unit);
    k_setcurtask(task);
    k_setcurpid(id);
    task->state = TASKREADY;
    k_intron();

    return id;
}

/* see <kern/proc.h> for definitions of scheduler classes */
struct proc *
procrun(long unit, long sched, int argc, char *argv[], char *envp[])
{
    long         id = taskgetid();
    struct proc *proc = k_proctab[id];

    if (proc) {
        procinit(unit, id, sched);
        proc->argc = argc;
        proc->argv = argv;
        proc->envp = envp;
    }

    return proc;
}

