#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>

static struct taskid taskidtab[NTASK] ALIGNED(PAGESIZE);
static struct taskid taskidqueue;

#define QUEUE_SINGLE_TYPE
#define QUEUE_TYPE struct taskid
#include <zero/queue.h>

long
taskgetid(void)
{
    struct taskid *queue = &taskidqueue;
    struct taskid *taskid;
    long           retval = -1;

    mtxlk(&queue->lk);
    taskid = queuepop(&queue);
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
    queueappend(taskid, &queue);
    mtxunlk(&queue->lk);

    return;
}

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
        queueappend(taskid, &queue);
    }
    mtxunlk(&queue->lk);

    return;
}

