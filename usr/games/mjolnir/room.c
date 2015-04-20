#include <stdio.h>
#include <stdlib.h>
#include <mjolnir/conf.h>
#if (MJOL_HASH)
#include <zero/hash.h>
#include <zero/trix.h>
#endif
#if (MJOL_USE_ZERO_RANDMT32)
#include <zero/randmt32.h>
#endif
#include <zero/trix.h>
#include <mjolnir/mjol.h>

#define MJOL_ROOM_MIN_WIDTH  8
#define MJOL_ROOM_MIN_HEIGHT 6
#define MJOL_ROOM_MAX_WIDTH  16
#define MJOL_ROOM_MAX_HEIGHT 12
#define MJOL_MIN_ROOMS       4
#define MJOL_MAX_ROOMS       8

extern struct mjolobj * mjolmkcorridor(void);
extern struct mjolobj * mjolmkdoor(void);
extern struct mjolobj * mjolmkhorizwall(void);
extern struct mjolobj * mjolmkvertwall(void);

void
mjolmkroom(struct mjolroom *room)
{
    struct mjolobj ***objtab = mjolgame->objtab[mjolgame->lvl];
    long              x;
    long              y;
    long              w;
    long              h;
    long              lim1;

    x = mjolrand() % (room->width >> 2);
    x = min(x, room->width - MJOL_ROOM_MIN_WIDTH - 2);
    x = max(x, 1);
    y = mjolrand() % (room->height >> 2);
    y = min(y, room->height - MJOL_ROOM_MIN_HEIGHT - 2);
    y = max(y, 1);
#if 0
    x = mjolrand() % max(room->width - MJOL_ROOM_MIN_WIDTH - 1, 1);
    x = max(x, 1);
    y = mjolrand() % max(room->height - MJOL_ROOM_MIN_HEIGHT - 1, 1);
    y = max(y, 1);
#endif
    w = MJOL_ROOM_MIN_WIDTH + (mjolrand() % (room->width >> 1));
    w = max(w, MJOL_ROOM_MIN_WIDTH);
    w = min(w, room->width - x - 2);
    w = min(w, MJOL_ROOM_MAX_WIDTH);
    h = MJOL_ROOM_MIN_HEIGHT + (mjolrand() % (room->height >> 1));
    h = max(h, MJOL_ROOM_MIN_HEIGHT);
    h = min(h, room->height - y - 2);
    h = min(h, MJOL_ROOM_MAX_HEIGHT);
    room->x += x;
    room->y += y;
    room->width = w;
    room->height = h;
    /* draw top wall */
    y = room->y;
    lim1 = room->x + w;
    for (x = room->x ; x < lim1 ; x++) {
        objtab[y][x] = mjolmkhorizwall();
    }
    /* draw bottom wall */
    y = room->y + h - 1;
    lim1 = room->x + w;
    for (x = room->x ; x < lim1 ; x++) {
        objtab[y][x] = mjolmkhorizwall();
    }
    /* draw left wall */
    x = room->x;
    lim1 = room->y + h - 1;
    for (y = room->y + 1 ; y < lim1 ; y++) {
        objtab[y][x] = mjolmkvertwall();
    }
    /* draw right wall */
    x = room->x + w - 1;
    lim1 = room->y + h - 1;
    for (y = room->y + 1 ; y < lim1 ; y++) {
        objtab[y][x] = mjolmkvertwall();
    }
    
    return;
}

void
mjolconnrooms(struct mjolgame *game,
              struct mjolroom *src, struct mjolroom *dest)
{
    struct mjolobj ***objtab = game->objtab[game->lvl];
    long              x;
    long              y;
    long              lim;
    long              val;
    long              delta;
    
    if (dest->y + dest->height < src->y) {
        /* dest is above src */
        delta = src->x - dest->x - dest->width;
        if (delta <= 0) {
            /* topleft, not adjacent */
            lim = dest->x + max(mjolrand() % dest->width, 1);
            x = src->x;
            y = src->y + max(mjolrand() % src->height, 1);
            objtab[y][x] = mjolmkdoor();
            while (--x > lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            lim = dest->y + dest->height;
            while (--y > lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            objtab[y][x] = mjolmkdoor();
        } else if (delta) {
            /* top, adjacent */
            lim = dest->x + max(mjolrand() % delta, 1);
            x = src->x + max(mjolrand() % delta, 1);
            y = src->y + src->height;
            objtab[y][x] = mjolmkdoor();
            while (--y > lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            objtab[y][x] = mjolmkdoor();
        } else if (dest->x >= src->x + src->width) {
            /* topright, not adjacent */
            lim = dest->x + max(mjolrand() % dest->width, 1);
            /* draw vertical line up */
            x = src->x + max(mjolrand() % src->width, 1);
            y = src->y + src->height;
            objtab[y][x] = mjolmkdoor();
            while (--y < lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            /* draw horizontal line right */
            lim = dest->x;
            while (--x > lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            objtab[y][x] = mjolmkdoor();
        }
    } else {
        /* dest is above src */
        val = src->x - dest->x - dest->width;
        if (val > 1) {
            /* adjacent, draw straight vertical line */
            lim = dest->y;
            x = dest->x + max(mjolrand() % dest->width, 1);
            y = src->y;
            objtab[y][x] = mjolmkdoor();
            while (++y < lim) {
                objtab[y][x] = mjolmkcorridor();
            }
            objtab[y][x] = mjolmkdoor();
        } else {
            /* draw horizontal line */
            lim = dest->x + max(mjolrand() % dest->width, 1);
            x = src->x + max(mjolrand() % src->width, 1);
            y = src->y;
            objtab[y][x] = mjolmkdoor();
            while (--x > lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            /* draw vertical line */
            lim = dest->y + dest->height;
            while (--y > lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
        }
        objtab[y][x] = mjolmkdoor();
    }

    return;
}

void
mjolinitroom(struct mjolgame *game, struct mjolroom *room)
{
    struct mjolobj ***objtab = game->objtab[game->lvl];
    long x;
    long y;
    long lim;

    /* top wall */
    y = room->y;
    x = room->x;
    lim = x + room->width;
    while (x < lim) {
        if (!objtab[y][x]) {
            objtab[y][x] = mjolmkhorizwall();
        }
        x++;
    }
    /* bottom wall */
    y = room->y + room->height;
    x = room->x;
    lim = x + room->width;
    while (x < lim) {
        if (!objtab[y][x]) {
            objtab[y][x] = mjolmkhorizwall();
        }
        x++;
    }
    /* left wall */
    y = room->y;
    x = room->x;
    lim = y + room->height;
    while (++y < lim) {
        if (!objtab[y][x]) {
            objtab[y][x] = mjolmkvertwall();
        }
    }
    /* right wall */
    y = room->y;
    x = room->x + room->width;
    lim = y + room->height;
    while (++y < lim) {
        if (!objtab[y][x]) {
            objtab[y][x] = mjolmkvertwall();
        }
    }

    return;
}

#define MJOL_DIR_HORIZONTAL  0
#define MJOL_DIR_VERTICAL    1
void
mjolsplitroom(struct mjolroom *room)
{
    long pos;
    
    if (!room) {
        fprintf(stderr, "memory allocation failure\n");
        
        exit(1);
    }
    if (room->dir == MJOL_DIR_VERTICAL) {
      //        pos = MJOL_ROOM_MIN_WIDTH + (mjolrand() % max(room->width, MJOL_ROOM_MIN_WIDTH));
      //        pos = min(pos, room->width >> 1);
#if 0
        pos = mjolrand() % room->width;
        pos = max(pos, room->width - MJOL_ROOM_MIN_WIDTH - 2);
#endif
        pos = room->width >> 1;
//        pos = room->width >> 1;
//        pos = max(pos, MJOL_ROOM_MIN_WIDTH);
        room->left = calloc(1, sizeof(struct mjolroom));
        room->right = calloc(1, sizeof(struct mjolroom));
        if (!room->left || !room->right) {
            fprintf(stderr, "memory allocation failure\n");
            
            exit(1);
        }
        room->left->x = room->x;
        room->left->y = room->y;
        room->left->width = pos;
        room->left->height = room->height;
        room->left->dir = MJOL_DIR_HORIZONTAL;
        room->right->x = room->x + pos;
        room->right->y = room->y;
        room->right->width = room->width - pos;
        room->right->height = room->height;
        room->right->dir = MJOL_DIR_HORIZONTAL;
    } else {
#if 0
        pos = mjolrand() % room->height;
        pos = max(pos, room->height - MJOL_ROOM_MIN_HEIGHT - 2);
#endif
        pos = room->height >> 1;
//        pos = max(pos, MJOL_ROOM_MIN_HEIGHT);
        room->left = calloc(1, sizeof(struct mjolroom));
        room->right = calloc(1, sizeof(struct mjolroom));
        if (!room->left || !room->right) {
            fprintf(stderr, "memory allocation failure\n");
            
            exit(1);
        }
        room->left->x = room->x;
        room->left->y = room->y;
        room->left->width = room->width;
        room->left->height = pos;
        room->left->dir = MJOL_DIR_VERTICAL;
        room->right->x = room->x;
        room->right->y = room->y + pos;
        room->right->width = room->width;
        room->right->height = room->height - pos;
        room->right->dir = MJOL_DIR_VERTICAL;
    }

    return;
}

#if 0
struct mjolroom **
mjolinitrooms(struct mjolgame *game, long *nret)
{
//    struct mjolroom  *tree = calloc(1, sizeof(struct mjolroom));
//    struct mjolroom  *tab[MJOL_MAX_ROOMS << 1];
    struct mjolroom  *room = calloc(1, sizeof(struct mjolroom));
    long              n = MJOL_MIN_ROOMS + (mjolrand()
                                            % (MJOL_MAX_ROOMS
                                               - MJOL_MIN_ROOMS));
//    struct mjolroom **ret = calloc(n, sizeof(struct mjolroom *));
    long              num = 0;
    long              max;
    long              lim;
    long              ndx;
#if (MJOL_HASH)
    long              val;
#endif
//    long              min;
//    long              max;
    struct mjolroom **ret = calloc(n, sizeof(struct mjolroom *));
    struct mjolroom **stk;
    struct mjolroom **tab = calloc(n << 1, sizeof(struct mjolroom **));
    long              ndx1;
    long              ndx2;

    if (!ret || !tab || !room) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    fprintf(stderr, "generating %ld rooms\n", n);
    /* split the dungeon */
    room->x = 0;
    room->y = 0;
    room->width = game->width;
    room->height = game->height;
    room->dir = mjolrand() & 0x01;
    mjolsplitroom(room);
    num = 2;
    stk[1] = room->left;
    stk[2] = room->right;
    lim = (n << 1) + 2;
    ndx = 1;
    ndx1 = 2;
    ndx2 = 3;
    max = 2; // ndx2 - ndx;
//    while (max < n) {
    while (ndx1 < lim) {
        long two = 2;

        fprintf(stderr, "SPLIT: MAX == %ld, NDX == %ld, NDX1 == %ld, NDX2 == %ld\n", max, ndx, ndx1, ndx2);
        room = stk[ndx];
        mjolsplitroom(room);
        stk[ndx1] = room->left;
        stk[ndx2] = room->right;
        num++;
        ndx++;
        max++;
        ndx1 += two;
        ndx2 += two;
    }
    *nret = n;
    num = 0;
    max = ndx2 - 2;
//    while (ndx <= max) {
    while (num <= n) {
        fprintf(stderr, "NDX == %ld, MAX == %ld, NUM == %ld\n", ndx, max, num);
        room = stk[ndx];
        mjolmkroom(room);
        tab[num] = room;
        num++;
        ndx++;
    }
#if 0
    max = ndx2 - 2;
    lim = max - ndx + 1;
    while (--n) {
        val = mjolrand();
//        num = ndx + (hashq128(&val, sizeof(long), 8 * sizeof(uint32_t)) % lim);
        num = ndx + (val % lim);
        fprintf(stderr, "NUM == %ld (MAX == %ld, NDX == %ld, LIM == %ld\n",
                num, max, ndx, lim);
        room = stk[num];
        lim--;
        if (num != max) {
            stk[num] = stk[max];
        }
        ret[n] = room;
        mjolmkroom(room);
        max--;
    }
    fprintf(stderr, "NUM == %ld (MAX == %ld, NDX == %ld, LIM == %ld\n",
            num, max, ndx, lim);
    room = stk[max];
    ret[n] = room;
    mjolmkroom(room);
#endif
    mjolprintlvl(game, game->lvl);

    return ret;
}
#endif

struct mjolroom **
mjolinitrooms(struct mjolgame *game, long *nret)
{
//    struct mjolroom  *tree = calloc(1, sizeof(struct mjolroom));
//    struct mjolroom  *tab[MJOL_MAX_ROOMS << 1];
    struct mjolroom  *room = calloc(1, sizeof(struct mjolroom));
    long              n = MJOL_MIN_ROOMS + (mjolrand()
                                            % (MJOL_MAX_ROOMS
                                               - MJOL_MIN_ROOMS
                                               + 1));
//    struct mjolroom **ret = calloc(n, sizeof(struct mjolroom *));
    long              num;
//    long              lim = n + (n & 0x01);
    long              lim = (n << 1) + 2;
#if 0
    long              max;
#endif
    long              ndx;
#if (MJOL_HASH)
    long              val;
#endif
//    long              min;
//    long              max;
    struct mjolroom **tab = calloc(n, sizeof(struct mjolroom *));
    struct mjolroom **stk = calloc((lim << 1) + 2, sizeof(struct mjolroom *));
    long              ndx1;
    long              ndx2;

    if (!tab || !stk || !room) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    fprintf(stderr, "generating %ld rooms\n", n);
    /* split the dungeon */
    room->x = 0;
    room->y = 0;
    room->width = game->width;
    room->height = game->height;
    room->dir = mjolrand() & 0x01;
    mjolsplitroom(room);
//    num = 2;
    stk[0] = room->left;
    stk[1] = room->right;
//    lim = (n << 1);
//    lim = (n << 1) + 2;
    num = 2;
    ndx = 0;
    ndx1 = 0;
    ndx2 = 1;
    while (num < n) {
        ndx1 += 2;
        ndx2 += 2;
        fprintf(stderr, "#1: %ld -> %ld, %ld\n", ndx, ndx1, ndx2);
        room = stk[ndx];
        mjolsplitroom(room);
        stk[ndx1] = room->left;
        stk[ndx2] = room->right;
        num++;
        ndx++;
    }
    num = 0;
    while (num < n) {
        fprintf(stderr, "#2(%p): %ld -> %ld\n", room, ndx, num);
        room = stk[ndx];
        mjolmkroom(room);
        tab[num] = room;
        num++;
        ndx++;
    }
    *nret = n;
    mjolprintlvl(game, game->lvl);

    return tab;
}

void
mjolgendng(struct mjolgame *game)
{
    long               nlvl = game->nlvl;
    long              *nroomtab = calloc(nlvl, sizeof(long));
    struct mjolroom ***lvltab = calloc(nlvl, sizeof(struct mjolroom ***));
    struct mjolroom  **roomtab;
    long               nroom;
    long               lvl;

    if (!nroomtab) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    mjolsrand(time(NULL));
//    mjolsrand(~0L);
    for (lvl = 0 ; lvl < nlvl ; lvl++) {
        game->lvl = lvl;
        roomtab = mjolinitrooms(game, &nroom);
        if (!roomtab) {
            fprintf(stderr, "memory allocation failure\n");

            exit(1);
        }
        nroomtab[lvl] = nroom;
        lvltab[lvl] = roomtab;
    }
    game->lvl = 0;
    game->nroomtab = nroomtab;
    game->lvltab = lvltab;

    return;
}

