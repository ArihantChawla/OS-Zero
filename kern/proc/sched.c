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

NORETURN
void
schedyield(void)
{
    struct task *task = k_curtask;

    schedpicktask(task);

    /* NOTREACHED */
    for ( ; ; ) { ; }
}

NOINLINE
void
schedloop(void)
{
    /* scheduler loop; interrupted by timer [and other] interrupts */
    do {
        /* enable all interrupts */
#if !(APIC)
        outb(0x00, PICMASK1);
        outb(0x00, PICMASK2);
#endif
        /* wait for interrupt */
        k_waitint();
    } while (1);

    /* NOTREACHED */
    return;
}

