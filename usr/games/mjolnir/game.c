#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <mjolnir/conf.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

extern struct mjolchar * mjolmkplayer(struct mjolgame *game);
extern long              mjolgetopt(struct mjolgame *game,
                                    int argc, char *argv[]);
extern void              mjolinitscr(struct mjolgame *game);
extern void              mjolgendng(struct mjolgame *game);
extern void              mjolinitcmd(void);
extern void              mjoldocmd(struct mjolgame *game, int ch);
extern void              mjolchase(struct mjolchar *src, struct mjolchar *dest);
extern void              mjoldoturn(struct mjolgame *game,
                                    struct mjolchar *data);

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
    mjolquitgame = 1;
}

void
mjolintro(void)
{
    int ch;

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
    ch = getchar();

    return;
}

void
mjolinit(struct mjolgame *game, int argc, char *argv[])
{
    struct mjolgame   *data = calloc(1, sizeof(struct mjolgame *));
    struct mjolobj  ***objtab;
    long               x;

    mjolgame = game;
    signal(SIGINT, mjolquitsig);
    signal(SIGQUIT, mjolquitsig);
    signal(SIGTERM, mjolquitsig);
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
    mjolinitscr(game);
    if (!game->nlvl) {
        game->nlvl = MJOL_DEF_NLVL;
    }
    if (!game->width) {
        game->width = MJOL_DEF_WIDTH;
    }
    if (!game->height) {
        game->height = MJOL_DEF_HEIGHT;
    }
    objtab = calloc(game->width, sizeof(struct mjolobj *));
    for (x = 0 ; x < game->width ; x++) {
        objtab[x] = calloc(game->height, sizeof(struct mjolobj *));
        if (!objtab[x]) {
            fprintf(stderr, "memory allocation failure\n");
            
            exit(1);
        }
    }
    game->objtab = objtab;
    mjolinitscr(game);
    mjolinitcmd();
    
    return;
}

void
mjolheartbeat(void)
{
    struct mjolchar *src = mjolchaseq;

    while (src) {
        mjolchase(src, mjolgame->player);
        src = src->data.next;
    }
}

void
mjolgameloop(struct mjolgame *game)
{
    struct mjolchar *chtab[4];
    long             l;

    do {
        for (l = 0 ; l < 4 ; l++) {
            if (chtab[l]) {
                mjoldoturn(game, chtab[l]);
            }
        }
    } while (!mjolquitgame);
    mjolquit(0);

    /* NOTREACHED */
    exit(0);
}

