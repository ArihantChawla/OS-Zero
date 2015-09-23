#include <stddef.h>
#include <signal.h>
#include <kern/conf.h>
#include <kern/util.h>
#include <kern/obj.h>
#include <kern/malloc.h>
#include <zero/param.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/obj.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/vm.h>

struct proc proctab[NTASK] ALIGNED(PAGESIZE);
//struct task tasktab[NTASK] ALIGNED(PAGESIZE);

long
procinit(long id)
{
    long           taskid = (id == PROCKERN) ? id : taskgetid();
    struct proc   *proc = &proctab[taskid];
    struct task   *task;
    void          *ptr;

    if (taskid == PROCKERN) {
        /* bootstrap */
        task = &proc->task;
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
//        kbzero(proc, sizeof(struct proc));
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
            kbzero(ptr, KERNSTKSIZE);
            proc->task.kstk = ptr;
        }
        ptr = kmalloc(PROCSTKSIZE);
        if (ptr) {
            kbzero(ptr, PROCSTKSIZE);
            proc->task.ustk = ptr;
        } else {
            kfree(proc->pdir);
            kfree(proc->task.kstk);
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
            kfree(proc->task.ustk);
            kfree(proc->task.kstk);
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
            kfree(proc->task.ustk);
            kfree(proc->task.kstk);
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
    struct proc *proc = kmalloc(sizeof(struct proc));

    proc->task.sched = sched;
    proc->task.id = taskgetid();
    proc->task.parent = proc;
    proc->argc = argc;
    proc->argv = argv;
    proc->envp = envp;

    return proc;
}

