#include <kern/conf.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/mtx.h>
#include <kern/proc/task.h>
#include <kern/unit/x86/asm.h>
#if (!APIC)
#include <sys/io.h>
#include <kern/unit/x86/pic.h>
#endif
#include <kern/unit/ia32/task.h>

FASTCALL void (*schedpicktask)(struct task *);
#if (ZEROSCHED)
FASTCALL void   taskpick(struct task *task);
#endif

void
schedinit(void)
{
#if (ZEROSCHED)
    schedpicktask = taskpick;
#else
#error define supported scheduler such as ZEROSCHED
#endif

    return;
}

void
schedyield(void)
{
    struct task *oldtask = k_curtask;
    struct task *task = NULL;

    do {
        schedpicktask(oldtask);
    } while (!task);
    if (task != oldtask) {
        m_tcbjmp(&task->m_tcb);
    } else {

        return;
    }

    /* NOTREACHED */
    for ( ; ; ) { ; }
}

