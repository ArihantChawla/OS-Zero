#include <time.h>
#include <kern/sched.h>
#include <kern/proc/task.h>

static struct task *k_sleepqueue;

/* FIXME: add a multilevel tree for sleeping tasks for speed...? */
/* FIXME: add a lookup table for tasks that have called nanosleep() */
void
schedsetsleep(struct task *task)
{
    time_t       timelim = task->timelim;
    struct task *queue = k_sleepqueue;
    struct task *sleeptask;

    if (task->waitchan) {
        schedsetwait(task);
    } else {
        sleeptask = k_sleepqueue;
        if (sleeptask) {
            while ((sleeptask) && (sleeptask->next)) {
                if (task->timelim < sleeptask->timelim) {
                    task->prev = sleeptask->prev;
                    task->next = sleeptask;
                    sleeptask->prev = task;

                    return;
                }
                sleeptask = sleeptask->next;
            }
            task->prev = sleeptask;
            task->next = NULL;
            sleeptask->next = task;
        } else {
            task->prev = NULL;
            task->next = NULL;
            k_sleepqueue = task;
        }
    }

    return;
}

