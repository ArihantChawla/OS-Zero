#ifndef __GAMES_DICE_H__
#define __GAMES_DICE_H__

#define DICE_SRAND_TIME 0 /* timestamp-seed every roll */

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
#define rolld(val) (srand(time(NULL)), rand() % (val))
#define rolldn(n, val)                                                  \
    (srandk(time(NULL)), randk() % (n * (val)))
#else
#define rolld(val) (randk() % (val))
#define rolldn(n, val)                                                  \
    (randk() % (n * (val)))
#endif

struct diced20 {
    unsigned long nd4;
    unsigned long nd6;
    unsigned long nd8;
    unsigned long nd10;
    unsigned long nd12;
    unsigned long nd20;
};

#endif /* __GAMES_DICE_H__ */

