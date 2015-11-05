#include <mjolnir/conf.h>
#if (TEST)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <dungeon/dng.h>
#include <mjolnir/mjol.h>
#include <mjolnir/obj.h>
#include <corewar/cw.h>
#include <corewar/zeus.h>

extern void             mjolquitsig(int sig);
extern struct mjolchr * mjolmkplayer(void);
extern void             mjolinit(struct mjolgame *game,
                                 int argc, char *argv[]);
extern void             dnginitobjmjol(void);
extern void             mjolgameloop(struct mjolgame *game);

int
main(int argc, char *argv[])
{
    struct mjolgame game;

    memset(&game, 0, sizeof(game));
    signal(SIGINT, mjolquitsig);
    signal(SIGQUIT, mjolquitsig);
    signal(SIGTERM, mjolquitsig);
    mjolinit(&game, argc, argv);
    dnginitobjmjol();
    mjolgameloop(&game);

    /* NOTREACHED */
    exit(0);
}

