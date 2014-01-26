#include <gfx/rgb.h>
#include <kern/conf.h>
#include <kern/io/drv/chr/con.h>
#include <kern/io/drv/pc/vga.h>

#if 0
argb32_t        confgcolor = GFXWHITE;
argb32_t        conbgcolor = GFXBLACK;
conputsfunc    *conputs = vgaputs;
conputcharfunc *conputchar = vgaputchar;
#endif

struct con contab[NCON] ALIGNED(PAGESIZE);
long       concur;

void
coninit(int w, int h)
{
#if (VBE)
    vbeinitcon(w, h);
#else
    vgainitcon(w, h);
#endif
}
