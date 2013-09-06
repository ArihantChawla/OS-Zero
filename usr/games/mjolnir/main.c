#if (TEST)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <mjolnir/conf.h>
#if (MJOL_USE_ZERO_RANDMT32)
#include <zero/randmt32.h>
#endif
#include <dungeon/dng.h>
#include <mjolnir/mjol.h>

extern void               mjolquitsig(int sig);
extern struct mjolchar *  mjolmkplayer(void);
extern void               mjolinit(struct mjolgame *game,
                                   int argc, char *argv[]);
extern void               mjolinitobj(void);
extern void               mjolgameloop(struct mjolgame *game);

struct mjolrect ** mjolinitrooms(struct mjolgame *game, long *nroom);
int
main(int argc, char *argv[])
{
    struct mjolgame game = { { 0 } };

    signal(SIGINT, mjolquitsig);
    signal(SIGQUIT, mjolquitsig);
    signal(SIGTERM, mjolquitsig);
    mjolsrand(time(NULL));
    mjolinitobj();
    mjolinit(&game, argc, argv);
    mjolgameloop(&game);

    /* NOTREACHED */
    exit(0);
}

