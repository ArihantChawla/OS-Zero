#include <stdio.h>
#include <stdlib.h>
#include <mjolnir/conf.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

extern long               mjolgetopt(struct mjolgame *game,
                                     int argc, char *argv[]);
extern struct mjolchar  * mjolmkplayer(struct mjolgame *game);
extern struct mjolrect ** mjolinitrooms(struct mjolgame *game, long *nroom);
extern void               mjolopenscr(struct mjolgame *game);
extern void               mjolinitcmd(void);
extern void               mjolinitobj(void);
void                      mjolgendng(struct mjolgame *game);
extern void               mjoldocmd(struct mjolgame *game, int ch);
extern long               mjoldoturn(struct mjolgame *game,
                                     struct mjolchar *data);
extern long               mjolchaseall(struct mjolgame *game);

extern struct mjolchar *mjolplayer;
extern struct mjolchar *mjolchaseq;

static char             mjolgamename[] = "mjolnir";
static volatile long    mjolquitgame;
struct mjolgame        *mjolgame;

void
mjolquit(long val)
{
    void (*func)(void) = mjolgame->scr->close;

    if (func) {
        func();
    }

    exit(val);
}

void
mjolquitsig(int sig)
{
    mjolquit(sig);
}

void
mjolintro(void)
{
    printf("\n");
    printf("Legend has it that many moons ago, Fenris, the eldest child of Loki and\n");
    printf("Angrboda, broke free from his chain, Pleignir, and stole Mjolnir from Thor.\n");
    printf("Fenris is believed to have hidden Mjolnir, Thor's hammer, into a dungeon.\n");
    printf("\n");
    printf("It is said that Thor shall ascend the one to return Mjolnir to a deity for\n");
    printf("saving the world from the forces of evil creating chaos with lightnings and\n");
    printf("thunder. Clairvoyants say evil forces are already on the hunt for Mjolnir.\n");
    printf("\n");
    printf("Armed with nothing more than a few food rations, your pet dog, a +1 blessed\n");
    printf("armor, a +3 neutral sword, and a piece of Pleignir, you enter the Dungeon.\n");
    printf("\n");
    printf("Good luck, adventurer, and beware of Fenris!\n");
    printf("\n");
    printf("Press a key to continue...\n");
    getchar();

    return;
}

void
mjolinit(struct mjolgame *game, int argc, char *argv[])
{
    struct mjolgame    *data = calloc(1, sizeof(struct mjolgame *));
    struct mjolobj  ****objtab;
    struct mjolchar ****chartab;
    long                y;
    long                w;
    long                h;
    long                lvl;
    long                lim;

    mjolgame = game;
    if (!data) {
        fprintf(stderr, "failed to allocate game data\n");

        exit(1);
    }
    game->data.name = mjolgamename;
    game->nick = calloc(1, sizeof(struct dngchar *));
    mjolgetopt(game, argc, argv);
    if (!game->player) {
        game->player = mjolmkplayer(game);
    }
    if (!game->nick) {
        game->nick = MJOL_DEF_NICK;
    }
    if (!game->scrtype) {
#if (MJOL_TTY)
        game->scrtype = MJOL_SCR_TTY;
#elif (MJOL_VGA_TEXT)
        game->scrtype = MJOL_SCR_VGA_TEXT;
#endif
    }
    if (!game->scrtype) {
        fprintf(stderr, "no supported screen type found\n");
        
        exit(1);
    }
    mjolintro();
    if (!game->nlvl) {
        game->nlvl = MJOL_DEF_NLVL;
    }
    if (!game->width) {
        game->width = MJOL_DEF_WIDTH;
    }
    if (!game->height) {
        game->height = MJOL_DEF_HEIGHT;
    }
    objtab = calloc(game->nlvl, sizeof(struct mjolobj ***));
    chartab = calloc(game->nlvl, sizeof(struct mjolchar ***));
#if 0
    for (x = 0 ; x < game->width ; x++) {
        objtab[x] = calloc(game->height, sizeof(struct mjolobj *));
        if (!objtab[x]) {
            fprintf(stderr, "memory allocation failure\n");
            
            exit(1);
        }
    }
#endif
    lim = game->nlvl;
    w = game->width;
    h = game->height;
    for (lvl = 0 ; lvl < lim ; lvl++) {
        objtab[lvl] = calloc(h, sizeof(struct mjolobj ***));
        chartab[lvl] = calloc(h, sizeof(struct mjolchar ***));
        for (y = 0 ; y < h ; y++) {
            objtab[lvl][y] = calloc(w, sizeof(struct mjolobj **));
            chartab[lvl][y] = calloc(w, sizeof(struct mjolchar **));
        }
    }
    game->objtab = objtab;
    game->chartab = chartab;
    mjolopenscr(game);
    mjolinitobj();
    mjolinitcmd();
    mjolgendng(game);
    
    return;
}

void
mjolgameloop(struct mjolgame *game)
{
    do {
        mjolplayer->hp -= mjoldoturn(game, mjolplayer);
        mjolplayer->hp -= mjolchaseall(game);
    } while (!mjolquitgame);
    mjolquit(0);

    /* NOTREACHED */
    exit(0);
}

