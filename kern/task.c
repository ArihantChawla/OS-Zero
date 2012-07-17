#include <zero/mtx.h>
#include <kern/task.h>

static struct pid    pidtab[NTHR] ALIGNED(PAGESIZE);
static struct pidq   pidq;
static volatile long pidlk;

long
taskgetpid(void)
{
    struct pid *pid;
    long        retval;

    mtxlk(&pidlk, INITPID);
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
    mtxunlk(&pidlk, INITPID);

    return retval;
}

void
taskfreepid(long id)
{
    struct pid *pid;
    mtxlk(&pidlk, INITPID);
    pid = &pidtab[id];
    pid->prev = pidq.tail;
    if (pidq.tail) {
        pidq.tail->next = pid;
    } else {
        pidq.head = pid;
    }
    pidq.tail = pid;
    mtxunlk(&pidlk, INITPID);
}

void
taskinit(void)
{
    int         i;
    struct pid *pid;

    mtxlk(&pidlk, INITPID);
    pid = &pidtab[0];
    pidq.head = pidq.tail = pid;
    for (i = 0 ; i < NTHR ; i++) {
        pid = &pidtab[i];
        pid->id = i;
        pid->prev = pidq.tail;
        pidq.tail->next = pid;
        pidq.tail = pid;
    }
    mtxunlk(&pidlk, INITPID);

    return;
}

