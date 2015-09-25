#ifndef __STYLE_STYLE_H__
#define __STYLE_STYLE_H__

#include <zero/mtx.h>

struct stkitem {
    uintptr_t       val;
    struct stkitem *next;
};

#define stkpush(stk, item)                                              \
    do {                                                                \
        mtxlk(&(stk)->lk);                                              \
        (item)->next = (stk)->top;                                      \
        (stk)->top = (item);                                            \
        mtxunlk(&(stk)->lk);                                            \
    } while (0)

struct stk {
    volatile long   lk;
    struct stkitem *top;
};

#endif /* __STYLE_STYLE_H__ */

