#include <stdint.h>
#include <zero/asm.h>
#include <sys/io.h>
#include <kern/conf.h>
#include <kern/util.h>
#include <kern/unit/x86/asm.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/pit.h>
#include <kern/unit/x86/pic.h>

extern uint64_t  kernidt[];
extern void     *irqvec[];
extern void      irqtimer(void);
volatile long    irqtimerfired;

void
pitinit(void)
{
    uint64_t *idt = kernidt;

    kprintf("initialising timer interrupt to %d Hz\n", HZ);
    trapsetintgate(&idt[trapirqid(IRQTIMER)], irqtimer, TRAPUSER);
    /* initialise timer */
    outb(PITCMD, PITCTRL);
    pitsethz(HZ);

    return;
}

/*
 * sleep for msec milliseconds, then call trigger func
 * only to be used before the scheduler is enabled
 */
void
pitsleep(long msec, void (*func)(void))
{
    long hz = 1000L / msec;

    /* enable timer interrupt, disable other interrupts */
    outb(~0x01, PICMASK1);
    outb(~0x00, PICMASK2);
    irqtimerfired = 0;
    irqvec[IRQTIMER] = func;
    outb(PITDUALBYTE | PITONESHOT, PITCTRL);
    pitsethz(hz);
    while (!irqtimerfired) {
        k_waitint();
    }
    /* enable all interrupts */
    outb(0x00, PICMASK1);
    outb(0x00, PICMASK2);

    return;
}

