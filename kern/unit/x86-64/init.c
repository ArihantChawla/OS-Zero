#include <zero/cdecl.h>
#include <zero/param.h>

ASMLINK
void
kinit(struct mboothdr *hdr)
{
    unsigned long    pmemsz;

    __asm__ __volatile__ ("cli\n");
    __asm__ __volatile__ ("movl %%ebx, %0\n" : "=rm" (boothdr));
    pmemsz = grubmemsz(boothdr);
    trapinit();
    longmode(boothdr, pmemsz); // jumps into kmain()
}

