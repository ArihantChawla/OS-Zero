#if (TEST)
#include <assert.h>
#endif
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

#define MJOL_DIR_HORIZONTAL 0
#define MJOL_DIR_VERTICAL   1
#define MJOL_ROOM_MIN_DIM   4
#define MJOL_MIN_ROOMS      2
#define MJOL_MAX_ROOMS      8

struct mjolrect *
mjolsplitrect(struct mjolrect *rect)
{
    struct mjolrect *ret = calloc(1, sizeof(struct mjolrect));
    long             dir = randmt32() & 0x01;
    long             pos;

    assert(rect->width != 0);
    if (!ret) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    if (dir == MJOL_DIR_HORIZONTAL) {
        pos = MJOL_ROOM_MIN_DIM + (randmt32() % rect->width);
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
        pos = MJOL_ROOM_MIN_DIM + (randmt32() % rect->height);
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
    struct mjolrect **tab = calloc(MJOL_MAX_ROOMS << 1,
                                   sizeof(struct mjolrect *));
    struct mjolrect  *item = calloc(1, sizeof(struct mjolrect));
    struct mjolrect  *rect;
    long              n = MJOL_MIN_ROOMS + (randmt32()
                                            % (MJOL_MAX_ROOMS
                                               - MJOL_MIN_ROOMS));
    struct mjolrect **ret;
    long              ndx = 0;
    long              val = 0;
    long              l;
    long              x;
    long              y;
    long              w;
    long              h;

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
    l = n;
    tab[0] = item;
    tab[1] = item->left;
    if (l > 1) {
        tab[2] = item->right;
        if (l > 2) {
            /* TODO: try to replace the switch with a loop */
            switch (l) {
                case 3:
                    item = mjolsplitrect(tab[1]);
                    tab[3] = item->left;
                    tab[4] = item->right;
                    val = 2;
                    ndx = 4;

                    break;
                case 4:
                    item = mjolsplitrect(tab[1]);
                    tab[3] = item->left;
                    tab[4] = item->right;
                    item = mjolsplitrect(tab[2]);
                    tab[5] = item->left;
                    tab[6] = item->right;
                    val = 3;
                    ndx = 6;

                    break;
                case 5:
                    item = mjolsplitrect(tab[1]);
                    tab[3] = item->left;
                    tab[4] = item->right;
                    item = mjolsplitrect(tab[2]);
                    tab[5] = item->left;
                    tab[6] = item->right;
                    item = mjolsplitrect(tab[3]);
                    tab[7] = item->left;
                    tab[8] = item->right;
                    val = 4;
                    ndx = 8;

                    break;
                case 6:
                    item = mjolsplitrect(tab[1]);
                    tab[3] = item->left;
                    tab[4] = item->right;
                    item = mjolsplitrect(tab[2]);
                    tab[5] = item->left;
                    tab[6] = item->right;
                    item = mjolsplitrect(tab[3]);
                    tab[7] = item->left;
                    tab[8] = item->right;
                    item = mjolsplitrect(tab[4]);
                    tab[9] = item->left;
                    tab[10] = item->right;
                    val = 5;
                    ndx = 10;

                    break;
                case 7:
                    item = mjolsplitrect(tab[1]);
                    tab[3] = item->left;
                    tab[4] = item->right;
                    item = mjolsplitrect(tab[2]);
                    tab[5] = item->left;
                    tab[6] = item->right;
                    item = mjolsplitrect(tab[3]);
                    tab[7] = item->left;
                    tab[8] = item->right;
                    item = mjolsplitrect(tab[4]);
                    tab[9] = item->left;
                    tab[10] = item->right;
                    item = mjolsplitrect(tab[5]);
                    tab[11] = item->left;
                    tab[12] = item->right;
                    val = 6;
                    ndx = 12;

                    break;
                case 8:
                    item = mjolsplitrect(tab[1]);
                    tab[3] = item->left;
                    tab[4] = item->right;
                    item = mjolsplitrect(tab[2]);
                    tab[5] = item->left;
                    tab[6] = item->right;
                    item = mjolsplitrect(tab[3]);
                    tab[7] = item->left;
                    tab[8] = item->right;
                    item = mjolsplitrect(tab[4]);
                    tab[9] = item->left;
                    tab[10] = item->right;
                    item = mjolsplitrect(tab[5]);
                    tab[11] = item->left;
                    tab[12] = item->right;
                    item = mjolsplitrect(tab[6]);
                    tab[13] = item->left;
                    tab[14] = item->right;
                    val = 7;
                    ndx = 14;

                    break;
                default:
                    fprintf(stderr, "MJOL_MAX_ROOMS > 8 not supported\n");

                    exit(1);
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
    l = 0;
    while (ndx >= val) {
        rect = tab[ndx];
        ret[l] = rect;
        x = (randmt32() % rect->width) - MJOL_ROOM_MIN_DIM;
        x = max(x, 0);
        x = max(x, rect->width >> 1);
        y = (randmt32() % rect->height) - MJOL_ROOM_MIN_DIM;
        y = max(y, 0);
        y = max(x, rect->height >> 1);
        rect->x += x;
        rect->y += y;
        w = MJOL_ROOM_MIN_DIM + (randmt32()
                                 % max(rect->width - x - MJOL_ROOM_MIN_DIM, 1));
        h = MJOL_ROOM_MIN_DIM + (randmt32()
                                 % max(rect->height - y - MJOL_ROOM_MIN_DIM, 1));
        w = max(w, MJOL_ROOM_MIN_DIM);
        h = max(h, MJOL_ROOM_MIN_DIM);
        rect->width = w;
        rect->height = h;
        ndx--;
        l++;
    }

    return ret;
}

