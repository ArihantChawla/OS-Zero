#ifndef __DUNGEON_CELL_H__
#define __DUNGEON_CELL_H__

#define DNG_OBJMAPS 1

#include <dungeon/conf.h>
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
static __inline__ long
dngprobpct(void)
{
    long val = dngrand();
    long ret = modu100(val);

    return ret;
}

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
    long             id;
    long             size;
    char            *map;
    long             ncor;      // # of corridors
    long             ncormax;   // maximum # of (allocated) corridors
    struct cellcor **cortab;    // corridor structures
};

/* user-settable cave parameter structure */
struct cellcaveparm {
    long rndval;        // random value
    long niter;         // # of times to visit cells
    long size;          // current # of cells in dungeon
    long minsize;       // minimum # of cells in dungeon
    long maxsize;       // maximum # of cells in dungeon
#if 0
    long minrmsize;     // minimum room size
    long maxrmsize;     // maximum room size
#endif
    long closeprob;     // probability of closing a cell in %
    long nlimnbor;      // cells with <= this neighbors get closed
    long nrmnbor;       // cells with >= this empty neighbors get closed
    long nfillnbor;     // empty cells with >= this neighbors get opened
};

/* user-settable corridor parameter structure */
struct cellcorparm {
    long brkout;        // maximum # of tries to connect caves
    long spacing;       // minimum distance from a closed cell
    long minlen;        // minimum corridor length
    long maxlen;        // maximum corridor length
    long maxturn;       // maximum number of turns
};

#define CELL_GENPARM_INIT 0x00000001L
struct cellgenparm {
    long                flg;
    struct cellcaveparm caveparm;
    struct cellcorparm  corparm;
};

struct cellgenparm;
struct celldng {
    long                  width;        // dungeon width
    long                  height;       // dungeon height
#if 0
    char                 *map;          // dungeon cell-bitmap
#endif
#if (DNG_OBJMAP)
    dngobjfunc           *cellfunc;
    dngobjfunc           *corfunc;
    dngobj               *objmap;
#endif
    long                  ncave;        // # of caves
    long                  ncavemax;     // maximum # of (allocated) caves
    struct cellcave     **cavetab;      // cave structures
    long                 *caveidtab;    // map of cell-owner caves
    struct cellgenparm    genparm;
};

void cellsetdefparm(struct cellgenparm *parm);
void cellinitdng(struct celldng *dng, long ncave, long width, long height);
long cellbuilddng(struct celldng *dng, long nlvl);

#endif /* __DUNGEON_CELL_H__ */

