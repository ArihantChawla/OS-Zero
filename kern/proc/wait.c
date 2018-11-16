#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/hash.h>
#include <mach/param.h>
#include <kern/sched.h>
#include <kern/proc/task.h>

struct task *k_taskwaithash[TASKWAITHASHITEMS] ALIGNED(PAGESIZE);

/* add task to wait table */
void
schedsetwait(struct task *task)
{
    uintptr_t chan = task->waitchan;
    uintptr_t uptr;
    long      key = tmhash32((uint32_t)chan);

    key &= (TASKWAITHASHITEMS - 1);
    m_lkbit((m_atomic_t *)&k_taskwaithash[key], TASK_LK_BIT_POS);
    uptr = k_taskwaithash[key];
    uptr &= ~TASK_LK_BIT;
    task->next = (struct task *)uptr;
    if (uptr) {
        ((struct task *)uptr)->prev = task;
    }
    m_atomwrite((m_atomic_t *)&k_taskwaithash[key], task);

    return;
}

/* wake up tasks waiting on chan */
void
schedwakeup(uintptr_t chan)
{
    return;
}

