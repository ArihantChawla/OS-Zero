#include <gfx/rgb.h>
#include <kern/conf.h>
#include <kern/io/drv/chr/cons.h>
#include <kern/io/drv/pc/vga.h>

#if 0
argb32_t        confgcolor = GFXWHITE;
argb32_t        conbgcolor = GFXBLACK;
conputsfunc    *conputs = vgaputs;
conputcharfunc *conputchar = vgaputchar;
#endif

struct cons constab[NCONS] ALIGNED(PAGESIZE);
long        conscur;

void
consinit(int w, int h)
{
#if (VBE)
    vbeinitcons(w, h);
#else
    vgainitcons(w, h);
#endif
}
