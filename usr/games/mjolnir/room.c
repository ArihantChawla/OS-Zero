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
#define MJOL_MAX_ROOMS       8

extern struct mjolobj * mjolmkcorridor(void);
extern struct mjolobj * mjolmkdoor(void);

void
mjolmkroom(struct mjolrect *rect)
{
    long x;
    long y;
    long w;
    long h;

    x = mjolrand() % rect->width;
    x = max(x, rect->width - MJOL_ROOM_MIN_WIDTH);
    x = min(x, rect->width >> 2);
    y = mjolrand() % rect->height;
    y = max(y, rect->height - MJOL_ROOM_MIN_HEIGHT);
    y = min(x, rect->height >> 2);
    rect->x += x;
    rect->y += y;
    w = MJOL_ROOM_MIN_WIDTH
        + (mjolrand() % max(rect->width - x - MJOL_ROOM_MIN_WIDTH, 1));
    h = MJOL_ROOM_MIN_HEIGHT
        + (mjolrand() % max(rect->height - y - MJOL_ROOM_MIN_HEIGHT, 1));
    rect->width = w;
    rect->height = h;

    return;
}

void
mjolconnrooms(struct mjolgame *game,
              struct mjolrect *src, struct mjolrect *dest)
{
    struct mjolobj ***objtab = game->objtab;
    long              x;
    long              y;
    long              lim;
    long              val;

    if (dest->y + dest->height < src->y) {
        /* dest is above src */
        if (dest->x + dest->width <= src->x) {
            /* topleft */
            lim = dest->x + max(mjolrand() % dest->width, 1);
            x = src->x;
            y = src->y + max(mjolrand() % src->height, 1);
            objtab[x][y] = mjolmkdoor();
            while (x-- > lim) {
                if (!objtab[x][y]) {
                    objtab[x][y] = mjolmkcorridor();
                }
            }
            lim = dest->y + dest->height;
            while (y-- > lim) {
                if (!objtab[x][y]) {
                    objtab[x][y] = mjolmkcorridor();
                }
            }
            objtab[x][y] = mjolmkdoor();
        } else if (dest->x >= src->x + src->width) {
            /* topright */
            lim = dest->x + max(mjolrand() % dest->width, 1);
            x = src->x + src->width;
            y = src->y + max(mjolrand() % src->height, 1);
            objtab[x][y] = mjolmkdoor();
            while (x++ < lim) {
                if (!objtab[x][y]) {
                    objtab[x][y] = mjolmkcorridor();
                }
            }
            lim = dest->y + dest->height;
            while (y-- > lim) {
                if (!objtab[x][y]) {
                    objtab[x][y] = mjolmkcorridor();
                }
            }
            objtab[x][y] = mjolmkdoor();
        } else {
            /* dest is above src */
            val = dest->x + dest->width - src->x;
            if (val > 1) {
                /* adjacent, draw straight vertical line */
                x = dest->x + dest->width - max(mjolrand() % val, 1);
                y = src->y;
                objtab[x][y] = mjolmkdoor();
            } else {
                /* draw horizontal line */
                lim = dest->x + max(mjolrand() % dest->width, 1);
                x = src->x + max(mjolrand() % src->width, 1);
                y = src->y;
                objtab[x][y] = mjolmkdoor();
                y++;
                while (x-- > lim) {
                    if (!objtab[x][y]) {
                        objtab[x][y] = mjolmkcorridor();
                    }
                }
            }
            /* draw vertical line */
            lim = dest->y + dest->height;
            while (y-- > lim) {
                if (!objtab[x][y]) {
                    objtab[x][y] = mjolmkcorridor();
                }
            }
            objtab[x][y] = mjolmkdoor();
        }
    } else if (dest->x + dest->width <= src->x) {
        /* dest is to the left of src */
        if (dest->y >= src->y + src->height) {
            /* bottomleft */
            lim = dest->x + max(mjolrand() % dest->width, 1);
            x = src->x;
            y = src->y + max(mjolrand() % src->height, 1);
            objtab[x][y] = mjolmkdoor();
            while (x-- > lim) {
                if (!objtab[x][y]) {
                    objtab[x][y] = mjolmkcorridor();
                }
            }
            lim = dest->y;
            while (y++ < lim) {
                if (!objtab[x][y]) {
                    objtab[x][y] = mjolmkcorridor();
                }
            }
            objtab[x][y] = mjolmkdoor();
        } else {
            /* left */
            val = dest->y + dest->height - src->y;
            if (val > 1) {
                /* adjacent, draw straight horizontal line */
                x = src->x;
                y = dest->y + dest->height - max(mjolrand() % val, 1);
                objtab[x][y] = mjolmkdoor();
            } else {
                /* draw vertical line */
                lim = dest->y + max(mjolrand() % dest->height, 1);
                x = src->x + max(mjolrand() % src->width, 1);
                y = src->y;
                objtab[x][y] = mjolmkdoor();
                while (y++ < lim) {
                    if (!objtab[x][y]) {
                        objtab[x][y] = mjolmkcorridor();
                    }
                }
            }
            /* draw horizontal line */
            lim = dest->x + dest->width;
            while (x-- > lim) {
                if (!objtab[x][y]) {
                    objtab[x][y] = mjolmkcorridor();
                }
            }
            objtab[x][y] = mjolmkdoor();
        }
    } else if (dest->y > src->y + src->height) {
        /* dest is below src */
        if (src->x < dest->x + dest->width) {
            /* bottomright */
            lim = dest->x + max(mjolrand() % dest->width, 1);
            x = src->x;
            y = src->y + max(mjolrand() % src->height, 1);
            objtab[x][y] = mjolmkdoor();
            while (x++ < lim) {
                if (!objtab[x][y]) {
                    objtab[x][y] = mjolmkcorridor();
                }
            }
            lim = dest->y;
            while (y++ < lim) {
                if (!objtab[x][y]) { 
                    objtab[x][y] = mjolmkcorridor();
                }
            }
            objtab[x][y] = mjolmkdoor();
        } else {
            /* below, draw straight vertical line */
            lim = dest->y;
            x = src->x + max(mjolrand() % src->width, 1);
            y = src->y + src->height;
            objtab[x][y] = mjolmkdoor();
            while (y++ < lim) {
                if (!objtab[x][y]) {
                    objtab[x][y] = mjolmkcorridor();
                }
            }
            objtab[x][y] = mjolmkdoor();
        }
    } else {
        /* dest is to the right of src */
        val = dest->y + dest->height - src->y;
        if (val > 1) {
            /* adjacent, draw straight horizontal line */
            x = src->x;
            y = dest->y + dest->height - max(mjolrand() % val, 1);
            objtab[x][y] = mjolmkdoor();
        } else {
            /* draw vertical line */
            lim = dest->y + max(mjolrand() % dest->height, 1);
            x = src->x + src->width;
            y = src->y + max(mjolrand() % src->height, 1);
            objtab[x][y] = mjolmkdoor();
            while (y--) {
                if (!objtab[x][y]) {
                    objtab[x][y] = mjolmkcorridor();
                }
            }
        }
        /* draw horizontal line */
        lim = dest->x;
        while (x++ < lim) {
            if (!objtab[x][y]) {
                objtab[x][y] = mjolmkcorridor();
            }
        }
        objtab[x][y] = mjolmkdoor();
    }

    return;
}

#define MJOL_DIR_HORIZONTAL  0
#define MJOL_DIR_VERTICAL    1
struct mjolrect *
mjolsplitrect(struct mjolrect *rect)
{
    struct mjolrect *ret = calloc(1, sizeof(struct mjolrect));
    long             dir = mjolrand() & 0x01;
    long             pos;

    if (!ret) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    if (dir == MJOL_DIR_HORIZONTAL) {
        pos = MJOL_ROOM_MIN_WIDTH + (mjolrand() % rect->width);
        pos = min(pos, rect->width >> 1);
        ret->left = calloc(1, sizeof(struct mjolrect));
        ret->right = calloc(1, sizeof(struct mjolrect));
        if (!ret->left || !ret->right) {
            fprintf(stderr, "memory allocation failure\n");

            exit(1);
        }
        ret->left->x = rect->x;
        ret->left->y = rect->y;
        ret->left->width = pos;
        ret->left->height = rect->height;
        ret->right->x = pos;
        ret->right->y = rect->y;
        ret->right->width = rect->width - pos;
        ret->right->height = rect->height;
    } else {
        pos = MJOL_ROOM_MIN_HEIGHT + (mjolrand() % rect->height);
        pos = min(pos, rect->height >> 1);
        ret->left = calloc(1, sizeof(struct mjolrect));
        ret->right = calloc(1, sizeof(struct mjolrect));
        if (!ret->left || !ret->right) {
            fprintf(stderr, "memory allocation failure\n");

            exit(1);
        }
        ret->left->x = rect->x;
        ret->left->y = rect->y;
        ret->left->width = rect->width;
        ret->left->height = pos;
        ret->right->x = rect->x;
        ret->right->y = pos;
        ret->right->width = rect->width;
        ret->right->height = rect->height - pos;
    }

    return ret;
}

struct mjolrect **
mjolgenrooms(struct mjolgame *game, long *nroom, long width, long height)
{
    struct mjolrect  *tab[MJOL_MAX_ROOMS << 1] = { NULL };
    struct mjolrect  *item = calloc(1, sizeof(struct mjolrect));
    long              n = MJOL_MIN_ROOMS + (mjolrand()
                                            % (MJOL_MAX_ROOMS
                                               - MJOL_MIN_ROOMS));
    struct mjolrect **ret = calloc(n, sizeof(struct mjolrect *));
    long              ndx = 0;
    long              val = 0;
    long              l;
    long              m;

    l = n;
    n++;
    if (!ret || !tab || !item) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    /* split the dungeon */
    item->x = 0;
    item->y = 0;
    item->width = game->width;
    item->height = game->height;
    item = mjolsplitrect(item);
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
                item = mjolsplitrect(tab[n - 1]);
                tab[m - 1] = item->left;
                tab[m] = item->right;
                mjolmkroom(item->left);
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
    /* build rooms */
    n = 0;
    while (ndx >= val) {
        ret[n] = tab[ndx];
        ndx--;
        n++;
    }
    while (ndx >= 0) {
        free(tab[ndx]);
        ndx--;
    }

    return ret;
}

