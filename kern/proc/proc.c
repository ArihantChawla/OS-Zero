#include <stddef.h>
#include <stdint.h>
#include <signal.h>
#include <kern/conf.h>
#include <kern/util.h>
#include <kern/obj.h>
#include <kern/malloc.h>
#include <zero/param.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/obj.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/ia32/vm.h>

struct proc proctab[NTASK] ALIGNED(PAGESIZE);
struct task tasktab[NTASK] ALIGNED(PAGESIZE);

long
procinit(long id)
{
    long            taskid = (id < TASKNPREDEF) ? id : taskgetid();
    struct proc    *proc = &proctab[taskid];
    struct task    *task = &tasktab[taskid];
    struct taskstk *stk;
    void           *ptr;
    uint8_t        *u8ptr;

    if (taskid == PROCKERN) {
        /* bootstrap */
        proc->task = task;
        task->parent = proc;
        k_curproc = proc;
        task->state = TASKREADY;
        task->nice = 0;
        task->sched = SCHEDSYS;
        task->prio = 0;
        k_curtask = task;
    }
    if (proc) {
        /* initialise page directory */
        ptr = kmalloc(NPDE * sizeof(pde_t));
        if (ptr) {
            kbzero(ptr, NPDE * sizeof(pde_t));
            proc->pdir = ptr;
        } else {
            kfree(proc);

            return -1;
        }
        ptr = kmalloc(KERNSTKSIZE);
        if (ptr) {
            u8ptr = ptr;
            stk = &task->kstk;
            u8ptr += KERNSTKSIZE;
            kbzero(ptr, KERNSTKSIZE);
            stk->top = u8ptr;
            stk->sp = u8ptr;
            stk->base = ptr;
            stk->size = KERNSTKSIZE;
        }
        ptr = kmalloc(TASKSTKSIZE);
        if (ptr) {
            u8ptr = ptr;
            stk = &task->ustk;
            u8ptr += KERNSTKSIZE;
            kbzero(ptr, TASKSTKSIZE);
            stk->top = u8ptr;
            stk->sp = u8ptr;
            stk->base = ptr;
            stk->size = TASKSTKSIZE;
        } else {
            kfree(proc->pdir);
            kfree(task->kstk.base);
            kfree(proc);

            return -1;
        }
        /* initialise descriptor table */
        ptr = kmalloc(TASKNDESC * sizeof(struct desc));
        if (ptr) {
            kbzero(ptr, TASKNDESC * sizeof(struct desc));
            proc->dtab = ptr;
        } else {
            kfree(proc->pdir);
            kfree(task->ustk.base);
            kfree(task->kstk.base);
            kfree(proc);

            return -1;
        }
#if 0
        /* initialise VM structures */
        ptr = kmalloc(VMNHDR * sizeof(struct vmpage));
        if (ptr) {
            kbzero(ptr, VMNHDR * sizeof(struct vmpage));
            proc->vmhdrtab = ptr;
        } else {
            kfree(proc->pdir);
            kfree(task->ustk.base);
            kfree(task->kstk.base);
            kfree(proc->dtab);
            kfree(proc);

            return -1;
        }
#endif
        task->state = TASKREADY;
    }

    return 0;
}

struct desc *
procgetdesc(struct proc *proc, long id)
{
    struct desc *ret = &proc->dtab[id];

    return ret;
}

/* see <kern/proc.h> for definitions of scheduler classes */
struct proc *
newproc(int argc, char *argv[], char *envp[], long sched)
{
    long         taskid = taskgetid();
    struct proc *proc = &proctab[taskid];
    struct task *task = &tasktab[taskid];

    task->state = TASKNEW;
    task->sched = sched;
    task->parent = proc;
    proc->task = task;
    proc->argc = argc;
    proc->argv = argv;
    proc->envp = envp;

    return proc;
}

