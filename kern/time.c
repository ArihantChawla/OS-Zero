#include <stdint.h>
#include <time.h>
#include <kern/time.h>

void
bt2tspec(struct btime *bt, struct timespec *ts)
{
    ts->tv_sec = bt->sec;
    ts->tv_nsec = ((btfrac_t)1000000000 * (uint32_t)(bt->frac >> 32)) >> 32;

    return;
}

void
tspec2bt(struct timespec *ts, struct btime *bt)
{
    bt->sec = ts->tv_sec;
    /* 18446744073 == 2^64 / 1000000000 */
    bt->frac = ts->tv_nsec * (btfrac_t)BTFRAC(18446744073);

    return;
}

