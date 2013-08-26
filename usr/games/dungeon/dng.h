#ifndef __DUNGEON_DNG_H__
#define __DUNGEON_DNG_H__

/* data structures */

struct dnggame {
    char  *name;        // name of the game
    int    argc;        // cmdline argument count
    char **argv;        // cmdline arguments
    void  *objdata;     // per-game object data
};

struct dngchar {
    long    id;         // character ID
    long    type;       // character type
    long    flg;        // flag bits
    long    x;          // level X-coordinate
    long    y;          // level Y-coordinate
    size_t  objsz;      // size of object data
    void   *objdata;    // per-game object data
};

struct dngobj {
    long    id;         // object ID
    long    type;       // character type
    long    flg;        // flag bits
    long    x;          // level X-coordinate
    long    y;          // level Y-coordinate
    size_t  objsz;      // size of object data
    void   *objdata;    // per-game object data
};

#endif /* __DUNGEON_DNG_H__ */

