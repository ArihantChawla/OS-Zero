#include <mjolnir/mjol.h>

long
mjolgetopt(struct mjolgamedata *gamedata, int argc, char *argv[])
{
    long  ndx;
    char *str;
    char *ptr;

    for (ndx = 1 ; ndx < argc ; ndx++) {
        str = argv[ndx];
        if (!strncmp(str, "-n", 2)) {
            /* get number of levels */
            ndx++;
            str = argv[ndx];
            gamedata->nlvl = strtol(str, &ptr, 10);
            if (ptr) {
                fprintf(stderr, "invalid number of levels: %s\n", str);
            }
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
#if (MJOLNIR_VGA_TEXT)
            if (!strncmp(str, "vga", 3)) {
                
            }
#endif
        } else if (!strncmp(str, "-h", 2)) {
            ndx++;
            str = argv[ndx];
            gamedata->height = strtol(str, &ptr, 10);
            if (ptr) {
                fprintf(stderr, "invalid level height: %s\n", str);
            }
        }
    }
}

