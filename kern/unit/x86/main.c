#include <kern/conf.h>
#if (VBE)
#include <kern/io/drv/pc/vbe.h>
#endif
extern void seginit(long id);
#if (VBE)
extern void vbeinit(void);
#endif
extern void trapinitprot(void);
extern void kinitprot(unsigned long pmemsz);
extern void kinitlong(unsigned long pmemsz);

long kernlongmode;

ASMLINK
void
kmain(struct mboothdr *boothdr, unsigned long longmode)
{
    unsigned long pmemsz = grubmemsz(boothdr);

    kernlongmode = longmode;
    /* determine amount of RAM */
    pmemsz = grubmemsz(boothdr);
    /* bootstrap kernel */
#if (!VBE)
    /* initialize interrupt handling */
    trapinitprot();
#endif
    /* INITIALISE BASE HARDWARE */
    /* initialise memory segmentation */
    seginit(0);
#if (VBE)
    /* initialise VBE graphics subsystem */
    vbeinit();
    trapinitprot();
#endif
    kinitprot(pmemsz);
    
    /* NOTREACHED */
    return;
}

