#ifndef __CPUNK_LEVEL_H__
#define __CPUNK_LEVEL_H__

struct cpunklevel * cpunknewlevel(void);

#define CPUNK_LEVEL_WIDTH  80
#define CPUNK_LEVEL_HEIGHT 24
struct cpunklevel {
    char items[CPUNK_LEVEL_HEIGHT][CPUNK_LEVEL_WIDTH];
};

#endif /* __CPUNK_LEVEL_H__ */

