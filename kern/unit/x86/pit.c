#include <stddef.h>
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
extern void      irqtmr(void);
//volatile long    irqtmrfired;

void
pitinit(void)
{
    uint64_t *idt = kernidt;

    kprintf("initialising timer interrupt to %d Hz\n", HZ);
    trapsetintgate(&idt[trapirqid(IRQTMR)], irqtmr, TRAPUSER);
    /* initialise timer */
    outb(PITCMD, PITCTRL);
    pitsethz(HZ);

    return;
}

/*
 * sleep for msec milliseconds, then call trigger func
 * only to be used before the APIC timers and scheduler are enabled
 */
void
pitsleep(long msec)
{
    long hz = 1000L / msec;

    /* enable timer interrupt, disable other interrupts */
    outb(~0x01, PICMASK1);
    outb(~0x00, PICMASK2);
//  irqtmrfired = 0;
//    irqvec[IRQTMR] = func;
    irqvec[IRQTMR] = NULL;
    outb(PITDUALBYTE | PITONESHOT, PITCTRL);
    pitsethz(hz);
//    while (!irqtmrfired) {
        k_waitint();
//    }
    /* enable all interrupts */
    outb(0x00, PICMASK1);
    outb(0x00, PICMASK2);

    return;
}

