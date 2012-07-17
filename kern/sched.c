#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <kern/sched.h>

extern struct thr *runqueuetab[];
extern volatile long runqueuelktab[];

void
thrqueue(struct thr *thr)
{
    long prio = thr->prio;

    thrlkrunq(prio);
    thr->next = runqueuetab[prio];
    runqueuetab[prio] = thr;
    thrunlkrunq(prio);

    return;
}

long
thradjprio(struct thr *thr)
{
    long class;
    long prio;
    long retval;

    class = thr->class;
    if (class != THRRT) {
        prio = ++thr->prio & (THRNPRIO - 1);   // wrap around
        thr->prio = prio;
        retval = class * NPRIO + prio;
    } else {
        retval = thr->prio;
    }

    return retval;
}

void
thryield(void)
{
    struct thr   *thr;
    long          prio;

    thrsave(curthr);
    thr = NULL;
    thradjprio(curthr);
    thrqueue(curthr);
    for (prio = 0 ; prio < NPRIO ; prio++) {
        thrlkrunq(prio);
        thr = runqueuetab[prio];
        if (thr) {
            runqueuetab[prio] = thr->next;
            thrunlkrunq(prio);

            break;
        }
        thrunlkrunq(prio);
    }
    if ((thr) && thr != curthr) {
        thrjmp(thr);
    }

    /* fall back to running the earlier thread */
    return;
}

