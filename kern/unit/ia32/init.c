#include <kern/conf.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
//#include <kern/proc.h>
#include <kern/util.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/vm.h>

extern void trapinit(void);
extern void kmain(struct mboothdr *hdr, unsigned long pmemsz);

ASMLINK
void
kinit(struct mboothdr *boothdr)
{
    unsigned long    pmemsz;

//    __asm__ __volatile__ ("cli\n");
    /* determine amount of RAM */
    /* boot.S leaves the multiboot header address in %ebx */
//    __asm__ __volatile__ ("movl %%ebx, %0\n" : "=rm" (boothdr));
    pmemsz = grubmemsz(boothdr);
    /* bootstrap kernel */
#if (!VBE)
    trapinit();                         // interrupt management
#endif
//    __asm__ __volatile__ ("sti\n");
    kmain(boothdr, pmemsz);
}

