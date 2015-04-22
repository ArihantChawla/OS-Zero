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
    long              delta;
    long              val;

    if (src->y < dest->y) {
        /* top of src is above dest */
        if (src->x < dest->x) {
            /* left of src is to the left of dest */
            if (dest->x < src->x + src->width - 2) {
                /* src has horizontal items adjacent with dest */
                delta = src->x + src->width - dest->x - 1;
                lim = dest->y;
                val = mjolrand() % delta;
                x = dest->x + max(val, 1);
                y = src->y + src->height - 1;
                /* draw vertical line */
                fprintf(stderr, "DELTA == %ld, X == %ld, Y == %ld, LIM == %ld\n",
                        delta, x, y, lim);
                fprintf(stderr, "SRC: ");
                mjolprintroom(src);
                fprintf(stderr, "DEST: ");
                mjolprintroom(dest);
                free(objtab[y][x]);
                objtab[y][x] = mjolmkdoor();
                while (++y < lim) {
                    if (!objtab[y][x]) {
                        objtab[y][x] = mjolmkcorridor();
                    }
                }
                free(objtab[y][x]);
                objtab[y][x] = mjolmkdoor();
            } else {
                /* src is completely to the left of dest */
                if (dest->y < src->y + src->height - 2) {
                    /* src has vertical items adjacent with dest */
                    delta = src->y + src->height - dest->y - 1;
                    lim = dest->x;
                    val = mjolrand() % delta;
                    x = src->x + src->width - 1;
                    y = dest->y + max(val, 1);
                    /* draw horizontal line */
                    fprintf(stderr, "DELTA == %ld, X == %ld, Y == %ld, LIM == %ld\n",
                            delta, x, y, lim);
                    fprintf(stderr, "SRC: ");
                    mjolprintroom(src);
                    fprintf(stderr, "DEST: ");
                    mjolprintroom(dest);
                    free(objtab[y][x]);
                    objtab[y][x] = mjolmkdoor();
                    while (++x < lim) {
                        if (!objtab[y][x]) {
                            objtab[y][x] = mjolmkcorridor();
                        }
                    }
                    free(objtab[y][x]);
                    objtab[y][x] = mjolmkdoor();
                } else {
                    /* src is completely above and to the left of dest */
                    val = mjolrand() % (dest->height - 1);
                    lim = dest->y + max(val, 2);
                    val = mjolrand() % (src->width - 1);
                    x = src->x + max(val, 1);
                    y = src->y + src->height - 1;
                    /* draw vertical line */
                    free(objtab[y][x]);
                    objtab[y][x] = mjolmkdoor();
                    while (++y < lim) {
                        if (!objtab[y][x]) {
                            objtab[y][x] = mjolmkcorridor();
                        }
                    }
                    if (!objtab[y][x]) {
                        objtab[y][x] = mjolmkcorridor();
                    }
                    /* draw horizontal line */
                    lim = dest->x;
                    while (++x < lim) {
                        if (!objtab[y][x]) {
                            objtab[y][x] = mjolmkcorridor();
                        }
                    }
                    free(objtab[y][x]);
                    objtab[y][x] = mjolmkdoor();
                }
            }
        } else if (src->x < dest->x + dest->width - 2) {
            /* src has horizontal items adjacent with dest */
            delta = dest->x + dest->width - src->x - 1;
            lim = dest->y;
            val = mjolrand() % delta;
            x = src->x + max(val, 1);
            y = src->y + src->height - 1;
            /* draw vertical line */
            fprintf(stderr, "DELTA == %ld, X == %ld, Y == %ld, LIM == %ld\n",
                    delta, x, y, lim);
            fprintf(stderr, "SRC: ");
            mjolprintroom(src);
            fprintf(stderr, "DEST: ");
            mjolprintroom(dest);
            free(objtab[y][x]);
            objtab[y][x] = mjolmkdoor();
            while (++y < lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            free(objtab[y][x]);
            objtab[y][x] = mjolmkdoor();
        } else {
            /* src is completely to the right of dest */
            if (dest->y < src->y + src->height - 1) {
                /* src has vertical items adjacent with dest */
                delta = src->y + src->height - dest->y - 1;
                lim = src->x;
                val = mjolrand() % delta;
                x = dest->x + dest->width - 1;
                y = dest->y + max(val, 1);
                /* draw horizontal line */
                free(objtab[y][x]);
                objtab[y][x] = mjolmkdoor();
                while (++x < lim) {
                    if (!objtab[y][x]) {
                        objtab[y][x] = mjolmkcorridor();
                    }
                }
                free(objtab[y][x]);
                objtab[y][x] = mjolmkdoor();
            } else {
                /* src is completely above and to the right of dest */
                val = mjolrand() % (dest->height - 1);
                lim = dest->y + max(val, 2);
                x = src->x + max(val, 1);
                y = src->y + src->height - 1;
                /* draw vertical line */
                free(objtab[y][x]);
                objtab[y][x] = mjolmkdoor();
                while (++y < lim) {
                    if (!objtab[y][x]) {
                        objtab[y][x] = mjolmkcorridor();
                    }
                }
                /* draw horizontal line */
                lim = x;
                x = dest->x + dest->width - 1;
                while (++x < lim) {
                    if (!objtab[y][x]) {
                        objtab[y][x] = mjolmkcorridor();
                    }
                }
                free(objtab[y][x]);
                objtab[y][x] = mjolmkcorridor();
            }
        }
    } else if (src->y < dest->y + dest->height - 2) {
        /* top of src is below top of dest */
        /* src has vertical items adjacent with dest */
        if (src->x < dest->x) {
            /* src is to the left of dest */
            delta = min(dest->y + dest->height - src->y - 1, src->height - 1);
            lim = dest->x;
            x = src->x + src->width - 1;
            val = mjolrand() % delta;
            y = src->y + max(val, 1);
            /* draw horizontal line */
            fprintf(stderr, "DELTA == %ld, X == %ld, Y == %ld, LIM == %ld\n",
                    delta, x, y, lim);
            fprintf(stderr, "SRC: ");
            mjolprintroom(src);
            fprintf(stderr, "DEST: ");
            mjolprintroom(dest);
            free(objtab[y][x]);
            objtab[y][x] = mjolmkdoor();
            while (++x < lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            free(objtab[y][x]);
            objtab[y][x] = mjolmkdoor();
        } else {
            /* src is to the right of dest */
            delta = min(dest->y + dest->height - src->y, src->height - 1);
            lim = src->x;
            val = mjolrand() % delta;
            x = dest->x + dest->width - 1;
            y = src->y + max(val, 1);
            /* draw horizontal line */
            free(objtab[y][x]);
            objtab[y][x] = mjolmkdoor();
            while (++x < lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            free(objtab[y][x]);
            objtab[y][x] = mjolmkdoor();
        }
    } else {
        /* src is completely below dest */
        if (src->x < dest->x) {
            /* left of src is to the left of dest */
            if (dest->x < src->x + src->width - 2) {
                /* src has horizontal items adjacent with dest */
                delta = src->x + src->width - dest->x - 1;
                lim = dest->y + dest->width - 1;
                val = mjolrand() % delta;
                x = dest->x + max(val, 1);
                y = dest->y + dest->height - 1;
                /* draw vertical line */
                free(objtab[y][x]);
                objtab[y][x] = mjolmkdoor();
                while (++y < lim) {
                    if (!objtab[y][x]) {
                        objtab[y][x] = mjolmkcorridor();
                    }
                }
                free(objtab[y][x]);
                objtab[y][x] = mjolmkdoor();
            } else {
                /* src is completely to the left of dest */
                lim = dest->x + (mjolrand() % (dest->width - 1));
                x = src->x + src->width - 1;
                y = src->y + (mjolrand() % (src->height - 1));
                /* draw horizontal line */
                free(objtab[y][x]);
                objtab[y][x] = mjolmkdoor();
                while (++x < lim) {
                    if (!objtab[y][x]) {
                        objtab[y][x] = mjolmkcorridor();
                    }
                }
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
                lim = y;
                y = dest->y + dest->height - 1;
                /* draw vertical line */
                free(objtab[y][x]);
                objtab[y][x] = mjolmkdoor();
                while (++y < lim) {
                    if (!objtab[y][x]) {
                        objtab[y][x] = mjolmkcorridor();
                    }
                }
            }
        } else if (src->x < dest->x + dest->width - 2) {
            /* src has horizontal items adjacent with dest */
            delta = dest->x + dest->width - src->x - 1;
            lim = src->y;
            val = mjolrand() % delta;
            x = src->x + max(val, 1);
            y = dest->y + dest->height - 1;
            /* draw vertical line */
            free(objtab[y][x]);
            objtab[y][x] = mjolmkdoor();
            while (++y < lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            free(objtab[y][x]);
            objtab[y][x] = mjolmkdoor();
        } else {
            /* src is completely to the right of dest */
            lim = src->y + (mjolrand() % (src->height - 1));
            x = dest->x + (mjolrand() % (dest->width - 1));
            y = dest->y + dest->height - 1;
            /* draw vertical line */
            free(objtab[y][x]);
            objtab[y][x] = mjolmkdoor();
            while (++y < lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            free(objtab[y][x]);
            objtab[y][x] = mjolmkcorridor();
            lim = src->x;
            /* draw horizontal line */
            while (++x < lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            free(objtab[y][x]);
            objtab[y][x] = mjolmkdoor();
        }
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
#if 0
    long              n = 2;
#endif
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
    struct mjolroom  *room1 = calloc(1, sizeof(struct mjolroom));
    struct mjolroom  *room2;
    long              n = 2;
#if 0
    long              n = MJOL_MIN_ROOMS + (mjolrand()
                                            % (MJOL_MAX_ROOMS
                                               - MJOL_MIN_ROOMS
                                               + 1));
#endif
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

    if (!tab || !stk || !room1) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    fprintf(stderr, "generating %ld rooms\n", n);
    /* split the dungeon */
    room1->x = 0;
    room1->y = 0;
    room1->width = game->width;
    room1->height = game->height;
    room1->dir = mjolrand() & 0x01;
    mjolsplitroom(room1);
//    num = 2;
    stk[0] = room1->left;
    stk[1] = room1->right;
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
        room1 = stk[ndx];
        mjolsplitroom(room1);
        stk[ndx1] = room1->left;
        stk[ndx2] = room1->right;
        num++;
        ndx++;
    }
    room1 = stk[ndx];
    tab[0] = room1;
    mjolmkroom(room1);
    fprintf(stderr, "#(%p): %ld -> %ld\n", room1, ndx, num);
    num = 1;
    ndx++;
    while (num < n) {
        room2 = stk[ndx];
        fprintf(stderr, "#2(%p): %ld -> %ld\n", room2, ndx, num);
        mjolmkroom(room2);
        mjolconnrooms(game, room1, room2);
        tab[num] = room2;
        num++;
        ndx++;
        room1 = room2;
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

