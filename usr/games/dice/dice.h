#ifndef __GAMES_DICE_H__
#define __GAMES_DICE_H__

#if !defined(DICE_SRAND_TIME)
#define DICE_SRAND_TIME 0 /* timestamp-seed every roll */
#endif

/* standard D20 dice set */
#define DICE_1D4  4
#define DICE_1D6  6
#define DICE_1D8  8
#define DICE_1D10 10
#define DICE_1D12 12
#define DICE_1D20 20

#define MINLOG2 2
#define NLEVEL  24
#define diceshift(l) (MINLOG2 + (l))

#if (DICE_SRAND_TIME)
#include <time.h>
#define diceroll(max) (srandmt32(time(NULL)), (randmt32() % (max)) + 1)
#else
#define diceroll(max) ((randmt32() % (max)) + 1)
#endif

static __inline__ long
dicerolln(long n, long max)
{
    long retval = 0;

    while (n--) {
        retval += diceroll(max);
    }

    return retval;
}

/* roll buffer for d20 dice set */
struct diced20 {
    unsigned long nd4;  /* count of 4-side dice */
    unsigned long nd6;  /* count of 6-side dice */
    unsigned long nd8;  /* count of 8-side dice */
    unsigned long nd10; /* count of 10-side dice */
    unsigned long nd12; /* count of 12-side dice */
    unsigned long nd20; /* count of 20-side dice */
};

#endif /* __GAMES_DICE_H__ */

