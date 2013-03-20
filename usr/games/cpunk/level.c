#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zero/randk.h>
#include <cpunk_level.h>
#include <cpunk_item.h>

/*
 * NOTES
 * -----
 * - code only exists within computers
 * - the only corridors are around levels
 * - doors and holes are currently unimplemented
 */
#define cpunkmkitem() (itemtab[rand() % (sizeof(itemtab))])

struct cpunklevel *
cpunknewlevel(void)
{
    struct cpunklevel *newlevel;
    unsigned char      item;
    long               x;
    long               y;

    newlevel = malloc(sizeof(struct cpunklevel));
    bzero(newlevel->items,
          CPUNK_LEVEL_WIDTH * CPUNK_LEVEL_HEIGHT * sizeof(unsigned char));
#if 0
    newlevel->items = calloc(CPUNK_LEVEL_WIDTH * CPUNK_LEVEL_HEIGHT,
                             sizeof(unsigned char));
#endif
    for (x = 0 ; x < CPUNK_LEVEL_WIDTH ; x++) {
        newlevel->items[0][x] = CPUNK_WALL;
        newlevel->items[CPUNK_LEVEL_HEIGHT - 1][x] = CPUNK_WALL;
    }
    for (y = 0 ; y < CPUNK_LEVEL_HEIGHT ; y++) {
        newlevel->items[y][0] = CPUNK_WALL;
        newlevel->items[y][CPUNK_LEVEL_WIDTH - 1] = CPUNK_WALL;
    }
    for (y = 1 ; y < CPUNK_LEVEL_HEIGHT - 1; y++) {
        for (x = 1 ; x < CPUNK_LEVEL_WIDTH - 1 ; x++) {
            item = cpunkmkitem();
            newlevel->items[x][y] = item;
        }
    }

    return newlevel;
}

