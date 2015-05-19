#include <stdlib.h>
#include <dungeon/cell.h>

struct celldng celldng;

int
main(int argc, char *argv[])
{
    cellinitdng(&celldng, 16, 256, 256);
    cellbuilddng(&celldng, 16);

    exit(0);
}

