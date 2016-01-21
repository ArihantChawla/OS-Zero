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
#include <kern/obj.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/ia32/task.h>

struct proc proctab[NTASK] ALIGNED(PAGESIZE);
struct task tasktab[NTASK];

long
procinit(long id, long sched)
{
    struct cpu     *cpu = k_curcpu;
    struct proc    *proc = &proctab[id];
    struct task    *task = &tasktab[id];
    long            prio;
    long            val;
    struct taskstk *stk;
    void           *ptr;
    uint8_t        *u8ptr;

    proc->nice = 0;
    proc->task = task;
    task->m_task.flg = 0;
    if (k_cpuinfo->flags & CPUHASFXSR) {
        task->m_task.flg |= CPUHASFXSR;
    }
    val = 0;
    task->schedflg = val;
    task->state = TASKNEW;
    task->score = val;
    task->cpu = cpu->id;
    task->slice = val;
    task->runtime = val;
    task->slptime = val;
    task->ntick = val;
    val = cpu->ntick;
    task->lastrun = val;
    task->firstrun = val;
    task->lasttick = val;
    task->proc = proc;
    k_curproc = proc;
    k_curtask = task;
    proc->pid = id;
    if (id < TASKNPREDEF) {
        prio = SCHEDSYSPRIOMIN;
        task->sched = SCHEDSYSTEM;
        task->prio = prio;
    } else {
        if (sched == SCHEDNOCLASS) {
            prio = SCHEDUSERPRIOMIN;
            task->sched = SCHEDNORMAL;
        } else {
            prio = schedcalcbaseprio(task, sched);
            task->sched = sched;
        }
        task->prio = prio;
        /* initialise page directory */
        ptr = kmalloc(NPDE * sizeof(pde_t));
        if (ptr) {
            kbzero(ptr, NPDE * sizeof(pde_t));
            proc->vmpagemap.dir = ptr;
        } else {
            kfree(proc);
            
            return -1;
        }
#if 0
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
            kfree(proc->vmpagemap.dir);
            kfree(task->kstk.base);
            kfree(proc);
            
            return -1;
        }
#endif
        /* initialise descriptor table */
        ptr = kmalloc(TASKNDESC * sizeof(struct desc));
        if (ptr) {
            kbzero(ptr, TASKNDESC * sizeof(struct desc));
            proc->desctab = ptr;
            proc->ndesctab = TASKNDESC;
        } else {
            kfree(proc->vmpagemap.dir);
#if 0
            kfree(task->ustk.base);
            kfree(task->kstk.base);
#endif
            kfree(proc);
            
            return -1;
        }
#if 0
        /* initialise VM structures */
        ptr = kmalloc(NPAGEMAX * sizeof(struct userpage));
        if (ptr) {
            kbzero(ptr, NPAGEMAX * sizeof(struct userpage));
            proc->pagetab = ptr;
            proc->npagetab = NPAGEMAX;
        } else {
            kfree(proc->vmpagemap.dir);
#if 0
            kfree(task->ustk.base);
            kfree(task->kstk.base);
#endif
            kfree(proc->desctab);
            kfree(proc);
            
            return -1;
        }
#endif
    }
    task->state = TASKREADY;
    
    return 0;
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

