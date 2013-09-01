#if (TEST)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <time.h>
#include <zero/randmt32.h>
#include <dungeon/dng.h>
#include <mjolnir/mjol.h>

extern void              mjolinit(struct mjolgame *game,
                                  int argc, char *argv[]);
extern struct mjolrect **mjolgenrooms(struct mjolgame *game, long *nroom);
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

    srandmt32(time(NULL));
    mjolinit(&game, argc, argv);
    lvltab = mjolgenrooms(&game, &nroom);
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

