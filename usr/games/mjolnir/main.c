#if (TEST)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <time.h>
#include <mjolnir/conf.h>
#if (MJOL_USE_ZERO_RANDMT32)
#include <zero/randmt32.h>
#endif
#include <dungeon/dng.h>
#include <mjolnir/mjol.h>

extern void              mjolinit(struct mjolgame *game,
                                  int argc, char *argv[]);
void                     mjolinitobj(void);
extern struct mjolrect **mjolinitrooms(struct mjolgame *game, long *nroom);
extern void              mjolgameloop(struct mjolgame *game);

#if (TEST)
void
mjolprintrect(struct mjolrect *rect)
{
    fprintf(stderr, "x = %ld, y = %ld, width = %ld, height = %ld\n",
            rect->x, rect->y, rect->width, rect->height);
}
#endif

int
main(int argc, char *argv[])
{
    struct mjolgame   game = { { 0 } };
    struct mjolrect **lvltab;
    long              nroom = 0;

    mjolsrand(time(NULL));
    mjolinitobj();
    mjolinit(&game, argc, argv);
    lvltab = mjolinitrooms(&game, &nroom);
#if (TEST)
    {
        long l;

        for (l = 0 ; l < nroom ; l++) {
            mjolprintrect(lvltab[l]);
        }
    }
#endif
    mjolgameloop(&game);

    /* NOTREACHED */
    exit(0);
}

