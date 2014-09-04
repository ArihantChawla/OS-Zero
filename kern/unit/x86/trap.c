#include <stdint.h>
#define __KERNEL__ 1
#include <signal.h>
#include <sys/io.h>
#include <kern/conf.h>
#include <zero/param.h>
#include <zero/types.h>
#include <kern/unit/x86/asm.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/boot.h>

extern void picinit(void);
extern void idtset(void);

extern void trapde(void);
extern void trapdb(void);
extern void trapnmi(void);
extern void trapbp(void);
extern void trapof(void);
extern void trapbr(void);
extern void trapud(void);
extern void trapnm(void);
extern void trapdf(void);
extern void trapts(void);
extern void trapnp(void);
extern void trapss(void);
extern void trapgp(void);
extern void trappf(void);
extern void trapmf(void);
extern void trapac(void);
extern void trapmc(void);
extern void trapxf(void);

extern void irqtmr0(void);
extern void irqtmr(void);
extern void irqkbd(void);
extern void irqmouse(void);
#if (SMP)
extern void irqerror(void);
#endif
#if (APIC)
extern void irqspurious(void);
#endif

//extern void syscall(void);

#if (SMP)
extern volatile long   mpmultiproc;
#endif
extern uint64_t        kernidt[NINTR];
extern struct m_farptr idtptr;

long                   trappriotab[NINTR];
long                   trapsigmap[TRAPNCPU]
= {
    SIGFPE, 
    0,
    0,
    SIGTRAP,
    0,
    SIGBUS,
    SIGILL,
    SIGILL,
    0,
    0,
    0,
    SIGSEGV,
    SIGSTKFLT,
    SIGSEGV,
    0,
    0,
    SIGFPE,
    SIGBUS,
    SIGABRT,
    SIGFPE
};

#define trapsetprio(irq, prio)                                          \
    (trappriotab[(irq)] = (prio))

void
trapinitidt(uint64_t *idt)
{
    trapsetintgate(&idt[TRAPDE], trapde, TRAPSYS);
    trapsetintgate(&idt[TRAPDB], trapdb, TRAPSYS);
    trapsetintgate(&idt[TRAPNMI], trapnmi, TRAPSYS);
    trapsetintgate(&idt[TRAPBP], trapbp, TRAPSYS);
    trapsetintgate(&idt[TRAPOF], trapof, TRAPSYS);
    trapsetintgate(&idt[TRAPBR], trapbr, TRAPSYS);
    trapsetintgate(&idt[TRAPUD], trapud, TRAPSYS);
    trapsetintgate(&idt[TRAPNM], trapnm, TRAPSYS);
    trapsetintgate(&idt[TRAPDF], trapdf, TRAPSYS);
    trapsetintgate(&idt[TRAPTS], trapts, TRAPSYS);
    trapsetintgate(&idt[TRAPNP], trapnp, TRAPSYS);
    trapsetintgate(&idt[TRAPSS], trapss, TRAPSYS);
    trapsetintgate(&idt[TRAPGP], trapgp, TRAPSYS);
    trapsetintgate(&idt[TRAPPF], trappf, TRAPSYS);
    trapsetintgate(&idt[TRAPMF], trapmf, TRAPSYS);
    trapsetintgate(&idt[TRAPAC], trapac, TRAPSYS);
    trapsetintgate(&idt[TRAPMC], trapmc, TRAPSYS);
    trapsetintgate(&idt[TRAPXF], trapxf, TRAPSYS);
    /* system call trap */
    trapsetintgate(&idt[TRAPSYSCALL], syscall, TRAPUSER);
    /* IRQs */
    trapsetintgate(&idt[trapirqid(IRQTMR)], irqtmr0, TRAPUSER);
    trapsetintgate(&idt[trapirqid(IRQKBD)], irqkbd, TRAPUSER);
    trapsetintgate(&idt[trapirqid(IRQMOUSE)], irqmouse, TRAPUSER);
#if (SMP)
    trapsetintgate(&idt[trapirqid(IRQERROR)], irqerror, TRAPUSER);
#endif
#if (APIC)
    trapsetintgate(&idt[trapirqid(IRQAPICTMR)], irqtmr, TRAPUSER);
    trapsetintgate(&idt[trapirqid(IRQSPURIOUS)], irqspurious, TRAPUSER);
#endif
#if 0
    trapsetintgate(&idt[TRAPV86MODE], trapv86, TRAPUSER);
#endif
    /* initialize interrupts */
    idtptr.lim = NINTR * sizeof(uint64_t) - 1;
    idtptr.adr = (uint32_t)idt;
    idtset();

    return;
}

void
trapinitprio(void)
{
    trapsetprio(IRQKBD, 0);
    trapsetprio(IRQMOUSE, 1);
    trapsetprio(IRQIDE0, 2);
    trapsetprio(IRQIDE1, 3);
}

void
trapinit(void)
{
    trapinitidt(kernidt);
    trapinitprio();
    picinit();  // initialise interrupt controllers
    /* mask timer interrupt, enable other interrupts */
    outb(0x01, 0x21);
    outb(0x00, 0xa1);
    k_enabintr();
//    __asm__ __volatile__ ("sti\n");
//    pitinit();  // initialise interrupt timer

    return;
}

