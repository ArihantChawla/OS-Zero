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

static struct tasklk    taskwaitmtxtab[NLVL0TASK] ALIGNED(PAGESIZE);
static struct tasktab  *taskwaittab[NLVL0TASK];
static struct taskid    taskidtab[NTASK];
static struct task     *taskstoppedtab[NTASK];
static struct task     *taskzombietab[NTASK];
//static struct task taskruntab[SCHEDNCLASS * SCHEDNPRIO];
static struct tasklk    taskrunmtxtab[SCHEDNCLASS * SCHEDNPRIO] ALIGNED(PAGESIZE);
static struct task     *taskruntab[SCHEDNCLASS * SCHEDNPRIO];
//static struct task taskrtqueue;
//extern long             trappriotab[NINTR];
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

void taskqueueready(struct task *task);
void taskaddwait(struct task *task);
void taskaddsleep(struct task *task);
void taskaddstopped(struct task *task);
void taskaddzombie(struct task *task);

typedef void taskfunc_t(struct task *);
taskfunc_t             *taskfunctab[TASKNSTATE]
= {
    NULL,               // TASKNEW
    taskqueueready,
    taskaddwait,
    taskaddsleep,
    taskaddstopped,
    taskaddzombie
};
static long            *taskniceptr = &tasknicetab[32];
//static struct tasklk    taskwaitmtx ALIGNED(CLSIZE);
static struct tasklk    taskidmtx ALIGNED(CLSIZE);
static struct taskid   *taskidqueue;
static struct task     *tasksleepqueue;

/* save task context */
FASTCALL
void
tasksave(struct task *task)
{
    uint8_t *fctx = task->m_tcb.fctx;

    if (k_curcpu->info->flags & CPUHASFXSR) {
        __asm__ __volatile__ ("fxsave (%0)\n" : : "r" (fctx));
    } else {
        __asm__ __volatile__ ("fnsave (%0)\n" : : "r" (fctx));
    }

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
    k_curpid = task->id;
    m_tcbjmp(&task->m_tcb);

    /* NOTREACHED */
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
        queue->prev->next = task;
        task->prev = queue->prev;
        task->next = queue;
        queue->prev = task;
    } else {
        listinit(task);
        *taskqueue = task;
    }

    return;
}

/* get/remove task from beginning of queue */
struct task *
taskpop(struct task **taskqueue)
{
    struct task *task = *taskqueue;

    if (task) {
        if (!listissingular(task)) {
            task->prev->next = task->next;
            task->next->prev = task->prev;
            *taskqueue = task->next;
        } else {
            *taskqueue = NULL;
        }
    }

    return task;
}

/* add task to end of queue */
void
taskqueue(struct task *task, struct task **taskqueue)
{
    struct task *queue = *taskqueue;

    if (queue) {
        queue->prev->next = task;
        task->prev = queue->prev;
        task->next = queue;
        queue->prev = task;
    } else {
        listinit(task);
        *taskqueue = task;
    }

    return;
}

/* add task to wait queue */
void
taskaddwait(struct task *task)
{
    struct tasktab    *tab;
    void              *ptr = NULL;
    void             **pptr;
    struct taskqueue  *qptr;
    struct taskqueue  *queue = NULL;
    uintptr_t          wchan = task->wchan;
    long               fail = 0;    
    long               key0;
    long               key1;
    long               key2;
    long               key3;
    void              *ptab[TASKNKEY - 1] = { NULL, NULL, NULL };

    key0 = taskwaitkey0(wchan);
    key1 = taskwaitkey1(wchan);
    key2 = taskwaitkey2(wchan);
    key3 = taskwaitkey3(wchan);
    ptr = taskwaittab[key0];
    pptr = ptr;
    mtxlk(&taskwaitmtxtab[key0].lk);
    if (!ptr) {
        ptr = kmalloc(NLVL1TASK * sizeof(struct tasktab));
        if (ptr) {
            kbzero(ptr, NLVL1TASK * sizeof(struct tasktab));
        }
        ptab[0] = ptr;
        pptr = ptr;
        taskwaittab[key0] = ptr;
    }
    if (ptr) {
        ptr = pptr[key1];
        if (!ptr) {
            ptr = kmalloc(NLVL2TASK * sizeof(struct tasktab));
            if (ptr) {
                kbzero(ptr, NLVL2TASK * sizeof(struct tasktab));
            }
        }
        ptab[1] = ptr;
        pptr[key1] = ptr;
        pptr = ptr;
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = pptr[key2];
        if (!ptr) {
            qptr = kmalloc(NLVL3TASK * sizeof(struct taskqueue));
            if (qptr) {
                kbzero(qptr, NLVL3TASK * sizeof(struct taskqueue));
            } 
            ptab[2] = qptr;
            pptr[key2] = qptr;
        } else {
            qptr = pptr[key2];
        }
    } else {
        fail = 1;
    }
    if (!fail) {
        queue = &qptr[key3];
        task->prev = NULL;
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
    }
    mtxunlk(&taskwaitmtxtab[key0].lk);
    
    return;
}

void
taskaddsleep(struct task *task)
{
    time_t       waketm = task->waketm;
    struct task *sleeptask = tasksleepqueue;

    if (!sleeptask) {
        listinit(task);
        tasksleepqueue = task;
    } else {
        while ((sleeptask) && (sleeptask->next)) {
            if (task->waketm < sleeptask->waketm) {
                task->next = sleeptask;
                task->prev = sleeptask->prev;
                sleeptask->prev = task;

                break;
            }
            sleeptask = sleeptask->next;
        }
        if (task->waketm < sleeptask->waketm) {
            task->next = sleeptask;
            task->prev = sleeptask->prev;
            sleeptask->prev = task;
        } else {
            task->prev = sleeptask;
            sleeptask->next = task;
            task->next = NULL;
        }
    }

    return;
}

void
taskaddstopped(struct task *task)
{
    long id = task->id;

    taskstoppedtab[id] = task;

    return;
}

void
taskaddzombie(struct task *task)
{
    long id = task->id;

    taskzombietab[id] = task;

    return;
}

/* move a task from wait queue to ready queue */
void
taskunwait(uintptr_t wchan)
{
    struct tasktab    *tab;
    void              *ptr = NULL;
    struct taskqueue  *queue;
    struct task      **taskqptr;
    struct task       *task1;
    struct task       *task2;
    long               key0 = taskwaitkey0(wchan);
    long               key1 = taskwaitkey1(wchan);
    long               key2 = taskwaitkey2(wchan);
    long               key3 = taskwaitkey3(wchan);
    long               prio;
    void             **pptr;
    void              *ptab[TASKNKEY - 1] = { NULL, NULL, NULL };
    void             **pptab[TASKNKEY - 1] = { NULL, NULL, NULL };

    tab = taskwaittab[key0];
    mtxlk(&taskwaitmtxtab[key0].lk);
    if (ptr) {
        tab->nref--;
        ptab[0] = tab;
        pptab[0] = &taskwaittab[key0];
        tab = ((void **)tab)[key1];
        if (tab) {
            tab->nref--;
            ptab[1] = tab;
            pptab[1] = &tab[key0];
            tab = ((void **)tab)[key2];
            if (tab) {
                tab->nref--;
                ptab[2] = tab;
                pptab[2] = &tab[key2];
                queue = ((void **)tab)[key3];
            }
        }
        task1 = queue->next;
        while (task1) {
            if (task1->next) {
                task1->next->prev = NULL;
            }
            queue->next = task1->next;
            task2 = task1->next;
            prio = taskwakeprio(task1);
            mtxlk(&taskrunmtxtab[prio].lk);
            taskqptr = &taskruntab[prio];
            taskqueue(task1, taskqptr);
            mtxunlk(&taskrunmtxtab[prio].lk);
            task1 = task2;
        }
        tab = ptab[0];
        if (tab) {
            if (!tab->nref) {
                pptr = pptab[0];
                kfree(tab);
                *pptr = NULL;
            }
            tab = ptab[1];
            if (tab) {
                if (!tab->nref) {
                    pptr = pptab[1];
                    kfree(tab);
                    *pptr = NULL;
                }
                tab = ptab[2];
                if (tab) {
                    if (!tab->nref) {
                        pptr = pptab[2];
                        kfree(tab);
                        *pptr = NULL;
                    }
                }
            }
        }
    }
    mtxunlk(&taskwaitmtxtab[key0].lk);
}

void
taskqueueready(struct task *task)
{
    long          prio = task->prio;
    long          sched = task->sched;
    long          state = task->state;
    struct task **taskqptr;

    if (sched == SCHEDRT) {
        if (prio < 0) {
            /* SCHED_FIFO */
            prio = -prio;
            mtxlk(&taskrunmtxtab[prio].lk);
            taskqptr = &taskruntab[prio];
            taskpush(task, taskqptr);
            mtxunlk(&taskrunmtxtab[prio].lk);
        } else {
            /* SCHED_RR */
            mtxlk(&taskrunmtxtab[prio].lk);
            taskqptr = &taskruntab[prio];
            taskqueue(task, taskqptr);
            mtxunlk(&taskrunmtxtab[prio].lk);
        }
    } else {
        if (sched != SCHEDFIXED) {
            prio = taskadjprio(task);
        }
        mtxlk(&taskrunmtxtab[prio].lk);
        taskqptr = &taskruntab[prio];
        taskqueue(task, taskqptr);
        mtxunlk(&taskrunmtxtab[prio].lk);
    }

    return;
}

/* switch tasks */
FASTCALL
struct task *
taskpick(struct task *curtask)
{
    struct task  *task = NULL;
    struct task **taskqptr;
    long          prio = curtask->prio;
    long          sched = curtask->sched;
    long          state = curtask->state;
    taskfunc_t   *func = taskfunctab[state];

    if (curtask) {
        tasksave(curtask);
        func(curtask);
    }
    while (!task) {
        for (prio = 0 ; prio < SCHEDNCLASS * SCHEDNPRIO ; prio++) {
            mtxlk(&taskrunmtxtab[prio].lk);
            taskqptr = &taskruntab[prio];
            if (*taskqptr) {
                task = taskpop(taskqptr);
                mtxunlk(&taskrunmtxtab[prio].lk);

                return task;
            }
            mtxunlk(&taskrunmtxtab[prio].lk);
        }
        m_waitint();
    }

    return task;
}

/* get/remove task ID from beginning of queue */
struct taskid *
taskpopid(struct taskid **queue)
{
    struct taskid *taskid;

    taskid = *queue;
    if (taskid) {
        if (!listissingular(taskid)) {
            taskid->prev->next = taskid->next;
            taskid->next->prev = taskid->prev;
            *queue = taskid->next;
        } else {
            *queue = NULL;
        }
    }

    return taskid;
}

void
taskpushid(struct taskid *taskid, struct taskid **queue)
{
    struct taskid *idqueue = *queue;
    
    if (idqueue) {
        taskid->next = idqueue;
        taskid->prev = idqueue->prev;
        idqueue->prev = taskid;
    } else {
        listinit(taskid);
    }
    *queue = taskid;

    return;
}

long
taskgetid(void)
{
    struct taskid *taskid;
    long           retval = -1;

    mtxlk(&taskidmtx.lk);
    taskid = taskpopid(&taskidqueue);
    if (taskid) {
        retval = taskid->id;
    }
    mtxunlk(&taskidmtx.lk);

    return retval;
}

void
taskfreeid(long id)
{
    struct taskid *taskid = &taskidtab[id];
    
    mtxlk(&taskidmtx.lk);
    taskpushid(taskid, &taskidqueue);
    mtxunlk(&taskidmtx.lk);

    return;
}

void
taskinitids(void)
{
    long           id;
    struct taskid *taskid;

    mtxlk(&taskidmtx.lk);
    for (id = NTASK - 1 ; id >= TASKNPREDEF ; id--) {
        taskid = &taskidtab[id];
        taskid->id = id;
        taskpushid(taskid, &taskidqueue);
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

