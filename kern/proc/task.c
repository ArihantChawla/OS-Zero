#include <stdint.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <mt/mtx.h>
#include <kern/cpu.h>
#include <kern/proc/proc.h>
#include <kern/proc/task.h>
#include <kern/unit/ia32/task.h>

#if 0
#define DEQ_SINGLE_TYPE
#define DEQ_TYPE struct taskid
#include <zero/deq.h>
#endif

struct task         *k_tasktab[TASKSMAX] ALIGNED(PAGESIZE);
taskid_t             k_taskidstk[TASKSMAX];
volatile m_atomic_t  k_taskidndx;
struct task         *k_taskbuf;
volatile m_atomic_t  k_tasknbuf;

void
taskinitids(void)
{
    taskid id;

    for (id = TASKPREDEFS ; id < TASKSMAX ; id++) {
        k_taskidstk[id] = id;
    }
    k_taskidndx = TASKPREDEFS;
}

struct task *
taskget(void)
{
    struct task *task;
    struct task *head;
    struct task *cur;
    struct task *prev;
    long         ndx;
    uintptr_t    uptr;

    m_lkbit((m_atomic_t *)&k_taskbuf, TASK_LK_BIT_POS);
    uptr = k_taskbuf;
    uptr &= ~TASK_LK_BIT;
    task = (void *)uptr;
    if (uptr) {
        m_atomwrite((m_atomic_t *)&k_taskbuf, task->next);
    } else {
        task = kmalloc(TASKBUFITEMS * TASKALLOCSIZE);
        if (task) {
            cur = task;
            cur++;
            prev = NULL;
            head = cur;
            for (ndx = 1 ; ndx < TASKBUFITEMS ; ndx++) {
                cur->prev = prev;
                if (prev) {
                    prev->next = cur;
                }
                prev = cur;
                cur = cur->next;
            }
            m_lkbit((m_atomic_t *)&k_taskbuf, TASK_LK_BIT_POS);
            uptr = k_taskbuf;
            uptr &= ~TASK_LK_BIT;
            cur->next = (struct task *)uptr;
            m_atomwrite((m_atomic_t *)&k_taskbuf, head);
        }
    }

    return task;
}

void
taskinit(struct task *task, long unit)
{
    long id = (task) ? task->id : TASKKERN;

    taskinittls(unit, id);
    task = taskget();
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

taskid_t
taskgetid(void)
{
    long     ndx = m_fetchadd(k_taskidndx, 1);
    taskid_t id;

    if (ndx < TASKSMAX) {
        id = k_taskidstk[ndx];
    } else {
        m_fetchadd(k_taskidndx, -1);

        return -1;
    }

    return id;
}

void
taskputid(taskid_t id)
{
    long ndx = m_fetchadd(k_taskidndx, -1);

    IF (ndx >= 0) {
        k_taskidstk[ndx] = id;
    }

    return;
}

