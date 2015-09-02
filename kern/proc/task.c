#include <limits.h>
#include <zero/mtx.h>
#include <kern/proc/task.h>

static struct pid    pidtab[NTHR] ALIGNED(PAGESIZE);
static struct pidq   pidq;
static volatile long pidlk;

long
taskgetpid(void)
{
    struct pid *pid;
    long        retval;

    mtxlk(&pidlk);
    pid = pidq.head;
    if (!pid) {
        retval = -1;
    } else {
        if (pid->next) {
            pid->next->prev = NULL;
        }
        pidq.head = pid->next;
        retval = pid->id;
    }
    mtxunlk(&pidlk);

    return retval;
}

void
taskfreepid(long id)
{
    struct pid *pid;
    mtxlk(&pidlk);
    pid = &pidtab[id];
    pid->prev = pidq.tail;
    if (pidq.tail) {
        pidq.tail->next = pid;
    } else {
        pidq.head = pid;
    }
    pidq.tail = pid;
    mtxunlk(&pidlk);
}

void
taskinitpids(void)
{
    int         id = PROCNPREDEF;
    struct pid *pid;

    mtxlk(&pidlk);
    pid = &pidtab[id];
    pid->id = id;
    pidq.head = pidq.tail = pid;
    for (id = 1 ; id < NTHR ; id++) {
        pid = &pidtab[id];
        pid->id = id;
        pid->prev = pidq.tail;
        pidq.tail->next = pid;
        pidq.tail = pid;
    }
    mtxunlk(&pidlk);

    return;
}

void
taskinit(void)
{
    taskinitpids();

    return;
}

