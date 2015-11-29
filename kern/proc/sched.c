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
        m_tcbjmp(task);
    } else {

        return;
    }

    /* NOTREACHED */
    for ( ; ; ) { ; }
}

NOINLINE
void
schedloop(void)
{
    /* test scheduler loop; interrupted by timer [and other] interrupts */
    do {
        /* enable all interrupts */
#if !(APIC)
        outb(0x00, PICMASK1);
        outb(0x00, PICMASK2);
#endif
//        kprintregs();
        /* wait for interrupt */
        k_waitint();
    } while (1);

    /* NOTREACHED */
    return;
}

