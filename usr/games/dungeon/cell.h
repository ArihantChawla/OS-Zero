#ifndef __DUNGEON_CELL_H__
#define __DUNGEON_CELL_H__

#define DNG_OBJMAPS 1

#include <dungeon/conf.h>
#include <dungeon/api.h>
#if (DNG_RANDMT32)
#include <zero/randmt32.h>
#else
#include <stdlib.h>
#endif
#include <zero/trix.h>

#if (DNG_OBJMAPS)
typedef unsigned char dngobj;
typedef void          dngobjfunc(long, long);
#endif

/* REFERENCE: http://www.evilscience.co.uk/a-c-algorithm-to-build-roguelike-cave-systems-part-1/ */

#define DNG_WIDTH     80
#define DNG_HEIGHT    24

/* define pseudo-random number interface */
#if (DNG_RANDMT32)
#define dngsrand(val) srandmt32(val)
#define dngrand()     (randmt32() & 0x7fffffffU)
#else
#define dngsrand(val) srand()
#define dngrand()     (rand() & 0x7fffffffU)
#endif

/* macro to calculate percentual probabilities */
#define dngprobpct() (modu100(randmt32()))

/* directions for building and navigating dungeons */
#define DNG_NODIR     -1
#define DNG_NORTH      0
#define DNG_SOUTH      1
#define DNG_EAST       2
#define DNG_WEST       3
#define DNG_NDIR2      4
#define DNG_NORTHEAST  4
#define DNG_NORTHWEST  5
#define DNG_SOUTHWEST  6
#define DNG_SOUTHEAST  7
#define DNG_NDIR       8
//#define DNG_CENTER    8

/* value for uninitialised dungeon cells */
#define DNG_NOCAVE    -1

struct cellcoord {
    long xval;          // X-coordinate
    long yval;          // Y-coordinate
};

struct cellcor {
    long              n;        // number of cells in corridor
    long              nmax;     // number of allocated pnttab entries
    struct cellcoord *pnttab;   // table of points (X,Y-coordinate pairs)
};

struct cellcave {
    long            id;
    long            size;
};

struct celldng {
    long                  width;        // dungeon width
    long                  height;       // dungeon height
    char                 *map;          // dungeon cell-bitmap
#if (DNG_OBJMAP)
    dngobjfunc           *cellfunc;
    dngobjfunc           *corfunc;
    dngobj               *objmap;
#endif
    long                  ncave;        // # of caves
    long                  ncavemax;     // maximum # of (allocated) caves
    struct cellcave     **cavetab;      // cave structures
    long                  ncor;         // # of corridors
    long                  ncormax;      // maximum # of (allocated) corridors
    struct cellcor      **cortab;       // corridor structures
    long                 *caveidtab;    // map of cell-owner caves
    struct cellgenparm    genparm;
#if 0
    struct cellcaveparm   caveparm;
    struct cellcorparm    corparm;
#endif
};

void cellinitdng(struct celldng *dng, long ncave, long width, long height);
long cellbuilddng(struct celldng *dng, long nlvl);

#endif /* __DUNGEON_CELL_H__ */

