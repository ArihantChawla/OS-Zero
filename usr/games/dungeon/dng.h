#ifndef __DUNGEON_DNG_H__
#define __DUNGEON_DNG_H__

struct dngobj {
    size_t datasz;      // size of object data
    size_t nfunc;       // # of functions in functab
    long   type;        // object type (character, other object)
    void  *data;        // object data (attributes)
    void  *functab;     // object functions
};

#endif /* __DUNGEON_DNG_H__ */

