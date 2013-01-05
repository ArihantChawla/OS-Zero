#include <stdint.h>
#define __KERNEL__ 1
#include <kern/signal.h>
#include <kern/conf.h>
#include <zero/param.h>
#include <zero/types.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/trap.h>
#include <kern/unit/ia32/io.h>

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
    trapsetintgate(&idt[trapirqid(IRQTIMER)], irqtimer0, TRAPUSER);
    trapsetintgate(&idt[trapirqid(IRQKBD)], irqkbd, TRAPUSER);
    trapsetintgate(&idt[trapirqid(IRQMOUSE)], irqmouse, TRAPUSER);
#if (SMP)
    trapsetintgate(&idt[trapirqid(IRQERROR)], irqerror, TRAPUSER);
    trapsetintgate(&idt[trapirqid(IRQSPURIOUS)], irqspurious, TRAPUSER);
#endif
#if 0
    trapsetintgate(&idt[TRAPV86MODE], trapv86, TRAPUSER);
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

