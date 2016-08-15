#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <kern/proc/task.h>

#define DEQ_SINGLE_TYPE
#define DEQ_TYPE struct taskid
#include <zero/deq.h>

struct task          tasktab[NTASK] ALIGNED(PAGESIZE);
static struct taskid taskidtab[NTASK];
static struct taskid taskidqueue;

void
taskinitids(void)
{
    struct taskid *queue = &taskidqueue;
    struct taskid *taskid;
    long           id;

    mtxlk(&queue->lk);
    for (id = TASKNPREDEF ; id < NTASK ; id++) {
        taskid = &taskidtab[id];
        taskid->id = id;
        deqappend(taskid, &queue);
    }
    mtxunlk(&queue->lk);

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

    mtxlk(&queue->lk);
    taskid = deqpop(&queue);
    if (taskid) {
        retval = taskid->id;
    }
    mtxunlk(&taskid->lk);

    return retval;
}

void
taskfreeid(long id)
{
    struct taskid *queue = &taskidqueue;
    struct taskid *taskid = &taskidtab[id];
    
    mtxlk(&queue->lk);
    deqappend(taskid, &queue);
    mtxunlk(&queue->lk);

    return;
}

