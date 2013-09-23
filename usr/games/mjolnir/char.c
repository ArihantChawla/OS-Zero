#include <stdio.h>
#include <stdlib.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
//#include <mjolnir/conf.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

extern long mjolhit(struct mjolchar *src, struct mjolchar *dest);
extern long mjoltrap(struct mjolobj *trap, struct mjolchar *dest);

mjolcmdfunc *mjolcmdfunctab[256][256];

struct mjolchar *mjolplayer;
struct mjolchar *mjolchaseq;

void
mjolinitchar(struct mjolchar *data, long lvl)
{
    data->hp = 16 + ((lvl > 0)
                     ? (mjolrand() % lvl)
                     : 0);
    data->maxhp = data->hp;
    data->gold = mjolrand() & 0xff;
    data->str = 4 + ((lvl > 0)
                     ? (mjolrand() % lvl)
                     : 0);
    data->maxstr = data->str;
    data->arm = 0;
    data->exp = 0;
    data->speed = 1;
    data->turn = 0;
    data->nturn = 0;
/*  dex, lock, intl, def */

    return;
}

struct mjolchar *
mjolmkchar(long type)
{
    struct mjolchar *data = calloc(1, sizeof(struct mjolchar));

    if (!data) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    data->data.type = type;

    return data;
}

struct mjolchar *
mjolmkplayer(void)
{
    struct mjolchar *player = mjolmkchar(MJOL_CHAR_PLAYER);

    mjolinitchar(player, 1);
    mjolplayer = player;

    return player;
}

/* determine how many continuous turns a character has */
long
mjolhasnturn(struct mjolchar *chardata)
{
    long          retval = 0;
    long          speed = chardata->speed;
    unsigned long turn;

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

    return retval;
}

long
mjoldoturn(struct mjolgame *game, struct mjolchar *data)
{
    long          retval = 0;
    long          n = mjolhasnturn(data);
    int         (*printmsg)(const char *, ...) = game->scr->printmsg;
    int         (*getkbd)(void) = game->scr->getch;
    mjolcmdfunc  *func;
    int           cmd;
    int           dir;
    int           item;
    
    if (!n) {
        printmsg("You cannot move\n");
    }
    while (n) {
        if (n > 1) {
            printmsg("You have %ld turns\n", n);
        }
        do {
            cmd = getkbd();
        } while (cmd > 0xff);
        printmsg(" %x\n", cmd);
//            clrmsg();
        if (mjolhasdir(cmd)) {
            printmsg("Which direction?");
            do {
                dir = getkbd();
            } while (dir > 0xff);
        }
        if (mjolhasarg(cmd)) {
            do {
                item = getkbd();
            } while (item > 0xff);
        }
        func = mjolcmdfunctab[cmd][item];
        if (func) {
            retval += func(data, NULL);
            n--;
        }
    }
    
    return retval;
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
             hitfunc *func, long mindist)
{
    long               retval = 0;
    struct mjolchar ***chartab = mjolgame->chartab[mjolgame->lvl];
    struct mjolobj  ***objtab = mjolgame->objtab[mjolgame->lvl];
    struct mjolchar   *chr;
    struct mjolobj    *obj;
    struct mjolobj    *item;
    long               destx = dest->data.x;
    long               desty = dest->data.y;
    long               srcx = src->data.x;
    long               srcy = src->data.y;
    long               dx = destx - srcx;
    long               dy = desty - srcy;
    long               type;
    long               val;

    if (srcx == destx && srcy == desty) {

        return retval;
    }
    if (((labs(dx) == 1 && labs(dy) <= 1)
         || (labs(dy) == 1 && labs(dx) <= 1))
        && (func)) {
        /* attack */
        retval = func(src, dest);
    } else {
        /* TODO: make collision-checking/path-finding actually work... =) */
        if (labs(dx) > mindist) {
            if  (dx < -mindist) {
                /* dest is to the left of src */
                dx = -1;
            } else {
                /* dest is to the right of src */
                dx = 1;
            }
            srcx += dx;
            if (dy < -mindist) {
                /* dest is above src */
                dy = -1;
            } else if (dy > mindist) {
                /* dest is below src */
                dy = 1;
            }
            chr = chartab[srcy + dy][srcx];
            type = chr->data.type;
            if (mjolcanmoveto(type)) {
                /* src can moves horizontally and vertically */
                srcy += dy;
            } else {
                chr = chartab[srcy][srcx];
                type = chr->data.type;
                if (mjolcanmoveto(type)) {
                    /* src moves horizontally but not vertically */
                    ;
                } else {
                    srcx -= dx;
                    chr = chartab[srcy + dy][srcx];
                    type = chr->data.type;
                    if (mjolcanmoveto(type)) {
                        /* src moves vertically but not horizontally */
                        srcy += dy;
                    }
                }
            }
        } else if (labs(dy) > mindist) {
            /* vertical movement only */
            if (dy < -mindist) {
                dy = -1;
            } else {
                dy = 1;
            }
            srcy += dy;
            chr = chartab[srcy][srcx];
            type = chr->data.type;
            if (mjolcanmoveto(type)) {
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
                chr = chartab[srcy][srcx];
                type = chr->data.type;
                if (mjolcanmoveto(type)) {
                    /* move into chosen direction */
                    ;
                } else {
                    /* change horizontal direction */
                    dx = -dx;
                    srcx += dx << 1;
                    chr = chartab[srcy][srcx];
                    type = chr->data.type;
                    if (mjolcanmoveto(type)) {
                        /* valid move */
                        ;
                    } else {
                        /* change vertical direction */
                        srcx -= dx;
                        dy = -dy;
                        srcy += dy << 1;
                        chr = chartab[srcy][srcx];
                        type = chr->data.type;
                        if (mjolcanmoveto(type)) {
                            /* valid move */
                            ;
                        } else {
                            /* try vertical and horizontal */
                            srcx -= dx;
                            chr = chartab[srcy][srcx];
                            type = chr->data.type;
                            if (mjolcanmoveto(type)) {
                                /* valid move */
                                ;
                            } else {
                                srcx += 2 * dx;
                                chr = chartab[srcy][srcx];
                                type = chr->data.type;
                                if (mjolcanmoveto(type)) {
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
        if (!mindist && srcx == destx && srcy == desty
            && !(src->data.flg & MJOL_CHAR_NO_PICK)) {
            item = objtab[destx][desty];
            while (item) {
                type = item->data.type;
                if (mjolcanpickup(type)) {
                    obj = item->data.next;
                    if (obj) {
                        obj->data.prev = item->data.prev;
                    }
                    obj = item->data.prev;
                    if (obj) {
                        obj->data.next = item->data.next;
                    } else {
                        chartab[destx][desty] = item->data.next;
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
            objtab[src->data.y][src->data.x] = src->data.next;
        }
        src->data.prev = NULL;
        src->data.next = objtab[srcy][srcx];
        obj = src->data.next;
        if (obj) {
            obj->data.prev = src;
        }
        src->data.x = srcx;
        src->data.y = srcy;
        chartab[srcy][srcx] = src;
    }

    return retval;
}

long
mjolchase(struct mjolchar *src, struct mjolchar *dest)
{
    long retval = mjolfindmove(src, dest, mjolhit, 1);

    return retval;
}

long
mjolchaseall(struct mjolgame *game)
{
    long             retval = 0;
    struct mjolchar *src = mjolchaseq;

    while (src) {
        retval += mjolchase(src, mjolplayer);
        src = src->data.next;
    }

    return retval;
}

