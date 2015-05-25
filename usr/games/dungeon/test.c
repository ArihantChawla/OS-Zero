#include <stdlib.h>
#include <dungeon/api.h>
#include <dungeon/cell.h>

struct celldng     celldng;
struct cellgenparm genparm;

static void
cellinitparm(struct cellgenparm *parm)
{
    cellsetdefparm(parm);
    parm->caveparm.minsize = 64 * 36;
    parm->caveparm.maxsize = 256 * 144;
    parm->corparm.brkout = 1024 * 768;
}

static void
cellplotdng(struct celldng *dng)
{
    ;
}

int
main(int argc, char *argv[])
{
    cellinitparm(&genparm);
    cellinitdng(&celldng, 16, 256, 256);
    cellbuilddng(&celldng, 16);

    exit(0);
}

