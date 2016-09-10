#ifndef __DUNGEON_DNG_H__
#define __DUNGEON_DNG_H__

#include <dungeon/conf.h>
//#include <dungeon/cell.h>
#include <stddef.h>

/* data structures */
struct dnggame {
    char  *name;        // name of the game
    int    argc;        // cmdline argument count
    char **argv;        // cmdline arguments
};

struct dngobj {
    long  id;   // object ID
    long  type; // character type
    long  flg;  // flag bits
    long  x;    // level X-coordinate
    long  y;    // level Y-coordinate
    void *prev; // previous in queue, NULL for head item
    void *next; // next in queue, NULL for tail item
};

/* macros */
#define dngiscell(x, y, w, h)                                           \
    (((x) >= 0) && ((x) < (w)) && ((y) >= 0) && ((y) < h))
#define dnggetcellbit(dng, x, y)                                        \
    (bitset((dng)->map, (y) * (dng)->width + (x)))
#define dngsetcellbit(dng, x, y)                                        \
    (setbit((dng)->map, (y) * (dng)->width + x))
#define dngclrcellbit(dng, x, y)                                        \
    (clrbit((dng)->map, (y) * (dng)->width + x))
#define dnggetcorbit(dng, x, y)                                         \
    (bitset((dng)->cormap, (y) * (dng)->width + (x)))
#define dngsetcorbit(dng, x, y)                                         \
    (setbit((dng)->cormap, (y) * (dng)->width + x))
#define dngclrcorbit(dng, x, y)                                         \
    (clrbit((dng)->cormap, (y) * (dng)->width + x))
#define dngrevdir(dir) (dngrevdirtab[(dir)])
#define dngmovedir(x, y, dir)                                           \
    ((x) += dngdirofstab[(dir)].xval, (y) += dngdirofstab[(dir)].yval)
#define dngsetcaveid(dng, x, y, cid)                                    \
    ((dng)->caveidtab[(y) * (dng)->width + (x)] = (cid))
#define dnggetcaveid(dng, x, y)                                         \
    ((dng)->caveidtab[(y) * (dng)->width + (x)])

#endif /* __DUNGEON_DNG_H__ */

