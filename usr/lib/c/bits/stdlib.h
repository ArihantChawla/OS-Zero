#ifndef __ZERO_BITS_STDLIB_H__
#define __ZERO_BITS_STDLIB_H__

struct _atexit {
    void           (*func)(void);
    struct _atexit  *next;
};

struct _onexit {
    void           (*func)(int, void *);
    struct _onexit  *next;
};

#endif /* __ZERO_BITS_STDLIB_H__ */

