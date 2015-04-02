#include <mjolnir/mjol.h>

#if (MJOLDEBUG)

#include <stdio.h>

void
mjolprintobj(struct mjolobj *obj)
{
    fprintf(stderr, "%c", (char)obj->data.type);
}

void
mjolprintroom(struct mjolroom *room)
{
    fprintf(stderr, "ROOM %p\n", room);
    fprintf(stderr, "X == %ld, Y == %ld, W == %ld, H == %ld\n",
            room->x, room->y, room->width, room->height);
#if 0
    if (room->left) {
        fprintf(stderr, "LEFT %p: ", room);
        mjolprintroom(room->left);
    }
    if (room->right) {
        fprintf(stderr, "RIGHT %p: ", room);
        mjolprintroom(room->right);
    }
#endif

    return;
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
                fprintf(stderr, ".");
            }
        }
        fprintf(stderr, "\n");
    }
}

#endif /* MJOLDEBUG */

