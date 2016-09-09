#ifndef __KERN_TIME_H__
#define __KERN_TIME_H__

/* REFERENCE: http://phk.freebsd.dk/pubs/timecounter.pdf */

#include <stdint.h>
#include <sys/types.h>

#define TIME_SECOND 1000000000    // nanosecond resolution

#define BTFRAC(i)      INT64_C(i)
#define RNTPCORRECT(i) (((i) * 2199) >> 9)

typedef uint64_t btfrac_t;
typedef uint32_t hwfreq_t;

struct btime {
    time_t   sec;
    btfrac_t frac;
};

#if 0
struct hwtimecnt {
    const char   *name;
    hwtimefunc_t *read; // read-function
    hwfreq_t      freq; // frequency
    uint32_t      nbit; // number of bits implemented
};

struct timehands {
    struct hwtimecnt *cnt;
};
#endif

//time_t kcurtime(void);

static __inline__ void
btadd(struct btime *bt1, struct btime *bt2, struct btime *res)
{
    btfrac_t uval = bt1->frac;

    res->frac = bt1->frac + bt2->frac;
    res->sec = bt1->sec + bt2->sec;
    if (uval > res->frac) {
        res->sec++;
    }

    return;
}

#endif /* __KERN_TIME_H__ */

