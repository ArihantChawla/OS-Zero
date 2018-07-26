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
        /* wait for interrupt */
        k_waitint();
    } while (1);

    /* NOTREACHED */
    return;
}

