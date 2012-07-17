#ifndef __SYS_SELECT_H__
#define __SYS_SELECT_H__

#include <zero/param.h>

/* TODO: place these in proper headers */
struct timespec {
    long tv_sec;
    long tv_nsec;
};

#define NFDSET 32768    // 4 K bitmaps

struct fdset {
    long bits[NFDSET >> (LONGSIZELOG2 + 3)];
};
typedef struct fdset fd_set;

#endif /* __SYS_SELECT_H__ */

