#ifndef __CPUNK_ALTAR_H__
#define __CPUNK_ALTAR_H__

#define CPUNK_ALTAR_GOOD    1
#define CPUNK_ALTAR_FAIR    2
#define CPUNK_ALTAR_NEUTRAL 3
#define CPUNK_ALTAR_BAD     4
#define CPUNK_ALTAR_EVIL    5
struct cpunkaltar {
    long              align;    // good, fair, neutral, bad, evil
    long              nsacr;    // # of sacrifices made
    long              nitem;    // # of items on altar
    struct cpunkitem *items;    // item table
};

#endif /* __CPUNK_ALTAR_H__ */

