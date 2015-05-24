#ifndef __DUNGEON_API_H__
#define __DUNGEON_API_H__

/* user-settable cave parameter structure */
struct cellcaveparm {
    long rndval;        // random value
    long niter;         // # of times to visit cells
    long size;          // current # of cells in dungeon
    long minsize;       // minimum # of cells in dungeon
    long maxsize;       // maximum # of cells in dungeon
    long minrmsize;     // minimum room size
    long maxrmsize;     // maximum room size
    long closeprob;     // probability of closing a cell in %
    long nlimnbor;      // cells with <= this neighbors get closed
    long nrmnbor;       // cells with >= this empty neighbors get closed
    long nfillnbor;     // empty cells with >= this neighbors get opened
};

/* user-settable corridor parameter structure */
struct cellcorparm {
    long breakout;      // maximum # of tries to connect caves
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

#endif /* __DUNGEON_API_H__ */

