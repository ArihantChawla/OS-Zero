#include <stdio.h>
#include <stdlib.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

static uint8_t   chdirbitmap[32] ALIGNED(CLSIZE);
static uint8_t   chargbitmap[32];
struct mjolchar *mjolchaseq;

struct mjolchar *
mjolmkplayer(struct mjolgame *game)
{
    struct mjolchar *data = calloc(1, sizeof(struct mjolchar));

    if (!data) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    data->data.type = MJOL_CHAR_PLAYER;

    return data;
}

void
mjoldoturn(struct mjolgame *game, struct mjolchar *data)
{
    long   n = mjolcharnturn(data);
    int  (*printmsg)(const char *, ...) = game->scr->printmsg;
    int  (*getkbd)(void) = game->scr->getch;
    int    cmd;
    int    dir;
    int    obj;

    if (n) {
        printmsg("You have %ld turns", n);
        while (n--) {
            cmd = getkbd();
//            clrmsg();
            if (bitset(chdirbitmap, cmd)) {
                printmsg("Which direction?");
                dir = getkbd();
            }
            if (bitset(chargbitmap, cmd)) {
                obj = getkbd();
            }
            data->nturn++;
        }
    }
}

void
mjolchase(struct mjolchar *src, struct mjolchar *dest)
{
    void           ***objtab = mjolgame->objtab;
    struct mjolobj   *obj;
    long              destx = dest->data.x;
    long              desty = dest->data.y;
    long              srcx = src->data.x;
    long              srcy = src->data.y;
    long              dx = destx - srcx;
    long              dy = desty - srcy;
    long              type;
    long              val;

    if ((labs(dx) == 1 && labs(dy) <= 1) || (labs(dy) == 1 && labs(dx) <= 1)) {
        /* attack */
        mjolhit(src, dest);
    } else {
        /* TODO: make collision-checking/path-finding actually work... =) */
        if (labs(dx) > 1) {
            if  (dx < -1) {
                /* dest is to the left of src */
                dx = -1;
            } else {
                /* dest is to the right of src */
                dx = 1;
            }
            srcx += dx;
            if (dy < -1) {
                /* dest is above src */
                dy = -1;
            } else if (dy > 1) {
                /* dest is below src */
                dy = 1;
            }
            obj = objtab[srcx][srcy + dy];
            type = obj->data.type;
            if (mjolisobj(type)) {
                /* src can moves horizontally and vertically */
                srcy += dy;
            } else {
                obj = objtab[srcx][srcy];
                type = obj->data.type;
                if (mjolisobj(type)) {
                    /* src moves horizontally but not vertically */
                    ;
                } else {
                    srcx -= dx;
                    obj = objtab[srcx][srcy + dy];
                    type = obj->data.type;
                    if (mjolisobj(type)) {
                        /* src moves vertically but not horizontally */
                        srcy += dy;
                    }
                }
            }
        } else if (labs(dy) > 1) {
            /* vertical movement only */
            if (dy < -1) {
                dy = -1;
            } else {
                dy = 1;
            }
            srcy += dy;
            obj = objtab[srcx][srcy];
            type = obj->data.type;
            if (mjolisobj(type)) {
                /* src moves vertically */
            } else {
                srcy -= dy;
                val = mjolrand() & 0x01;
                if (val) {
                    /* try to move left */
                    dx = -1;
                } else {
                    /* try to move right */
                    dx = 1;
                }
                srcx += dx;
                obj = objtab[srcx][srcy];
                type = obj->data.type;
                if (mjolisobj(type)) {
                    /* move into chosen direction */
                    ;
                } else {
                    /* change horizontal direction */
                    dx = -dx;
                    srcx += 2 * dx;
                    obj = objtab[srcx][srcy];
                    type = obj->data.type;
                    if (mjolisobj(type)) {
                        /* valid move */
                        ;
                    } else {
                        /* change vertical direction */
                        srcx -= dx;
                        dy = -dy;
                        srcy += 2 * dy;
                        obj = objtab[srcx][srcy];
                        type = obj->data.type;
                        if (mjolisobj(type)) {
                            /* valid move */
                            ;
                        } else {
                            /* try vertical and horizontal */
                            srcx -= dx;
                            obj = objtab[srcx][srcy];
                            type = obj->data.type;
                            if (mjolisobj(type)) {
                                /* valid move */
                                ;
                            } else {
                                srcx += 2 * dx;
                                obj = objtab[srcx][srcy];
                                type = obj->data.type;
                                if (mjolisobj(type)) {
                                    /* valid move */
                                } else {
                                    /* no move found */

                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
        obj = src->data.next;
        if (obj) {
            obj->data.prev = src->data.prev;
        }
        obj = src->data.prev;
        if (obj) {
            obj->data.next = src->data.next;
        } else {
            objtab[src->data.x][src->data.y] = src->data.next;
        }
        src->data.prev = NULL;
        src->data.next = objtab[srcx][srcy];
        obj = src->data.next;
        if (obj) {
            obj->data.prev = src;
        }
        src->data.x = srcx;
        src->data.y = srcy;
        objtab[srcx][srcy] = src;
    }

    return;
}

