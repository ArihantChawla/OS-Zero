#include <kern/conf.h>
#if (!VBE2)
#include <kern/io/drv/pc/vga.h>
#endif

typedef void conputsfunc(char *str);
typedef void conputcharfunc(int ch);

#if (VBE2)
conputsfunc    *conputs;
conputcharfunc *conputchar;
#else
conputsfunc    *conputs = vgaputs;
conputcharfunc *conputchar = vgaputchar;
#endif

