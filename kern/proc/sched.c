#include <kern/conf.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/fastidiv.h>
#include <zero/mtx.h>
#include <kern/sched.h>
#include <kern/proc/task.h>
#include <kern/unit/x86/asm.h>
#if (!APIC)
#include <sys/io.h>
#include <kern/unit/x86/pic.h>
#endif
#include <kern/unit/ia32/task.h>

//FASTCALL struct task        *(*schedswtchtask)(struct task *);
#if (ZEROSCHED)
extern FASTCALL struct task   *taskswtch(struct task *task);
#endif

/* lookup table for fast division with multiplication and shift */
struct divul scheddivultab[SCHEDHISTORYSIZE];

void
schedinit(void)
{
#if 0
#if (ZEROSCHED)
    schedswtchtask = taskswtch;
#else
#error define supported scheduler such as ZEROSCHED
#endif
#endif
    fastuldiv32gentab(scheddivultab, SCHEDHISTORYSIZE);

    return;
}

static __inline__ void
schedadjcpupct(struct task *task, long run)
{
    long     tick = k_curcpu->ntick;
    unsigned last = task->lastrun;
    long     diff = tick - last;
    long     delta;
    long     ntick;
    long     val;
    long     div;

    if (diff >= SCHEDHISTORYNTICK) {
        task->ntick = 0;
        task->firstrun = tick - SCHEDHISTORYNTICK;
    } else {
        unsigned long first = task->firstrun;

        delta = tick - first;
        if (delta >= SCHEDHISTORYSIZE) {
            ntick = task->ntick;
            div = last - first;
            val = tick - SCHEDHISTORYNTICK;
            last -= val;
            ntick = fastuldiv32(ntick, div, scheddivultab);
            ntick *= last;
            task->firstrun = val;
        }
    }
    if (run) {
        ntick = diff >> SCHEDTICKSHIFT;
        task->ntick = ntick;
    }
    task->lastrun = tick;

    return;
}

void
schedyield(void)
{
    struct task *oldtask = k_curtask;
    struct task *task = NULL;

    task = taskswtch(oldtask);
    if (task != oldtask) {
        m_taskjmp(&task->m_task);
    } else {

        return;
    }

    /* NOTREACHED */
    for ( ; ; ) { ; }
}

