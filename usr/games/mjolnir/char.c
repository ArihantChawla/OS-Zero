#include <stdio.h>
#include <stdlib.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

extern long mjolhit(struct mjolchar *src, struct mjolchar *dest);
extern long mjoltrap(struct mjolobj *trap, struct mjolchar *dest);

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

/* determine how many continuous turns a character has */
long
mjolcharnturn(struct mjolchar *chardata)
{
    long speed = chardata->speed;
    unsigned long turn;
    long retval = 0;

    if (speed < 0) {
        turn = chardata->turn;
        /* only move every abs(speed) turns */
        if (chardata->nturn == turn) {
            /* allow movement */
            retval = 1;
            turn -= speed;
            chardata->turn = turn;
        }
    } else {
        /* return speed */
        retval = speed;
    }
    chardata->nturn++;

    return retval;
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
mjoladditem(struct mjolchar *dest, struct mjolobj *item)
{
    ;
}

void
mjoldie(struct mjolchar *dest)
{
    ;
}

typedef long hitfunc(struct mjolchar *, struct mjolchar *);
long
mjolfindmove(struct mjolchar *src, struct mjolchar *dest,
             hitfunc *func, long lim)
{
    long              retval = 0;
    void           ***objtab = mjolgame->objtab;
    struct mjolobj   *obj;
    struct mjolobj   *item;
    long              destx = dest->data.x;
    long              desty = dest->data.y;
    long              srcx = src->data.x;
    long              srcy = src->data.y;
    long              dx = destx - srcx;
    long              dy = desty - srcy;
    long              type;
    long              val;

    if (srcx == destx && srcy == desty) {

        return retval;
    }
    if (((labs(dx) == lim && labs(dy) <= lim)
         || (labs(dy) == lim && labs(dx) <= lim))
        && (func)) {
        /* attack */
        retval = func(src, dest);
    } else {
        /* TODO: make collision-checking/path-finding actually work... =) */
        if (labs(dx) > lim) {
            if  (dx < -lim) {
                /* dest is to the left of src */
                dx = -1;
            } else {
                /* dest is to the right of src */
                dx = 1;
            }
            srcx += dx;
            if (dy < -lim) {
                /* dest is above src */
                dy = -1;
            } else if (dy > lim) {
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
        } else if (labs(dy) > lim) {
            /* vertical movement only */
            if (dy < -lim) {
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

                                    return retval;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (!lim && srcx == destx && srcy == desty
            && !(src->data.flg & MJOL_CHAR_NO_PICK)) {
            item = objtab[destx][desty];
            while (item) {
                type = item->data.type;
                if (mjolisitem(type)) {
                    obj = item->data.next;
                    if (obj) {
                        obj->data.prev = item->data.prev;
                    }
                    obj = item->data.prev;
                    if (obj) {
                        obj->data.next = item->data.next;
                    } else {
                        objtab[destx][desty] = item->data.next;
                    }
                    mjoladditem(dest, item);
                }
                item = item->data.next;
            }
            item = objtab[destx][desty];
            while (item) {
                type = item->data.type;
                if (type == MJOL_OBJ_TRAP) {
                    retval += mjoltrap(item, dest);
                }
                item = item->data.next;
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

    return retval;
}

void
mjolchase(struct mjolchar *src, struct mjolchar *dest)
{
    long val = mjolfindmove(src, dest, mjolhit, 1);

    if (val) {
        dest->hp -= val;
        if  (dest->hp <= 0) {
            mjoldie(dest);
        }
    }

    return;
}

