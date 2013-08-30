#include <stdio.h>
#include <stdlib.h>
#include <zero/randmt32.h>
#include <zero/trix.h>
#include <mjolnir/mjol.h>

void
mjolgenroom(struct mjolgame *game, struct mjolrect *rect)
{
    ;
}

#define MJOL_DIR_HORIZONTAL  0
#define MJOL_DIR_VERTICAL    1
#define MJOL_ROOM_MIN_DIM    4
#define MJOL_ROOM_MIN_WIDTH  8
#define MJOL_ROOM_MIN_HEIGHT 4
#define MJOL_MIN_ROOMS       2
#define MJOL_MAX_ROOMS       8

struct mjolrect *
mjolsplitrect(struct mjolrect *rect)
{
    struct mjolrect *ret = calloc(1, sizeof(struct mjolrect));
    long             dir = randmt32() & 0x01;
    long             pos;

    if (!ret) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    if (dir == MJOL_DIR_HORIZONTAL) {
        pos = MJOL_ROOM_MIN_WIDTH + (randmt32() % rect->width);
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
        pos = MJOL_ROOM_MIN_HEIGHT + (randmt32() % rect->height);
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
mjolgenlvl(struct mjolgame *game, long *nroom)
{
    struct mjolrect  *tab[MJOL_MAX_ROOMS << 1] = { NULL };
    struct mjolrect  *item = calloc(1, sizeof(struct mjolrect));
    struct mjolrect  *rect;
    long              n = MJOL_MIN_ROOMS + (randmt32()
                                            % (MJOL_MAX_ROOMS
                                               - MJOL_MIN_ROOMS));
    struct mjolrect **ret;
    long              ndx = 0;
    long              val = 0;
    long              l;
    long              m;
    long              x;
    long              y;
    long              w;
    long              h;

    l = n;
    n++;
    ret = calloc(n, sizeof(struct mjolrect *));
    if (!tab || !item || !ret) {
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
    if (l) {
        tab[2] = item->right;
        if (l > 1) {
            val = l;
            ndx = l << 1;
            for (n = 2 ; n <= l ; n++) {
                m = n << 1;
                item = mjolsplitrect(tab[n - 1]);
                tab[m - 1] = item->left;
                tab[m] = item->right;
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
        rect = tab[ndx];
        ret[n] = rect;
        x = (randmt32() % rect->width) - MJOL_ROOM_MIN_WIDTH;
        x = max(x, 0);
        x = max(x, rect->width >> 1);
        y = (randmt32() % rect->height) - MJOL_ROOM_MIN_HEIGHT;
        y = max(y, 0);
        y = max(x, rect->height >> 1);
        rect->x += x;
        rect->y += y;
        w = MJOL_ROOM_MIN_DIM + (randmt32()
                                 % max(rect->width - x - MJOL_ROOM_MIN_WIDTH,
                                       1));
        h = MJOL_ROOM_MIN_DIM + (randmt32()
                                 % max(rect->height - y - MJOL_ROOM_MIN_HEIGHT,
                                       1));
        w = max(w, MJOL_ROOM_MIN_DIM);
        h = max(h, MJOL_ROOM_MIN_DIM);
        rect->width = w;
        rect->height = h;
        ndx--;
        n++;
    }
    while (ndx >= 0) {
        free(tab[ndx]);
        ndx--;
    }

    return ret;
}

