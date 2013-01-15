#include <zero/cdecl.h>
#include <zero/param.h>

ASMLINK
void
kmain(void)
{
    struct mboothdr *boothdr;
    unsigned long    pmemsz;

    __asm__ __volatile__ ("cli\n");
    __asm__ __volatile__ ("movl %%ebx, %0\n" : "=rm" (boothdr));
    trapinit();
    longmode(); // jumps into kinit()
}

