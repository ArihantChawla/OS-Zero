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
extern void      irqtmr0(void);
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

void
pitsleep(long msec)
{
    long n = 1000L * msec;

    while (n--) {
//        iodelay();
        ;
    }

    return;
}

#if 0
/*
 * sleep for msec milliseconds, then call trigger func
 * only to be used before the APIC timers and scheduler are enabled
 */
void
pitsleep(long msec)
{
    uint64_t *idt = kernidt;
    long       hz = 1000L / msec;

    /* enable timer interrupt, disable other interrupts */
    outb(~0x01, PICMASK1);
    outb(~0x00, PICMASK2);
    irqvec[IRQTMR] = NULL;
//    trapsetintgate(&idt[trapirqid(IRQTMR)], irqtmr0, TRAPUSER);
    outb(PITDUALBYTE | PITONESHOT, PITCTRL);
    pitsethz(hz);
    k_waitint();
    /* enable all interrupts */
    outb(0x00, PICMASK1);
    outb(0x00, PICMASK2);

    return;
}
#endif

