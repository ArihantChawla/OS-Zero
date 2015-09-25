#include <kern/conf.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
//#include <kern/proc.h>
#include <kern/util.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/vm.h>

extern void trapinit(void);
extern void kmain(struct mboothdr *hdr, unsigned long pmemsz);

ASMLINK NORETURN
void
kinit(struct mboothdr *boothdr, unsigned long longmode)
{
    unsigned long    pmemsz;

//    __asm__ __volatile__ ("cli\n");
    /* determine amount of RAM */
    pmemsz = grubmemsz(boothdr);
    /* bootstrap kernel */
#if (!VBE)
    trapinitprot();                         // interrupt management
#endif
//    __asm__ __volatile__ ("sti\n");
    kmain(boothdr, pmemsz);

    /* NOTREACHED */
    for ( ; ; ) { ; }
}

