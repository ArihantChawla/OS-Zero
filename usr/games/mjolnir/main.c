#if (TEST)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <mjolnir/conf.h>
#include <dungeon/dng.h>
#include <mjolnir/mjol.h>

extern void               mjolquitsig(int sig);
extern struct mjolchar *  mjolmkplayer(void);
extern void               mjolinit(struct mjolgame *game,
                                   int argc, char *argv[]);
extern void               mjolinitobj(void);
extern void               mjolgameloop(struct mjolgame *game);

int
main(int argc, char *argv[])
{
    struct mjolgame game = { { 0 } };

    signal(SIGINT, mjolquitsig);
    signal(SIGQUIT, mjolquitsig);
    signal(SIGTERM, mjolquitsig);
    mjolinit(&game, argc, argv);
    mjolgameloop(&game);

    /* NOTREACHED */
    exit(0);
}

