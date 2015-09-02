#include <stddef.h>
#include <signal.h>
#include <kern/conf.h>
#include <kern/util.h>
#include <kern/obj.h>
#include <kern/mem.h>
#include <zero/param.h>
#include <kern/proc/proc.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/vm.h>

struct proc proctab[NPROC] ALIGNED(PAGESIZE);
struct thr  thrtab[NTHR] ALIGNED(PAGESIZE);

long
procinit(long id)
{
    struct proc *proc = &proctab[id];
    struct thr  *thr;
    void        *ptr;

    if (!id) {
        /* bootstrap */
        k_curproc = proc;
        /* process ID will be zero */
        thr = &thrtab[0];
        thr->state = TASK_READY;
        thr->nice = 0;
        thr->sched = TASK_KERNEL;
        thr->prio = 0;
        proc->thr = thr;
        k_curthr = thr;
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
            proc->kstk = ptr;
        }
        ptr = kmalloc(THRSTKSIZE);
        if (ptr) {
            kbzero(ptr, THRSTKSIZE);
            proc->ustk = ptr;
        } else {
            kfree(proc->pdir);
            kfree(proc->kstk);
            kfree(proc);

            return -1;
        }
        /* initialise descriptor table */
        ptr = kmalloc(OBJNDESC * sizeof(desc_t));
        if (ptr) {
            kbzero(ptr, OBJNDESC * sizeof(desc_t));
            proc->dtab = ptr;
        } else {
            kfree(proc->pdir);
            kfree(proc->ustk);
            kfree(proc->kstk);
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
            kfree(proc->ustk);
            kfree(proc->kstk);
            kfree(proc->dtab);
            kfree(proc);

            return -1;
        }
#endif
        thr->state = TASK_READY;
    }

    return 0;
}

desc_t
procgetdesc(struct proc *proc, long id)
{
    desc_t ret = proc->dtab[id];

    return ret;
}

/* see <kern/proc.h> for definitions of scheduler classes */
struct proc *
newproc(int argc, char *argv[], char *envp[], int sched)
{
    struct proc *proc = kmalloc(sizeof(struct proc));

    proc->sched = sched;
    proc->pid = taskgetpid();
    proc->parent = k_curproc->pid;
    proc->argc = argc;
    proc->argv = argv;
    proc->envp = envp;

    return proc;
}

