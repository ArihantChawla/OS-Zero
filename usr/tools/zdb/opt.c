#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zdb/zdb.h>

void
zdbsetdefopt(struct zdb *zdb)
{
    zdb->ui.type = ZDB_DEFUI;
    zdb->api.type = ZDB_DEFAPI;

    return;
}

void
zdbgetopt(struct zdb *zdb, int argc, char *argv[])
{
    int   ndx;
    char *cp;
    long  val;

    zdbsetdefopt(zdb);
    for (ndx = 1 ; ndx < argc ; ndx++) {
        cp = argv[ndx];
        if (!strncmp(cp, "--ui", 4)) {
            ndx++;
            if (ndx < argc) {
                cp = argv[ndx];
                if (!strncmp(cp, "zero", 4)) {
                    zdb->ui.type = UI_SYS_ZERO;
                } else if (!strncmp(cp, "xorg", 4)) {
                    zdb->ui.type = UI_SYS_XORG;
                } else {
                    fprintf(stderr, "ZDB: --ui: invalid argument %s\n",
                            cp);

                    exit(1);
                }
            } else {
                fprintf(stderr, "ZDB: --ui: no argument\n");

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
        } else if (!strncmp(cp, "--api", 5)) {
            ndx++;
            if (ndx < argc) {
                cp = argv[ndx];
                if (!strncmp(cp, "gdb", 3)) {
                    zdb->api.type = ZDB_API_GDB;
                } else if (!strncmp(cp, "wpm", 3)) {
                    zdb->api.type = ZDB_API_WPM;
                } else if (!strncmp(cp, "zpu", 3)) {
                    zdb->api.type = ZDB_API_ZPU;
                } else if (!strncmp(cp, "bochs", 5)) {
                    zdb->api.type = ZDB_API_ZPU;
                } else {
                    fprintf(stderr, "ZDB: --uitype: invalid argument %s\n",
                            cp);
                    
                    exit(1);
                }
            } else {
                fprintf(stderr, "ZDB: --api: no argument\n");

                exit(1);
            }
        }
    }

    return;
}

