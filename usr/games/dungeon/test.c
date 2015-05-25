#include <stdlib.h>
#include <dungeon/api.h>
#include <dungeon/cell.h>

struct celldng     celldng;
struct cellgenparm genparm;

static void
cellinitparm(struct cellgenparm *parm)
{
    cellsetdefparm(parm);
#if 0
    parm->caveparm.minsize = 64 * 36;
    parm->caveparm.maxsize = 256 * 144;
#endif
    parm->caveparm.minsize = 4096;
    parm->caveparm.maxsize = 16384;
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
    cellinitparm(&genparm);
    cellinitdng(&celldng, 16, 1024 / 8, 768 / 8);
    cellbuilddng(&celldng, 1);
//    cellplotdng(&celldng);
    cellx11loop(&celldng);

    exit(0);
}

