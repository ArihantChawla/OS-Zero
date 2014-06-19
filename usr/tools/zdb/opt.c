#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zdb/zdb.h>

void
zdbgetopt(struct zdb *zdb, int argc, char *argv[])
{
    int   ndx;
    char *cp;
    long  val;

    zdb->ui.type = ZDB_DEFUI;
    for (ndx = 1 ; ndx < argc ; ndx++) {
        cp = argv[ndx];
        if (!strncmp(cp, "--uitype", 8)) {
            ndx++;
            if (ndx < argc) {
                cp = argv[ndx];
                if (!strncmp(cp, "zero", 4)) {
                    zdb->ui.type = UI_SYS_ZERO;
                } else if (!strncmp(cp, "xorg", 4)) {
                    zdb->ui.type = UI_SYS_XORG;
                } else {
                    fprintf(stderr, "ZDB: --uitype: invalide argument %s\n",
                            cp);

                    exit(1);
                }
            } else {
                fprintf(stderr, "ZDB: --uitype: no argument\n");

                exit(1);
            }
        } else if (!strncmp(cp, "--font", 6)) {
            ndx++;
            if (ndx < argc) {
                cp = argv[ndx];
                zdb->font.name = cp;
            } else {
                fprintf(stderr, "ZDB: --font: no argument\n");

                exit(1);
            }
        }
    }
}

