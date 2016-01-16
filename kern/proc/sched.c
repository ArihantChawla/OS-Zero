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

FASTCALL struct task        *(*schedswtchtask)(struct task *);
#if (ZEROSCHED)
extern FASTCALL struct task   *taskswtch(struct task *task);
#endif

/* lookup table for fast division with multiplication and shift */
struct divul scheddivultab[SCHEDHISTORYSIZE];

void
schedinit(void)
{
#if (ZEROSCHED)
    schedswtchtask = taskswtch;
#else
#error define supported scheduler such as ZEROSCHED
#endif
    fastuldiv32gentab(scheddivultab, SCHEDHISTORYSIZE);

    return;
}

void
schedyield(void)
{
    struct task *oldtask = k_curtask;
    struct task *task = NULL;

    task = schedswtchtask(oldtask);
    if (task != oldtask) {
        m_taskjmp(&task->m_task);
    } else {

        return;
    }

    /* NOTREACHED */
    for ( ; ; ) { ; }
}

