#include <stdint.h>
#define __KERNEL__ 1
#include <kern/signal.h>
#include <kern/conf.h>
#include <zero/param.h>
#include <zero/types.h>
#include <kern/ia32/boot.h>
#include <kern/ia32/trap.h>
#include <kern/ia32/io.h>

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

extern void irqtimer0(void);
extern void irqtimer(void);
extern void irqkbd(void);
extern void irqmouse(void);
#if (SMP)
extern void irqerror(void);
extern void irqspurious(void);
#endif

extern void syscall(void);

#if (SMP)
extern volatile long   mpmultiproc;
#endif
extern uint64_t        kernidt[];
extern struct m_farptr idtptr;

#if 0
uint8_t trapsigmap[NIDT] ALIGNED(PAGESIZE) = {
    SIGFPE,     // DE
    0,          // DB
    0,          // NMI
    SIGTRAP,    // BP
    0,          // OF
    SIGBUS,     // BR
    SIGILL,     // UD
    SIGILL,     // NM
    0,          // DF
    0,          // RES1
    0,          // TS
    SIGSEGV,    // NP
    SIGSTKFLT,  // SS
    SIGSEGV,    // GP
    0,          // PF
    0,          // RES2
    SIGFPE,     // MF
    SIGBUS,     // AC
    SIGABRT,    // MC
    SIGFPE      // XF
};
#endif

//long interrbits = INTERRBITS;

void
idtinit(uint64_t *idt)
{
    trapsetgate(&idt[TRAPDE], trapde, TRAPSYS);
    trapsetgate(&idt[TRAPDB], trapdb, TRAPSYS);
    trapsetgate(&idt[TRAPNMI], trapnmi, TRAPSYS);
    trapsetgate(&idt[TRAPBP], trapbp, TRAPSYS);
    trapsetgate(&idt[TRAPOF], trapof, TRAPSYS);
    trapsetgate(&idt[TRAPBR], trapbr, TRAPSYS);
    trapsetgate(&idt[TRAPUD], trapud, TRAPSYS);
    trapsetgate(&idt[TRAPNM], trapnm, TRAPSYS);
    trapsetgate(&idt[TRAPDF], trapdf, TRAPSYS);
    trapsetgate(&idt[TRAPTS], trapts, TRAPSYS);
    trapsetgate(&idt[TRAPNP], trapnp, TRAPSYS);
    trapsetgate(&idt[TRAPSS], trapss, TRAPSYS);
    trapsetgate(&idt[TRAPGP], trapgp, TRAPSYS);
    trapsetgate(&idt[TRAPPF], trappf, TRAPSYS);
    trapsetgate(&idt[TRAPMF], trapmf, TRAPSYS);
    trapsetgate(&idt[TRAPAC], trapac, TRAPSYS);
    trapsetgate(&idt[TRAPMC], trapmc, TRAPSYS);
    trapsetgate(&idt[TRAPXF], trapxf, TRAPSYS);
    /* system call trap */
    trapsetgate(&idt[TRAPSYSCALL], syscall, TRAPUSER);
    /* IRQs */
    trapsetgate(&idt[trapirqid(IRQTIMER)], irqtimer0, TRAPUSER);
    trapsetgate(&idt[trapirqid(IRQKBD)], irqkbd, TRAPUSER);
    trapsetgate(&idt[trapirqid(IRQMOUSE)], irqmouse, TRAPUSER);
#if (SMP)
    trapsetgate(&idt[trapirqid(IRQERROR)], irqerror, TRAPUSER);
    trapsetgate(&idt[trapirqid(IRQSPURIOUS)], irqspurious, TRAPUSER);
#endif
#if 0
    trapsetgate(&idt[TRAPV86MODE], trapv86, TRAPUSER);
#endif
    /* initialize interrupts */
    idtptr.lim = NIDT * sizeof(uint64_t) - 1;
    idtptr.adr = (uint32_t)idt;
    idtset();

    return;
}

void
trapinit(void)
{
    idtinit(kernidt);
    picinit();  // initialise interrupt controllers
    /* mask timer interrupt, enable other interrupts */
    outb(0x01, 0x21);
    outb(0x00, 0xa1);
//    pitinit();  // initialise interrupt timer

    return;
}

