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
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/proc/sched.h>
#include <kern/mem.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/task.h>
#include <zero/mtx.h>

static struct taskwait  taskwaittab[NLVL0TASK] ALIGNED(PAGESIZE);
static struct taskqueue taskruntab[SCHEDNCLASS * SCHEDNPRIO];
static struct taskqueue taskrtqueue;
//extern long             trappriotab[NINTR];
static struct taskid    taskidtab[NTASK] ALIGNED(PAGESIZE);
static struct taskidq   taskidq;
static volatile long    taskidlk;

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

/* TODO: use <zero/list.h>? */

/* add task to end of queue */
void
taskqueue(struct task *task, struct taskqueue *taskqueue)
{
    task->prev = taskqueue->tail;
    if (task->prev) {
        task->prev->next = task;
    } else {
        taskqueue->head = task;
    }
    taskqueue->tail = task;

    return;
}

/* add taskead to wait queue */
long
taskaddwait(struct task *task)
{
    struct task     *head;
    struct taskwait *tab;
    struct taskwait *ptr = NULL;
    uintptr_t        wchan = task->wchan;
    long             ret = -1;
    long             fail = 0;    
    uint64_t         key0;
    uint64_t         key1;
    uint64_t         key2;
    uint64_t         key3;
    void            *ptab[TASKNKEY] = { NULL, NULL, NULL, NULL };

    key0 = taskwaitkey0(wchan);
    key1 = taskwaitkey1(wchan);
    key2 = taskwaitkey2(wchan);
    key3 = taskwaitkey3(wchan);
    tab = &taskwaittab[key0];
    mtxlk(&tab->lk);
    ptr = tab->ptr;
    if (!ptr) {
        ptr = kmalloc(NLVL0TASK * sizeof(struct taskwait));
        if (ptr) {
            kbzero(ptr, NLVL0TASK * sizeof(struct taskwait));
        }
        ptab[0] = ptr;
    }
    if (ptr) {
        ptr = ptr[key0].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL1TASK * sizeof(struct taskwait));
            if (ptr) {
                kbzero(ptr, NLVL1TASK * sizeof(struct taskwait));
            }
            ptab[1] = ptr;
        }
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = ptr[key1].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL2TASK * sizeof(struct taskwait));
            if (ptr) {
                kbzero(ptr, NLVL2TASK * sizeof(struct taskwait));
            }
            ptab[2] = ptr;
        }
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = ptr[key2].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL3TASK * sizeof(struct taskwait));
            if (ptr) {
                kbzero(ptr, NLVL3TASK * sizeof(struct taskwait));
            }
            ptab[3] = ptr;
        }
    } else {
        fail = 1;
    }
    if (!fail) {
        if (ptab[0]) {
            tab->nref++;
            tab->ptr = ptab[0];
        }
        tab = tab->ptr;
        if (ptab[1]) {
            tab[key0].nref++;
            tab[key0].ptr = ptab[1];
        }
        tab = tab[key0].ptr;
        if (ptab[2]) {
            tab[key1].nref++;
            tab[key1].ptr = ptab[2];
        }
        tab = tab[key1].ptr;
        if (ptab[3]) {
            tab[key2].nref++;
            tab[key2].ptr = ptab[3];
        }
        tab = tab[key2].ptr;
        tab = &tab[key3];
        tab->nref++;
        task->prev = NULL;
        task->next = tab->ptr;
        if (task->next) {
            task->next->prev = task;
        }
        task->next = tab->ptr;
        tab->ptr = task;
        ret ^= ret;
    }
    mtxunlk(&tab->lk);
    
    return ret;
}

/* move a task from wait queue to ready queue */
void
taskwakeup(uintptr_t wchan)
{
    struct taskwait  *tab;
    struct taskwait  *ptr = NULL;
    struct taskqueue *taskq;
    struct task      *task1;
    struct task      *task2;
    long              key0 = taskwaitkey0(wchan);
    long              key1 = taskwaitkey1(wchan);
    long              key2 = taskwaitkey2(wchan);
    long              key3 = taskwaitkey3(wchan);
    long              prio;
//    long             n;
    struct taskwait  *ptab[4] = { NULL };

    tab = &taskwaittab[key0];
    mtxlk(&tab->lk);
    ptr = tab;
    if (ptr) {
        ptab[0] = ptr;
        ptr = ((void **)ptr->ptr)[key0];
        if (ptr) {
            ptab[1] = ptr;
            ptr = ((void **)ptr->ptr)[key1];
            if (ptr) {
                ptab[2] = ptr;
                ptr = ((void **)ptr->ptr)[key2];
                if (ptr) {
                    ptab[3] = ptr;
                    ptr = ((void **)ptr->ptr)[key3];
                }
            }
        }
        task1 = ptr->ptr;
        if (task1) {
            task2 = task1->next;
            taskq = &taskruntab[task1->prio];
            mtxlk(&taskq->lk);
            taskqueue(task1, &taskruntab[task1->prio]);
            ptr->ptr = task2;
            mtxunlk(&taskq->lk);
        }
        while (task1) {
            prio = taskwakeprio(task1);
            task2 = task1->next;
            taskqueue(task1, &taskruntab[prio]);
            task1 = task2;
        }
        /* TODO: free tables if possible */
        ptr = ptab[0];
        if (ptr) {
            if (!--ptr->nref) {
                kfree(ptr->ptr);
                ptr->ptr = NULL;
            }
            ptr = ptab[1];
            if (ptr) {
                if (!--ptr->nref) {
                    kfree(ptr->ptr);
                    ptr->ptr = NULL;
                }
                ptr = ptab[2];
                if (ptr) {
                    if (!--ptr->nref) {
                        kfree(ptr->ptr);
                        ptr->ptr = NULL;
                    }
                    ptr = ptab[3];
                    if (ptr) {
                        if (!--ptr->nref) {
                            kfree(ptr->ptr);
                            ptr->ptr = NULL;
                        }
                    }
                }
            }
        }
    }
    mtxunlk(&tab->lk);
}

/* switch taskeads */
FASTCALL
struct task *
taskpick(void)
{
    struct task      *task = k_curtask;
    struct taskqueue *taskq;
    long              prio;
    long              sched;
    long              state;

    if (task) {
        sched = task->sched;
        state = task->state;
        if (sched == SCHEDRT && state == TASKREADY) {
            taskq = &taskrtqueue;
            mtxlk(&taskq->lk);
            taskqueue(task, taskq);
            mtxunlk(&taskq->lk);
        } else if (state == TASKREADY) {
            prio = taskadjprio(task);
            taskq = &taskruntab[prio];
            mtxlk(&taskq->lk);
            taskqueue(task, taskq);
            mtxunlk(&taskq->lk);
        } else if (state == TASKWAIT) {
            taskaddwait(task);
        }
    }
    task = NULL;
    while (!task) {
        for (prio = 0 ; prio < SCHEDNCLASS * SCHEDNPRIO ; prio++) {
            taskq = &taskruntab[prio];
            mtxlk(&taskq->lk);
            task = taskq->head;
            task->prev = NULL;
            if (task) {
                if (task->next) {
                    task->next->prev = NULL;
                } else {
                    taskq->tail = NULL;
                }
                taskq->head = task->next;
                mtxunlk(&taskq->lk);

                return task;
            } else {
                mtxunlk(&taskq->lk);
            }
        }
    }

    return task;
}

long
taskgetid(void)
{
    struct taskid *taskid;
    long           retval;

    mtxlk(&taskidlk);
    taskid = taskidq.head;
    if (!taskid) {
        retval = -1;
    } else {
        if (taskid->next) {
            taskid->next->prev = NULL;
        }
        taskidq.head = taskid->next;
        retval = taskid->id;
    }
    mtxunlk(&taskidlk);

    return retval;
}

void
taskfreeid(long id)
{
    struct taskid *taskid;
    
    mtxlk(&taskidlk);
    taskid = &taskidtab[id];
    taskid->prev = taskidq.tail;
    if (taskidq.tail) {
        taskidq.tail->next = taskid;
    } else {
        taskidq.head = taskid;
    }
    taskidq.tail = taskid;
    mtxunlk(&taskidlk);
}

void
taskinitids(void)
{
    long           id = TASKNPREDEF;
    struct taskid *taskid;

    mtxlk(&taskidlk);
    taskid = &taskidtab[id];
    taskid->id = id;
    taskidq.head = taskidq.tail = taskid;
    for (id = 1 ; id < NTASK ; id++) {
        taskid = &taskidtab[id];
        taskid->id = id;
        taskid->prev = taskidq.tail;
        taskidq.tail->next = taskid;
        taskidq.tail = taskid;
    }
    mtxunlk(&taskidlk);

    return;
}

void
taskinit(void)
{
    taskinitids();

    return;
}

#endif /* ZEROSCHED */

