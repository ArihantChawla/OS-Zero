#include <kern/conf.h>
#include <sys/io.h>
#include <zero/cdefs.h>
#include <kern/asm.h>
#include <kern/unit/x86/pic.h>

NOINLINE
void
schedloop(void)
{
    /* test scheduler loop; interrupted by timer [and other] interrupts */
    do {
#if (!APIC)
        /* set masks to zero; enable all interrupts */
        outb(0x00, PICMASK1);
        outb(0x00, PICMASK2);
#endif
        /* wait for interrupt */
        k_waitint();
    } while (1);

    /* NOTREACHED */
    return;
}

