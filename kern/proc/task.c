#include <zero/cdefs.h>
#include <mach/param.h>
#include <mt/mtx.h>
#include <kern/cpu.h>
#include <kern/proc/task.h>

#define DEQ_SINGLE_TYPE
#define DEQ_TYPE struct taskid
#include <zero/deq.h>

struct task          k_tasktab[NTASK] ALIGNED(PAGESIZE);
static struct taskid k_taskidtab[NTASK];
static struct taskid k_taskidqueue;

void
taskinitids(void)
{
    struct taskid *queue = &k_taskidqueue;
    struct taskid *taskid;
    long           id;

    fmtxlk(&queue->lk);
    for (id = TASKNPREDEF ; id < NTASK ; id++) {
        taskid = &k_taskidtab[id];
        taskid->id = id;
        deqappend(taskid, &queue);
    }
    fmtxunlk(&queue->lk);

    return;
}

void
taskinittls(long unit, long id)
{
    volatile struct cpu *cpu = &k_cputab[unit];
    struct task         *task = &k_tasktab[id];

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

