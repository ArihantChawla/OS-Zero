#ifndef __DUNGEON_DNG_H__
#define __DUNGEON_DNG_H__

#include <stddef.h>

/* data structures */
struct dnggame {
    char            *name;      // name of the game
    int              argc;      // cmdline argument count
    char           **argv;      // cmdline arguments
};

struct dngobj {
    long    id;                 // object ID
    long    type;               // character type
    long    flg;                // flag bits
    long    x;                  // level X-coordinate
    long    y;                  // level Y-coordinate
};

#endif /* __DUNGEON_DNG_H__ */

