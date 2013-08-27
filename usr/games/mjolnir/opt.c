#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mjolnir/conf.h>
#include <mjolnir/mjol.h>

void
mjolusage(void)
{
    printf("mjolnir <options>\n");
    printf("--help\tprint this help\n");
    printf("-n name\tset name of player\n");
    
}

void
mjolgetopt(struct mjolgamedata *gamedata, int argc, char *argv[])
{
    long    ndx;
    char   *str;
    char   *ptr;

    for (ndx = 1 ; ndx < argc ; ndx++) {
        str = argv[ndx];
        if (!strncmp(str, "--help", 6)) {
            mjolusage();

            exit(0);
        } else if (!strncmp(str, "-l", 2)) {
            /* get number of levels */
            ndx++;
            str = argv[ndx];
            gamedata->nlvl = strtol(str, &ptr, 10);
            if (ptr) {
                fprintf(stderr, "invalid number of levels: %s\n", str);
            }
        } else if (!strncmp(str, "-n", 2)) {
            /* get number of levels */
            ndx++;
            str = argv[ndx];
            gamedata->nicks[0] = str;
        } else if (!strncmp(str, "-w", 2)) {
            ndx++;
            str = argv[ndx];
            gamedata->width = strtol(str, &ptr, 10);
            if (ptr) {
                fprintf(stderr, "invalid level width: %s\n", str);
            }
        } else if (!strncmp(str, "-W", 2)) {
            ndx++;
            str = argv[ndx];
#if (MJOL_VGA_TEXT)
            if (!strncmp(str, "vga", 3)) {
                gamedata->scrtype = MJOL_SCR_VGA_TEXT;
            }
#else
            if (!strncmp(str, "vga", 3)) {
                fprintf(stderr, "unsupported screen type vga\n");

                exit(1);
            }
#endif
            if (!gamedata->scrtype) {
#if (MJOL_TTY)
                if (!strncmp(str, "tty", 3)) {
                    gamedata->scrtype = MJOL_SCR_TTY;
                }
#else
                if (!strncmp(str, "tty", 3)) {
                    fprintf(stderr, "unsupported screen type tty\n");

                    exit(1);
                }
#endif
                if (!gamedata->scrtype) {
#if (MJOL_X11)
                    if (!strncmp(str, "x11", 3)) {
                        gamedata->scrtype = MJOL_SCR_X11;
                    }
#else
                    if (!strncmp(str, "x11", 3)) {
                        fprintf(stderr, "unsupported screen type x11\n");

                        exit(1);
                    }
#endif
                }
            }
        } else if (!strncmp(str, "-h", 2)) {
            ndx++;
            str = argv[ndx];
            gamedata->height = strtol(str, &ptr, 10);
            if (ptr) {
                fprintf(stderr, "invalid level height: %s\n", str);
            }
        }
    }

    return;
}

