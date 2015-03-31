#include <mjolnir/mjol.h>

#if (MJOLDEBUG)

#include <stdio.h>

void
mjolprintobj(struct mjolobj *obj)
{
    fprintf(stderr, "%c", (char)obj->data.type);
}

void
mjolprintlvl(struct mjolgame *game, long lvl)
{
    struct mjolobj ***tab = game->objtab[lvl];
    struct mjolobj   *obj;
    long              y;
    long              x;

    fprintf(stderr, "LVL #%ld:\n", lvl);
    for (y = 0 ; y < game->height ; y++) {
        for (x = 0 ; x < game->width ; x++) {
            obj = tab[y][x];
            if (obj) {
                mjolprintobj(obj);
            } else {
                fprintf(stderr, " ");
            }
        }
        fprintf(stderr, "\n");
    }
}

#endif /* MJOLDEBUG */

