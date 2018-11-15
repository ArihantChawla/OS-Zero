#include <zero/cdefs.h>
#include <mach/param.h>
#include <mt/mtx.h>
#include <kern/cpu.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/unit/ia32/task.h>

#define DEQ_SINGLE_TYPE
#define DEQ_TYPE struct taskid
#include <zero/deq.h>

struct task          k_tasktab[TASKSMAX] ALIGNED(PAGESIZE);
static struct taskid k_taskidtab[TASKSMAX];
static struct taskid k_taskidqueue;

void
taskinitids(void)
{
    struct taskid *queue = &k_taskidqueue;
    struct taskid *taskid;
    long           id;

    fmtxlk(&queue->lk);
    for (id = TASKPREDEFS ; id < TASKSMAX ; id++) {
        taskid = &k_taskidtab[id];
        taskid->id = id;
        deqappend(taskid, &queue);
    }
    fmtxunlk(&queue->lk);

    return;
}

void
taskinit(struct task *task, long unit)
{
    long id = (task) ? task->id : TASKKERN;

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
    struct taskid *queue = &k_taskidqueue;
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
    struct taskid *queue = &k_taskidqueue;
    struct taskid *taskid = &k_taskidtab[id];

    fmtxlk(&queue->lk);
    deqappend(taskid, &queue);
    fmtxunlk(&queue->lk);

    return;
}

