#include <gfx/rgb.h>
#include <kern/conf.h>
#include <kern/io/drv/pc/vga.h>

typedef void conputsfunc(char *str);
typedef void conputcharfunc(int ch);

argb32_t        confgcolor = GFXWHITE;
argb32_t        conbgcolor = GFXBLACK;
conputsfunc    *conputs = vgaputs;
conputcharfunc *conputchar = vgaputchar;

