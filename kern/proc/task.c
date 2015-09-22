#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <sys/io.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/trix.h>
//#include <zero/randlfg2.h>
#include <zero/asm.h>
#include <kern/conf.h>
#include <kern/util.h>
#include <kern/obj.h>
#include <kern/sched.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/malloc.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/task.h>
#include <zero/mtx.h>

#define LIST_TYPE struct task
#include <zero/list.h>

/* this should be a single (aligned) cacheline */
struct tasklk {
    volatile long lk;
    uint8_t       pad[CLSIZE - sizeof(long)];
};

static struct taskwait *taskwaittab[NLVL0TASK] ALIGNED(PAGESIZE);
//static struct task taskruntab[SCHEDNCLASS * SCHEDNPRIO];
static struct tasklk    taskrunmtxtab[SCHEDNCLASS * SCHEDNPRIO] ALIGNED(PAGESIZE);
static struct task     *taskruntab[SCHEDNCLASS * SCHEDNPRIO];
//static struct task taskrtqueue;
//extern long             trappriotab[NINTR];
static struct taskid   *taskidtab[NTASK] ALIGNED(PAGESIZE);
static struct taskid   *taskidqueue;
static struct tasklk    taskidmtx ALIGNED(CLSIZE);
static struct tasklk    taskwaitmtx ALIGNED(CLSIZE);

/* save taskead context */
ASMLINK
void
tasksave(struct task *task, long retadr, long fp)
{
    uint8_t *fctx = task->m_tcb.fctx;

    if (k_curcpu->info->flags & CPUHASFXSR) {
        __asm__ __volatile__ ("fxsave (%0)\n" : : "r" (fctx));
    } else {
        __asm__ __volatile__ ("fnsave (%0)\n" : : "r" (fctx));
    }
    m_tcbsave(&task->m_tcb);
    task->m_tcb.iret.eip = retadr;
    task->m_tcb.iret.uesp = fp - sizeof(struct m_trapframe);
    task->m_tcb.genregs.ebp = fp;

    return;
}

/* run task */
FASTCALL
void
taskjmp(struct task *task)
{
    uint8_t *fctx = task->m_tcb.fctx;

    if (k_curcpu->info->flags & CPUHASFXSR) {
        __asm__ __volatile__ ("fxrstor (%0)\n" : : "r" (fctx));
    } else {
        __asm__ __volatile__ ("frstor (%0)\n" : : "r" (fctx));
    }
    k_curtask = task;
    k_curproc = task->parent;
    k_curpid = k_curproc->task.id;
    m_tcbjmp(&task->m_tcb);

    /* NOTREACHED */
    return;
}

#if (ZEROSCHED)

/* adjust task priority */
static __inline__ long
taskadjprio(struct task *task)
{
    long prio = task->prio;
    long nice = task->nice;
    long sched = task->sched;

    if (sched == SCHEDRT) {

        return prio;
    } else if (sched == SCHEDFIXED) {
        /* TODO */
        ;
    } else {
        /* wrap around back to 0 at SCHEDNPRIO / 2 */
        prio++;
        prio &= (SCHEDNPRIO >> 1) - 1;
//        prio = (SCHEDNPRIO * sched) + (SCHEDNPRIO >> 1) + prio + task->nice;
        prio += (SCHEDNPRIO * sched)
//            + (randlfg2() & ((SCHEDNPRIO >> 1) - 1))
            + nice;
        prio = min(SCHEDNPRIO * SCHEDNCLASS - 1, prio);
    }
    task->prio = prio;

    return prio;
}

static __inline__ long
taskwakeprio(struct task *task)
{
    long nice = task->nice;
    long sched = task->sched;
    long prio = sched * SCHEDNPRIO;

    return prio;
}

/* add task to beginning of queue */
void
taskpush(struct task *task, struct task *taskqueue)
{
    if (!listisempty(taskqueue)) {
        listaddbefore(task, taskqueue);
    } else {
        listinit(task, taskqueue);
    }

    return;
}

/* get/remove task from beginning of queue */
struct task *
taskpop(struct task *taskqueue)
{
    struct task *task;

    task = taskqueue;
    if (taskqueue) {
        if (!listisempty(taskqueue)) {
            if (listissingular(taskqueue)) {
                taskqueue->prev = NULL;
                taskqueue->next = NULL;
            } else {
                taskqueue->prev->next = taskqueue->next;
                taskqueue->next->prev = taskqueue->prev;
            }
        } else {
            task = NULL;
        }
    }

    return task;
}

/* add task to end of queue */
void
taskqueue(struct task *task, struct task *taskqueue)
{
    if (!listisempty(taskqueue)) {
        listaddafter(task, taskqueue->prev);
    } else {
        listinit(task, taskqueue);
    }

    return;
}

/* add taskead to wait queue */
long
taskaddwait(struct task *task)
{
    struct task      *head;
    struct taskwait  *tab;
    struct taskwait  *ptr = NULL;
    struct taskwait **pptr;
    struct taskwait  *item = NULL;
    uintptr_t         wchan = task->wchan;
    long              ret = -1;
    long              fail = 0;    
    uint64_t          key0;
    uint64_t          key1;
    uint64_t          key2;
    uint64_t          key3;
    void             *ptab[TASKNKEY - 1] = { NULL, NULL, NULL };

    key0 = taskwaitkey0(wchan);
    key1 = taskwaitkey1(wchan);
    key2 = taskwaitkey2(wchan);
    key3 = taskwaitkey3(wchan);
    ptr = taskwaittab[key0];
    pptr = (struct taskwait **)ptr;
    mtxlk(&taskwaitmtx.lk);
    if (!ptr) {
        ptr = kmalloc(NLVL1TASK * sizeof(struct taskwait));
        if (ptr) {
            kbzero(ptr, NLVL1TASK * sizeof(struct taskwait));
        }
        ptab[0] = ptr;
        pptr = (struct taskwait **)ptr;
        taskwaittab[key0] = ptr;
    }
    if (ptr) {
        ptr = pptr[key1];
        if (!ptr) {
            ptr = kmalloc(NLVL2TASK * sizeof(struct taskwait));
            if (ptr) {
                kbzero(ptr, NLVL2TASK * sizeof(struct taskwait));
            }
            ptab[1] = ptr;
            pptr[key1] = ptr;
            pptr = (struct taskwait **)ptr;
        }
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = pptr[key2];
        if (!ptr) {
            ptr = kmalloc(NLVL3TASK * sizeof(struct taskwait));
            if (ptr) {
                kbzero(ptr, NLVL3TASK * sizeof(struct taskwait));
            }
            ptab[2] = ptr;
            pptr[key2] = ptr;
            pptr = (struct taskwait **)ptr;
        }
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = pptr[key3];
        item = ptr;
    } else {
        fail = 1;
    }
    if (!fail) {
        item->ptr = task;
        tab = ptab[0];
        if (tab) {
            tab->nref++;
        }
        tab = ptab[1];
        if (tab) {
            tab->nref++;
        }
        tab = ptab[2];
        if (ptab[2]) {
            tab->nref++;
        }
        listaddafter(item, tab->prev);
        ret ^= ret;
    }
    mtxunlk(&taskwaitmtx.lk);
    
    return ret;
}

/* move a task from wait queue to ready queue */
void
taskwakeup(uintptr_t wchan)
{
    struct taskwait  *tab;
    struct taskwait  *ptr = NULL;
    struct task      *taskq;
    struct task      *task1;
    struct task      *task2;
    long              key0 = taskwaitkey0(wchan);
    long              key1 = taskwaitkey1(wchan);
    long              key2 = taskwaitkey2(wchan);
    long              key3 = taskwaitkey3(wchan);
    long              prio;
//    long             n;
    struct taskwait **pptr;
    struct taskwait  *ptab[TASKNKEY - 1] = { NULL, NULL, NULL };

    tab = taskwaittab[key0];
    mtxlk(&taskwaitmtx.lk);
    if (ptr) {
        ptab[0] = tab;
        tab = ((void **)tab)[key0];
        if (tab) {
            ptab[1] = tab;
            tab = ((void **)tab)[key1];
            if (tab) {
                ptab[2] = tab;
                tab = ((void **)tab)[key2];
                if (tab) {
                    tab = ((void **)tab)[key3];
                }
            }
        }
        task1 = tab->ptr;
        if (task1) {
            prio = task1->prio;
            mtxlk(&taskrunmtxtab[prio].lk);
            taskqueue(task1, taskruntab[prio]);
            mtxunlk(&taskrunmtxtab[prio].lk);
        }
        while (task1) {
            prio = taskwakeprio(task1);
            task2 = task1->next;
            mtxlk(&taskrunmtxtab[prio].lk);
            taskqueue(task1, taskruntab[prio]);
            mtxunlk(&taskrunmtxtab[prio].lk);
            task1 = task2;
        }
        /* TODO: free tables if possible */
        tab = ptab[0];
        if (tab) {
            if (!--tab->nref) {
                pptr = &taskwaittab[key0];
                kfree(tab);
                *pptr = NULL;
            }
            tab = ptab[1];
            if (tab) {
                if (!--tab->nref) {
                    pptr = &pptr[key1];
                    kfree(tab);
                    *pptr = NULL;
                }
                tab = ptab[2];
                if (tab) {
                    if (!--tab->nref) {
                        pptr = &pptr[key2];
                        kfree(tab);
                        *pptr = NULL;
                    }
                }
            }
        }
    }
    mtxunlk(&taskwaitmtx.lk);
}

#if 0

/* switch tasks */
FASTCALL
struct task *
taskpick(void)
{
    struct task      *task = k_curtask;
    struct task *taskq;
    long              prio;
    long              sched;
    long              state;

    if (task) {
        sched = task->sched;
        state = task->state;
        if (sched == SCHEDRT && state == TASKREADY) {
            prio = task->prio;
            if (prio < 0) {
                /* SCHED_FIFO */
                prio = -prio;
                taskq = taskruntab[prio];
                mtxlk(&taskrunmtxtab[prio].lk);
                taskpush(task, taskq);
                mtxunlk(&taskrunmtxtab[prio].lk);
            } else {
                /* SCHED_RR */
                taskq = taskruntab[prio];
                mtxlk(&taskrunmtxtab[prio].lk);
                taskqueue(task, taskq);
                mtxunlk(&taskrunmtxtab[prio].lk);
            }
        } else if (state == TASKREADY) {
            prio = taskadjprio(task);
            taskq = taskruntab[prio];
            mtxlk(&taskrunmtxtab[prio].lk);
            taskqueue(task, taskq);
            mtxunlk(&taskrunmtxtab[prio].lk);
        } else if (state == TASKWAIT) {
            taskaddwait(task);
        }
    }
    task = NULL;
    while (!task) {
        for (prio = 0 ; prio < SCHEDNCLASS * SCHEDNPRIO ; prio++) {
            taskq = taskruntab[prio];
            mtxlk(&taskrunmtxtab[prio].lk);
            task = taskq->head;
            task->prev = NULL;
            if (task) {
                if (task->next) {
                    task->next->prev = NULL;
                } else {
                    taskq->tail = NULL;
                }
                taskq->head = task->next;
                mtxunlk(&taskrunmtxtab[prio].lk);

                return task;
            } else {
                mtxunlk(&taskrunmtxtab[prio].lk);
            }
        }
    }

    return task;
}

#endif /* 0 */

/* switch tasks */
FASTCALL
struct task *
taskpick(void)
{
    struct task *task = k_curtask;
    struct task *taskq;
    long         prio;
    long         sched;
    long         state;

    if (task) {
        sched = task->sched;
        state = task->state;
        if (sched == SCHEDRT && state == TASKREADY) {
            prio = task->prio;
            if (prio < 0) {
                /* SCHED_FIFO */
                prio = -prio;
                mtxlk(&taskrunmtxtab[prio].lk);
                taskpush(task, taskruntab[prio]);
                mtxunlk(&taskrunmtxtab[prio].lk);
            } else {
                /* SCHED_RR */
                taskq = taskruntab[prio];
                mtxlk(&taskrunmtxtab[prio].lk);
                taskqueue(task, taskq);
                mtxunlk(&taskrunmtxtab[prio].lk);
            }
        } else if (state == TASKREADY) {
            prio = taskadjprio(task);
            taskq = taskruntab[prio];
            mtxlk(&taskrunmtxtab[prio].lk);
            taskqueue(task, taskq);
            mtxunlk(&taskrunmtxtab[prio].lk);

        } else if (state == TASKWAIT) {
            taskaddwait(task);
        }
    }
    task = NULL;
    while (!task) {
        for (prio = 0 ; prio < SCHEDNCLASS * SCHEDNPRIO ; prio++) {
            mtxlk(&taskrunmtxtab[prio].lk);
            taskq = taskruntab[prio];
            if (taskq) {
                task = taskpop(taskq);

                return task;
            } else {
                mtxunlk(&taskrunmtxtab[prio].lk);
            }
        }
    }

    return task;
}

/* get/remove task ID from beginning of queue */
struct taskid *
taskpopid(struct taskid *taskqueue)
{
    struct taskid *taskid;

    taskid = taskidqueue;
    if (taskidqueue) {
        if (!listisempty(taskidqueue)) {
            if (listissingular(taskidqueue)) {
                taskidqueue->prev = NULL;
                taskidqueue->next = NULL;
            } else {
                taskidqueue->prev->next = taskidqueue->next;
                taskidqueue->next->prev = taskidqueue->prev;
            }
        } else {
            taskid = NULL;
        }
    }

    return taskid;
}

long
taskgetid(void)
{
    struct taskid *taskid;
    long           retval = -1;

    mtxlk(&taskidmtx.lk);
    taskid = taskidqueue;
    if (taskid) {
        if (listissingular(taskidqueue)) {
            taskid = taskidqueue;
            taskidqueue->prev = NULL;
            taskidqueue->next = NULL;
        } else {
            taskid = taskpopid(taskidqueue);
            retval = taskid->id;
        }
    }
    mtxunlk(&taskidmtx.lk);

    return retval;
}

void
taskfreeid(long id)
{
    struct taskid *taskid;
    
    mtxlk(&taskidmtx.lk);
    taskid = taskidqueue;
    if (taskidqueue) {
        listaddbefore(taskid, taskidqueue);
    } else {
        listinit(taskid, taskidqueue);
    }
    mtxunlk(&taskidmtx.lk);
}

void
taskinitids(void)
{
    long           id = TASKNPREDEF;
    struct taskid *taskid;

    mtxlk(&taskidmtx.lk);
    taskid = taskidqueue;
    taskid->id = id;
    listinit(taskid, taskidqueue);
    for (id = 1 ; id < NTASK ; id++) {
        taskid = taskidtab[id];
        taskid->id = id;
        listaddafter(taskid, taskidqueue);
    }
    mtxunlk(&taskidmtx.lk);

    return;
}

void
taskinit(void)
{
    taskinitids();

    return;
}

#endif /* ZEROSCHED */

