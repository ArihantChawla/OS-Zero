#include <sys/io.h>
#include <zero/cdefs.h>
#include <kern/unit/x86/asm.h>

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

