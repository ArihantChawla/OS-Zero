#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <kern/proc/task.h>

#define DEQ_SINGLE_TYPE
#define DEQ_TYPE struct taskid
#include <zero/deq.h>

extern struct cpu    cputab[NCPU];
struct task          tasktab[NTASK] ALIGNED(PAGESIZE);
static struct taskid taskidtab[NTASK];
static struct taskid taskidqueue;

void
taskinitids(void)
{
    struct taskid *queue = &taskidqueue;
    struct taskid *taskid;
    long           id;

    fmtxlk(&queue->lk);
    for (id = TASKNPREDEF ; id < NTASK ; id++) {
        taskid = &taskidtab[id];
        taskid->id = id;
        deqappend(taskid, &queue);
    }
    fmtxunlk(&queue->lk);

    return;
}

void
taskinittls(long unit, long id)
{
    struct cpu  *cpu = &cputab[unit];
    struct task *task = &tasktab[id];

    k_setcurcpu(cpu);
    k_setcurunit(unit);
    k_setcurtask(task);
    k_setcurpid(id);
#if 0
    k_curcpu = (volatile struct cpu *)cpu;
    k_curunit = (volatile long)unit;
    k_curtask = (volatile struct task *)task;
    k_curpid = (volatile long)id;
#endif

    return;
}

void
taskinit(struct task *task, long unit)
{
    long id = (task) ? task->id : PROCKERN;

    taskinittls(unit, id);
    if (!task) {
        ;
    }

    return;
}

void
taskinitenv(void)
{
    taskinitids();

    return;
}

long
taskgetid(void)
{
    struct taskid *queue = &taskidqueue;
    struct taskid *taskid;
    long           retval = -1;

    fmtxlk(&queue->lk);
    taskid = deqpop(&queue);
    if (taskid) {
        retval = taskid->id;
    }
    fmtxunlk(&taskid->lk);

    return retval;
}

void
taskfreeid(long id)
{
    struct taskid *queue = &taskidqueue;
    struct taskid *taskid = &taskidtab[id];

    fmtxlk(&queue->lk);
    deqappend(taskid, &queue);
    fmtxunlk(&queue->lk);

    return;
}

