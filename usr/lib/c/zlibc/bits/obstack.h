#ifndef __BITS_OBSTACK_H__
#define __BITS_OBSTACK_H__

struct _obstack_chunk {
    char                  *limit;       // 1 past end of chunk
    struct _obstack_chunk *prev;        // prior chunk or NULL
    char                   contents[4]; // objects begin
};

struct obstack {
    long                   chunk_size;  // preferred allocation size
    struct _obstack_chunk *chunk;       // current struct _obstack_chunk
    char                  *object_base; // address of built object
    char                  *next_free;   // offset of next character
    char                  *chunk_limit; // char after chunk
    union {
        PTR_INT_TYPE       tmpint;
        void              *tmpptr;
    } tmp;
    int                    alignment_mask; // mask of alignment for objects
    struct _obstack_chunk *(*chunkfun)(void *, long);
    void                   (*freefun)(void *, struct _obstack_chunk *);
    void                  *extra_arg;   // first arg for functions above
    unsigned               use_extra_arg      : 1;
    unsigned               maybe_empty_object : 1;
    unsigned               alloc_failed       : 1;
};

#endif /* __BITS_OBSTACK_H__ */

