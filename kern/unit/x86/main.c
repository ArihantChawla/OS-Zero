#include <kern/conf.h>
#if (VBE)
#include <kern/io/drv/pc/vbe.h>
#endif
extern void seginit(long id);
#if (VBE)
extern void vbeinit(void);
#endif
extern void kinitprot(unsigned long pmemsz);
extern void kinitlong(unsigned long pmemsz);

ASMLINK
void
kmain(struct mboothdr *boothdr, unsigned long longmode)
{
    unsigned long pmemsz = grubmemsz(boothdr);
    
    /* INITIALISE BASE HARDWARE */
    /* initialise memory segmentation */
    seginit(0);
#if (VBE)
    /* initialise VBE graphics subsystem */
    vbeinit();
#endif
    if (longmode) {
//        kinitlong();
    } else {
        kinitprot(pmemsz);
    }
    
    /* NOTREACHED */
    return;
}

