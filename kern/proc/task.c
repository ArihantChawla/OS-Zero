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
static long             tasknicetab[64]
= {
    -25,
    -24,
    -24,
    -23,
    -22,
    -21,
    -20,
    -20,
    -19,
    -18,
    -17,
    -16,
    -15,
    -14,
    -13,
    -13,
    -12,
    -11,
    -10,
    -9,
    -9,
    -8,
    -7,
    -6,
    -5,
    -5,
    -4,
    -3,
    -2,
    -1,
    -1,
    0,
    0,
    1,
    2,
    2,
    3,
    4,
    5,
    6,
    6,
    7,
    8,
    9,
    10,
    10,
    11,
    12,
    13,
    14,
    14,
    15,
    15,
    16,
    16,
    17,
    18,
    19,
    20,
    20,
    21,
    22,
    23,
    24
};
static long            *taskniceptr = &tasknicetab[32];

/* save taskead context */
FASTCALL
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

static __inline__ long
tasksetnice(struct task *task, long val)
{
    long nice;
    
    val = max(-20, val);
    val = min(19, val);
    nice = taskniceptr[val];
    task->nice = nice;

    return nice;
}

/* adjust task priority */
static __inline__ long
taskadjprio(struct task *task)
{
    long prio = task->prio;
    long nice = task->nice;
    long sched = task->sched;

    /* wrap around back to 0 at SCHEDNPRIO */
    prio++;
    prio &= SCHEDNPRIO - 1;
    task->prio = prio;
    prio += SCHEDNPRIO * sched + nice;
    prio = min(SCHEDNPRIO * SCHEDNCLASS - 1, prio);
    prio = max(prio, 0);

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
taskpush(struct task *task, struct task **taskqueue)
{
    struct task *queue = *taskqueue;

    if (queue) {
        if (!listissingular(queue)) {
            listaddbefore(task, queue);
        } else {
            listinit(task);
        }
    } else {
        listinit(task);
    }
    *taskqueue = task;

    return;
}

/* get/remove task from beginning of queue */
struct task *
taskpop(struct task **taskqueue)
{
    struct task *queue = *taskqueue;
    struct task *task = queue;

    if (queue) {
        if (!listissingular(queue)) {
            listdel(task);
            *taskqueue = queue->next;
        } else {
            queue->prev = NULL;
            queue->next = NULL;
            *taskqueue = NULL;
        }
    }

    return task;
}

/* add task to end of queue */
FASTCALL
void
taskqueue(struct task *task, struct task **taskqueue)
{
    struct task *queue = *taskqueue;
    
    if (!listissingular(queue)) {
        listaddafter(task, queue->prev);
    } else {
        listinit(task);
        *taskqueue = task;
    }

    return;
}

/* add taskead to wait queue */
long
taskaddwait(struct task *task)
{
    struct taskwait       *tab;
    void                  *ptr = NULL;
    void                 **pptr;
    struct taskwaitqueue  *qptr;
    struct taskwaitqueue  *queue = NULL;
    uintptr_t              wchan = task->wchan;
    long                   ret = -1;
    long                   fail = 0;    
    long                   key0;
    long                   key1;
    long                   key2;
    long                   key3;
    void                  *ptab[TASKNKEY - 1] = { NULL, NULL, NULL };

    key0 = taskwaitkey0(wchan);
    key1 = taskwaitkey1(wchan);
    key2 = taskwaitkey2(wchan);
    key3 = taskwaitkey3(wchan);
    ptr = taskwaittab[key0];
    pptr = ptr;
    mtxlk(&taskwaitmtx.lk);
    if (!ptr) {
        ptr = kmalloc(NLVL1TASK * sizeof(struct taskwait));
        if (ptr) {
            kbzero(ptr, NLVL1TASK * sizeof(struct taskwait));
        }
        ptab[0] = ptr;
        pptr = ptr;
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
            pptr = ptr;
        }
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = pptr[key2];
        if (!ptr) {
            qptr = kmalloc(NLVL3TASK * sizeof(struct taskwaitqueue));
            if (qptr) {
                kbzero(qptr, NLVL3TASK * sizeof(struct taskwaitqueue));
            } 
            ptab[2] = qptr;
            pptr[key2] = qptr;
        }
    } else {
        fail = 1;
    }
    if (!fail) {
        queue = &qptr[key3];
        queue->nref++;
        queue->task = task;
        task->next = queue->next;
        if (task->next) {
            task->next->prev = task;
        }
        queue->next = task;
        tab = ptab[0];
        tab->nref++;
        tab->tab = ptab[1];
        tab = ptab[1];
        tab->nref++;
        tab->tab = ptab[2];
        tab = ptab[2];
        tab->nref++;
        ret ^= ret;
    }
    mtxunlk(&taskwaitmtx.lk);
    
    return ret;
}

/* move a task from wait queue to ready queue */
void
taskwakeup(uintptr_t wchan)
{
    struct taskwait       *tab;
    void                  *ptr = NULL;
    struct taskwaitqueue  *queue;
    struct task          **taskqptr;
    struct task           *task1;
    struct task           *task2;
    long                   key0 = taskwaitkey0(wchan);
    long                   key1 = taskwaitkey1(wchan);
    long                   key2 = taskwaitkey2(wchan);
    long                   key3 = taskwaitkey3(wchan);
    long                   prio;
    void                 **pptr;
    void                  *ptab[TASKNKEY - 1] = { NULL, NULL, NULL };

    tab = taskwaittab[key0];
    mtxlk(&taskwaitmtx.lk);
    if (ptr) {
        tab->nref--;
        ptab[0] = tab;
        tab = ((void **)tab)[key0];
        if (tab) {
            tab->nref--;
            ptab[1] = tab;
            tab = ((void **)tab)[key1];
            if (tab) {
                tab->nref--;
                ptab[2] = tab;
                tab = ((void **)tab)[key2];
                if (tab) {
                    tab->nref--;
                    queue = ((void **)tab)[key3];
                }
                if (queue) {
                    queue->nref--;
                }
#if 0
                if (tab) {
                    tab = ((void **)tab)[key3];
                }
#endif
            }
        }
//        task1 = tab->ptr;
        task1 = queue->next;
        if (task1) {
            if (task1->next) {
                task1->next->prev = NULL;
            }
            queue->next = task1->next;
            prio = task1->prio;
            taskqptr = &taskruntab[prio];
            mtxlk(&taskrunmtxtab[prio].lk);
            taskqueue(task1, taskqptr);
            mtxunlk(&taskrunmtxtab[prio].lk);
        }
        while (task1) {
            prio = taskwakeprio(task1);
            task2 = task1->next;
            taskqptr = &taskruntab[prio];
            mtxlk(&taskrunmtxtab[prio].lk);
            taskqueue(task1, taskqptr);
            mtxunlk(&taskrunmtxtab[prio].lk);
            task1 = task2;
        }
        /* TODO: free tables if possible */
        tab = ptab[0];
        if (tab) {
            if (!--tab->nref) {
                pptr = (void **)&taskwaittab[key0];
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

/* switch tasks */
FASTCALL
struct task *
taskpick(void)
{
    struct task  *task = k_curtask;
    struct task **taskqptr;
    long          prio = task->prio;
    long          sched = task->sched;
    long          state = task->state;

    if (state == TASKREADY) {
        if (sched == SCHEDRT) {
            if (prio < 0) {
                /* SCHED_FIFO */
                prio = -prio;
                taskqptr = &taskruntab[prio];
                mtxlk(&taskrunmtxtab[prio].lk);
                taskpush(task, taskqptr);
                mtxunlk(&taskrunmtxtab[prio].lk);
            } else {
                /* SCHED_RR */
                taskqptr = &taskruntab[prio];
                mtxlk(&taskrunmtxtab[prio].lk);
                taskqueue(task, taskqptr);
                mtxunlk(&taskrunmtxtab[prio].lk);
            }
        } else {
            if (sched != SCHEDFIXED) {
                prio = taskadjprio(task);
            }
            taskqptr = &taskruntab[prio];
            mtxlk(&taskrunmtxtab[prio].lk);
            taskqueue(task, taskqptr);
            mtxunlk(&taskrunmtxtab[prio].lk);
        }
    } else if (state == TASKWAIT) {
        taskaddwait(task);
    }
    task = NULL;
    while (!task) {
        for (prio = 0 ; prio < SCHEDNCLASS * SCHEDNPRIO ; prio++) {
            mtxlk(&taskrunmtxtab[prio].lk);
            taskqptr = &taskruntab[prio];
            if (taskqptr) {
                task = taskpop(taskqptr);

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
taskpopid(struct taskid *taskidqueue)
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
        listinit(taskid);
        taskidqueue = taskid;
    }
    mtxunlk(&taskidmtx.lk);
}

void
taskinitids(void)
{
    long           id = PROCNPREDEF;
    struct taskid *taskid;

    mtxlk(&taskidmtx.lk);
    taskid = taskidqueue;
    taskid->id = id;
    listinit(taskid);
    taskidqueue = taskid;
    for (id = 1 ; id < NTASK ; id++) {
        taskid = taskidtab[id];
        taskid->id = id;
        listaddafter(taskid, taskidqueue->prev);
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

