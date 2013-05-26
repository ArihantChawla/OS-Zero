#ifndef __ZVC_CAL_H__
#define __ZVC_CAL_H__

#include <zero/trix.h>

#define CAL_NONE          0
#define CAL_NAME_TOKEN    0x01
#define CAL_MICRO_TOKEN   0x02
#define CAL_NEWLINE_TOKEN 0x03

#define calisname(c) (bitset(calnametab, (c)))
extern uint8_t       calnametab[32];

struct caltoken {
    long             type;
    long             parm;
    char            *str;
    struct caltoken *prev;
    struct caltoken *next;
};

struct caltokenq {
    struct caltoken *head;
    struct caltoken *tail;
};

#endif /* __ZVC_CAL_H__ */

