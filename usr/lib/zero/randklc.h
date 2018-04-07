#ifndef __ZERO_RANDKLC_H__
#define __ZERO_RANDKLC_H__

/*
 * inline macros for Knuth's linear congruential pseudo-random number generator
 */

#define i_srandklfc32(seedu32) (g_seed32 = (seed))
#define i_randklfc32(val)                                               \
    ((val) = g_seed32, (val) *= 69069, (val) += 1234567, g_seed32 = (val))

#define i_srandklfc64(seedu64) (g_seed64 = (seed))
#define i_randklfc64(val)                                               \
    ((val) = g_seed64,                                                  \
     (val) *= 6364136223846793005,                                      \
     (val) += 1442695040888963407,                                      \
     g_seed64 = (val))

#endif /* __ZERO_RANDKLC_H__ */

