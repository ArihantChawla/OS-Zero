#include <stdlib.h>
#include <mjol/mjol.h>

char mjolgamename[] = "mjolnir";

struct mjolgameopt {
    long nlvl;
};

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

long
mjolinitgame(struct dnggame *game)
{
    struct mjolgamedata *data = calloc(1, sizeof(struct mjolgamedata *));

    game->name = mjolgamename;
    mjolgetopt(data, game->argc, game->argv);
    if (!data->nlvl) {
        data->nlvl = MJOL_DEF_NLVL;
    }
    if (!data->width) {
        data->width = MJOL_DEF_WIDTH;
    }
    if (!data->height) {
        data->height = MJOL_DEF_HEIGHT;
    }
    data->dngtab = calloc(data->nlvl * data->width * data->height,
                          sizeof (char));
    mjolgendng(data);
}

