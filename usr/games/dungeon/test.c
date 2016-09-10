#include <stdlib.h>
#include <stdio.h>
#include <dungeon/conf.h>
#include <dungeon/api.h>
#include <dungeon/cell.h>
#include <corewar/cw.h>
#include <corewar/zeus.h>

struct celldng     celldng;
struct cellgenparm genparm;
struct cwmars      cwmars;

static void
cellinitparm(struct cellgenparm *parm)
{
    cellsetdefparm(parm);
#if 0
    parm->caveparm.minsize = 64;
    parm->caveparm.maxsize = 1024;
#endif
    parm->caveparm.minsize = 16;
    parm->caveparm.maxsize = 1024;
    
//    parm->corparm.brkout = 1024 * 768;
//    parm->corparm.brkout = 10000;

    return;
}

#if 0
static void
cellplotdng(struct celldng *dng)
{
    /* plot caves */
    cellplotcave(dng);
    /* plot corridors */
    cellplotcor(dng);
}
#endif

int
main(int argc, char *argv[])
{
#if 0
    FILE *fp;
    long  base;
    long  lim;
    long  pc1;
    long  pc2;
#endif

    cellinitparm(&genparm);
#if (DNG_PIXELCELLS)
    cellinitdng(&celldng, 1024, 768);
#else
    cellinitdng(&celldng, 1024 / 8, 768 / 8);
#endif
    cellbuilddng(&celldng);
//    cellplotdng(&celldng);
    cellx11loop(&celldng);
    
    /* NOTREACHED */
    exit(0);
}

