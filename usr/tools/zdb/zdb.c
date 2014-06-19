#include <stdlib.h>
#include <stdio.h>
#include <zdb/zdb.h>

/*
 * commands
 * --------
 * - step       - execute next line
 * - print      - print contents of variables, registers, and memory
 * - set        - set contents of registers and memory
 * - break      - set breakpoint
 */

struct zdb *
zdbinit(struct zdb *zdb, int argc, char *argv[])
{
    zdbgetopt(zdb, argc, argv);
    uisetsys(&zdb->ui, zdb->ui.type);
    uiinit(&zdb->ui, argc, argv);

    return zdb;
}

int
main(int argc, char *argv[])
{
    struct zdb zdb;

    if (!zdbinit(&zdb, argc, argv)) {
        fprintf(stderr, "failed to initialise %s\n", argv[0]);

        exit(1);
    }

    exit(0);
}

