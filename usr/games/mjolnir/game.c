#include <stdlib.h>
#include <mjol/mjol.h>

char mjolgamename[] = "mjolnir";

long
mjolinitgame(struct dnggame *game)
{
    struct mjolgamedata *data = calloc(1, sizeof(struct mjolgamedata *));
    long                 retval = 0;

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

    return retval;
}

