#include <kern/conf.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
//#include <kern/proc.h>
#include <kern/util.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/vm.h>
#if 0
#include <zero/types.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/thr.h>
#include <kern/io/drv/pc/vga.h>
#include <kern/unit/x86/pit.h>
#include <kern/unit/x86/dma.h>
#include <kern/unit/ia32/kern.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/cpu.h>
#endif

extern void trapinit(void);
extern void kmain(struct mboothdr *hdr, unsigned long pmemsz);

ASMLINK
void
kinit(void)
{
    struct mboothdr *boothdr;
    unsigned long    pmemsz;

//    __asm__ __volatile__ ("cli\n");
    /* determine amount of RAM */
    /* boot.S leaves the multiboot header address in %ebx */
    __asm__ __volatile__ ("movl %%ebx, %0\n" : "=rm" (boothdr));
    pmemsz = grubmemsz(boothdr);
    /* bootstrap kernel */
#if (!VBE)
    trapinit();                         // interrupt management
#endif
//    __asm__ __volatile__ ("sti\n");
    kmain(boothdr, pmemsz);
}

