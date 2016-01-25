#include <stdint.h>
#define __KERNEL__ 1
#include <signal.h>
#include <sys/io.h>
#include <kern/conf.h>
#include <zero/param.h>
#include <zero/types.h>
//#include <kern/syscall.h>
#include <kern/asm.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/trap.h>

//extern void ksyscall(void);

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
    trapsetintrgate(&idt[TRAPDE], trapde, TRAPSYS);
    trapsetintrgate(&idt[TRAPDB], trapdb, TRAPSYS);
    trapsetintrgate(&idt[TRAPNMI], trapnmi, TRAPSYS);
    trapsetintrgate(&idt[TRAPBP], trapbp, TRAPSYS);
    trapsetintrgate(&idt[TRAPOF], trapof, TRAPSYS);
    trapsetintrgate(&idt[TRAPBR], trapbr, TRAPSYS);
    trapsetintrgate(&idt[TRAPUD], trapud, TRAPSYS);
    trapsetintrgate(&idt[TRAPNM], trapnm, TRAPSYS);
    trapsetintrgate(&idt[TRAPDF], trapdf, TRAPSYS);
    trapsetintrgate(&idt[TRAPTS], trapts, TRAPSYS);
    trapsetintrgate(&idt[TRAPNP], trapnp, TRAPSYS);
    trapsetintrgate(&idt[TRAPSS], trapss, TRAPSYS);
    trapsetintrgate(&idt[TRAPGP], trapgp, TRAPSYS);
    trapsetintrgate(&idt[TRAPPF], trappf, TRAPSYS);
    trapsetintrgate(&idt[TRAPMF], trapmf, TRAPSYS);
    trapsetintrgate(&idt[TRAPAC], trapac, TRAPSYS);
    trapsetintrgate(&idt[TRAPMC], trapmc, TRAPSYS);
    trapsetintrgate(&idt[TRAPXF], trapxf, TRAPSYS);
    /* system call trap */
    trapsetintrgate(&idt[TRAPSYSCALL], ksyscall, TRAPUSER);
    /* IRQs */
    trapsetintrgate(&idt[trapirqid(IRQTMR)], irqtmr0, TRAPUSER);
    trapsetintrgate(&idt[trapirqid(IRQKBD)], irqkbd, TRAPUSER);
    trapsetintrgate(&idt[trapirqid(IRQMOUSE)], irqmouse, TRAPUSER);
#if (SMP)
    trapsetintrgate(&idt[trapirqid(IRQERROR)], irqerror, TRAPUSER);
#endif
#if (APIC)
    trapsetintrgate(&idt[trapirqid(IRQAPICTMR)], irqtmr, TRAPUSER);
    trapsetintrgate(&idt[trapirqid(IRQSPURIOUS)], irqspurious, TRAPUSER);
#endif
#if 0
    trapsetintrgate(&idt[TRAPV86MODE], trapv86, TRAPUSER);
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
trapinitprot(void)
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

