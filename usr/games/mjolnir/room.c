#include <stdio.h>
#include <stdlib.h>
#include <mjolnir/conf.h>
#if (MJOL_USE_ZERO_RANDMT32)
#include <zero/randmt32.h>
#endif
#include <zero/trix.h>
#include <mjolnir/mjol.h>

//#define MJOL_ROOM_MIN_DIM    4
#define MJOL_ROOM_MIN_WIDTH  8
#define MJOL_ROOM_MIN_HEIGHT 6
#define MJOL_MIN_ROOMS       4
#define MJOL_MAX_ROOMS       16

extern struct mjolobj * mjolmkcorridor(void);
extern struct mjolobj * mjolmkdoor(void);
extern struct mjolobj * mjolmkhorwall(void);
extern struct mjolobj * mjolmkvertwall(void);

void
mjolmkroom(struct mjolroom *room)
{
    long x;
    long y;
    long w;
    long h;

    x = mjolrand() % room->width;
    x = max(x, room->width + MJOL_ROOM_MIN_WIDTH);
    x = min(x, room->width >> 2);
    y = mjolrand() % room->height;
    y = max(y, room->height - MJOL_ROOM_MIN_HEIGHT);
    y = min(y, room->height >> 2);
    room->x += x;
    room->y += y;
    w = MJOL_ROOM_MIN_WIDTH
        + (mjolrand() % max(room->width - x - MJOL_ROOM_MIN_WIDTH, 1));
    h = MJOL_ROOM_MIN_HEIGHT
        + (mjolrand() % max(room->height - y - MJOL_ROOM_MIN_HEIGHT, 1));
    room->width = w;
    room->height = h;

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

    if (dest->y + dest->height < src->y) {
        /* dest is above src */
        if (dest->x + dest->width <= src->x) {
            /* topleft */
            lim = dest->x + max(mjolrand() % dest->width - dest->x, 1);
            x = src->x;
            y = src->y + max(mjolrand() % src->height - src->y, 1);
            objtab[y][x] = mjolmkdoor();
            while (x-- > lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            lim = dest->y + dest->height;
            while (y-- > lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            objtab[y][x] = mjolmkdoor();
        } else if (dest->x >= src->x + src->width) {
            /* topright */
            lim = dest->x + max(mjolrand() % dest->width - dest->x, 1);
            x = src->x + src->width;
            y = src->y + max(mjolrand() % src->height - src->y, 1);
            objtab[y][x] = mjolmkdoor();
            while (x++ < lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            lim = dest->y + dest->height;
            while (y-- > lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            objtab[y][x] = mjolmkdoor();
        } else {
            /* dest is above src */
            val = dest->x + dest->width - src->x;
            if (val > 1) {
                /* adjacent, draw straight vertical line */
                x = dest->x + dest->width - max(mjolrand() % val, 1);
                y = src->y;
                objtab[y][x] = mjolmkdoor();
            } else {
                /* draw horizontal line */
                lim = dest->x + max(mjolrand() % dest->width - dest->x, 1);
                x = src->x + max(mjolrand() % src->width - src->x, 1);
                y = src->y;
                objtab[y][x] = mjolmkdoor();
                y++;
                while (x-- > lim) {
                    if (!objtab[y][x]) {
                        objtab[y][x] = mjolmkcorridor();
                    }
                }
            }
            /* draw vertical line */
            lim = dest->y + dest->height;
            while (y-- > lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            objtab[y][x] = mjolmkdoor();
        }
    } else if (dest->x + dest->width <= src->x) {
        /* dest is to the left of src */
        if (dest->y >= src->y + src->height) {
            /* bottomleft */
            lim = dest->x + max(mjolrand() % dest->width - dest->x, 1);
            x = src->x;
            y = src->y + max(mjolrand() % src->height - src->y, 1);
            objtab[y][x] = mjolmkdoor();
            while (x-- > lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            lim = dest->y;
            while (y++ < lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            objtab[y][x] = mjolmkdoor();
        } else {
            /* left */
            val = dest->y + dest->height - src->y;
            if (val > 1) {
                /* adjacent, draw straight horizontal line */
                x = src->x;
                y = dest->y + dest->height - max(mjolrand() % val, 1);
                objtab[y][x] = mjolmkdoor();
            } else {
                /* draw vertical line */
                lim = dest->y + max(mjolrand() % dest->height - dest->y, 1);
                x = src->x + max(mjolrand() % src->width - src->x, 1);
                y = src->y;
                objtab[y][x] = mjolmkdoor();
                while (y++ < lim) {
                    if (!objtab[y][x]) {
                        objtab[y][x] = mjolmkcorridor();
                    }
                }
            }
            /* draw horizontal line */
            lim = dest->x + dest->width;
            while (x-- > lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            objtab[y][x] = mjolmkdoor();
        }
    } else if (dest->y > src->y + src->height) {
        /* dest is below src */
        if (src->x < dest->x + dest->width) {
            /* bottomright */
            lim = dest->x + max(mjolrand() % dest->width - dest->x, 1);
            x = src->x;
            y = src->y + max(mjolrand() % src->height - src->y, 1);
            objtab[y][x] = mjolmkdoor();
            while (x++ < lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            lim = dest->y;
            while (y++ < lim) {
                if (!objtab[y][x]) { 
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            objtab[y][x] = mjolmkdoor();
        } else {
            /* below, draw straight vertical line */
            lim = dest->y;
            x = src->x + max(mjolrand() % src->width - src->x, 1);
            y = src->y + src->height;
            objtab[y][x] = mjolmkdoor();
            while (y++ < lim) {
                if (!objtab[y][x]) {
                    objtab[y][x] = mjolmkcorridor();
                }
            }
            objtab[y][x] = mjolmkdoor();
        }
    } else {
        /* dest is to the right of src */
        val = dest->y + dest->height - src->y;
        if (val > 1) {
            /* adjacent, draw straight horizontal line */
            x = src->x;
            y = dest->y + dest->height - max(mjolrand() % val, 1);
            objtab[y][x] = mjolmkdoor();
            while (val--) {
                if (objtab[val][x]) {
                    objtab[val][x] = mjolmkcorridor();
                }
            }
        } else {
            /* draw vertical line */
            lim = dest->y + max(mjolrand() % dest->height - dest->y, 1);
            x = src->x + src->width;
            y = src->y + max(mjolrand() % src->height - src->y, 1);
            val = y;
            objtab[y][x] = mjolmkdoor();
            while (val--) {
                if (!objtab[val][x]) {
                    objtab[val][x] = mjolmkcorridor();
                }
            }
        }
        /* draw horizontal line */
        lim = dest->x;
        while (x++ < lim) {
            if (!objtab[y][x]) {
                objtab[y][x] = mjolmkcorridor();
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
            objtab[y][x] = mjolmkhorwall();
        }
        x++;
    }
    /* bottom wall */
    y = room->y + room->height;
    x = room->x;
    lim = x + room->width;
    while (x < lim) {
        if (!objtab[y][x]) {
            objtab[y][x] = mjolmkhorwall();
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
struct mjolroom *
mjolsplitroom(struct mjolroom *room)
{
    struct mjolroom *ret = calloc(1, sizeof(struct mjolroom));
    long             dir = mjolrand() & 0x01;
    long             pos;

    if (!ret) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    if (dir == MJOL_DIR_HORIZONTAL) {
        pos = MJOL_ROOM_MIN_WIDTH + (mjolrand() % room->width);
        pos = min(pos, room->width >> 1);
        ret->left = calloc(1, sizeof(struct mjolroom));
        ret->right = calloc(1, sizeof(struct mjolroom));
        if (!ret->left || !ret->right) {
            fprintf(stderr, "memory allocation failure\n");

            exit(1);
        }
        ret->left->x = room->x;
        ret->left->y = room->y;
        ret->left->width = pos;
        ret->left->height = room->height;
        ret->right->x = pos;
        ret->right->y = room->y;
        ret->right->width = room->width - pos;
        ret->right->height = room->height;
    } else {
        pos = MJOL_ROOM_MIN_HEIGHT + (mjolrand() % room->height);
        pos = min(pos, room->height >> 1);
        ret->left = calloc(1, sizeof(struct mjolroom));
        ret->right = calloc(1, sizeof(struct mjolroom));
        if (!ret->left || !ret->right) {
            fprintf(stderr, "memory allocation failure\n");

            exit(1);
        }
        ret->left->x = room->x;
        ret->left->y = room->y;
        ret->left->width = room->width;
        ret->left->height = pos;
        ret->right->x = room->x;
        ret->right->y = pos;
        ret->right->width = room->width;
        ret->right->height = room->height - pos;
    }

    return ret;
}

struct mjolroom **
mjolinitrooms(struct mjolgame *game, long *nroom)
{
    struct mjolroom  *tab[MJOL_MAX_ROOMS << 1];
    struct mjolroom  *item = calloc(1, sizeof(struct mjolroom));
    long              n = MJOL_MIN_ROOMS + (mjolrand()
                                            % (MJOL_MAX_ROOMS
                                               - MJOL_MIN_ROOMS));
    struct mjolroom **ret = calloc(n, sizeof(struct mjolroom *));
    long              ndx = 0;
    long              val = 0;
    long              l;
    long              m;

    l = n;
    n++;
    if (!ret || !item) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    /* split the dungeon */
    item->x = 0;
    item->y = 0;
    item->width = game->width;
    item->height = game->height;
    item = mjolsplitroom(item);
    *nroom = n;
    tab[0] = item;
    tab[1] = item->left;
    mjolmkroom(item->left);
    if (l) {
        tab[2] = item->right;
        mjolmkroom(item->right);
        mjolconnrooms(game, item->left, item->right);
        if (l > 1) {
            val = l;
            ndx = l << 1;
            for (n = 2 ; n <= l ; n++) {
                m = n << 1;
                item = mjolsplitroom(tab[n - 1]);
                tab[m - 1] = item->left;
                mjolmkroom(item->left);
                tab[m] = item->right;
                mjolmkroom(item->right);
                mjolconnrooms(game, item->left, item->right);
            }
        } else {
            val = 1;
            ndx = 2;
        }
    } else {
        val = 1;
        ndx = 1;
    }
    /* build room table */
    l = 0;
    while (ndx >= val) {
        ret[l] = tab[ndx];
        l++;
        ndx--;
    }
    while (ndx-- >= 0) {
        free(tab[ndx]);
    }

    return ret;
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
    mjolsrand(~1);
    for (lvl = 0 ; lvl < nlvl ; lvl++) {
        game->lvl = lvl;
        roomtab = mjolinitrooms(game, &nroom);
        if (!roomtab) {
            fprintf(stderr, "memory allocation failure\n");

            exit(1);
        }
        nroomtab[lvl] = nroom;
        lvltab[lvl] = roomtab;
#if (MJOLDEBUG)
        mjolprintlvl(roomtab);
#endif
    }
    game->lvl = 0;
    game->nroomtab = nroomtab;
    game->lvltab = lvltab;

    return;
}

