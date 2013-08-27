#include <stdio.h>
#include <stdlib.h>
#include <mjolnir/mjol.h>

extern long mjolgetopt(struct mjolgamedata *gamedata, int argc, char *argv[]);
extern void mjolinitscr(struct mjolgamedata *data);
extern void mjolgendng(struct mjolgamedata *gamedata);

extern struct mjolscr mjolscr;

char mjolgamename[] = "mjolnir";

void
mjolgameintro(void)
{
    printf("Legend has it that many moons ago, Fenris, the eldest child of Loki and\n");
    printf("Angrboda, broke free from his chain, Pleignir, and stole Mjolnir from Thor.\n");
    printf("Fenris is believed to have hidden Mjolnir, Thor's hammer, into a dungeon.\n");
    printf("\n");
    printf("It is said that Thor shall ascend the one to return Mjolnir to a deity for\n");
    printf("saving the world from the forces of evil creating chaos with lightnings and\n");
    printf("thunder. Clairvoyants say evil forces are already on the hunt for Mjolnir.\n");
    printf("\n");
    printf("Armed with nothing more than a few food rations, your pet dog or cat, a +1\n");
    printf("blessed armor, a +3 neutral sword, and a piece of Pleignir, you enter the\n");
    printf("Dungeon. Good luck, adventurer, and beware of Fenris!\n");
    printf("\n");
    printf("Press a key to continue...\n");

    return;
}

void
mjolinitgame(struct dnggame *game)
{
    struct mjolgamedata *data = calloc(1, sizeof(struct mjolgamedata *));
    char                 ch;

    game->name = mjolgamename;
    data->nicks = calloc(1, sizeof(struct dngchar *));
    mjolgetopt(data, game->argc, game->argv);
    if (!data->nicks[0]) {
        data->nicks[0] = MJOL_DEF_NICK;
    }
    if (!data->scrtype) {
        fprintf(stderr, "no supported screen type found\n");

        exit(1);
    }
    mjolgameintro();
    ch = getchar();
    mjolinitscr(data);
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
                          sizeof(struct mjolobjstk));
    mjolinitscr(data);
    mjolgendng(data);

    return;
}

void
mjolgameloop(struct dnggame *dng)
{
    ;
}

