/*
 * 32-bit implementation of the Mersenne Twister MT19937 algorithm
 */

#define FASTLOOP 1

/*
 * REFERENCE: https://en.wikipedia.org/wiki/Mersenne_twister
 */

/*
 * optimisations
 * -------------
 * - elimination of modulus calculations and in-loop branches by unrolling loops
 */

#include <stdint.h>
#include <stdlib.h>
#include <zero/param.h>
#include <zero/cdecl.h>

#define RANDMT32NBUFITEM   624            // # of buffer values
/* magic numbers */
#define RANDMT32MULTIPLIER 0x6c078965
#define RANDMT32XORVALUE   0x9908b0df
/* shift counts */
#define RANDMT32SHIFT      30
#define RANDMT32SHIFT1     11
#define RANDMT32SHIFT2     7
#define RANDMT32SHIFT3     15
#define RANDMT32SHIFT4     18
/* bitmasks */
#define RANDMT32MASK2      0x9d2c5680
#define RANDMT32MASK3      0xefc60000

static int32_t      randbuf32[RANDMT32NBUFITEM] ALIGNED(PAGESIZE);
static int32_t      randnext32[RANDMT32NBUFITEM] ALIGNED(PAGESIZE);
static int32_t      randndx;
static volatile int randinit;

void
srandmt32(int32_t seed)
{
    int32_t val;
    int32_t tmp;
    int     i;

    if (!seed) {
        seed++;
    }
    randbuf32[0] = seed;
    val = seed >> RANDMT32SHIFT;
    tmp = RANDMT32MULTIPLIER * (seed ^ val) + 1;
    randbuf32[1] = tmp;
    for (i = 2 ; i < RANDMT32NBUFITEM; i++) {
        val = tmp >> RANDMT32SHIFT;
        tmp = RANDMT32MULTIPLIER * (tmp ^ val) + i;
        randbuf32[i] = tmp;
    }
}

void
_randbuf32(void)
{
    int      i;
    int32_t  x;
    int32_t  val1;
    int32_t  val2;
    
    for (i = 0 ; i < 623 - 397 ; i++) {
        val1 = i + 1;
        x = (randbuf32[i] & 0x80000000) + (randbuf32[val1] & 0x7fffffff);
        randbuf32[i] = randbuf32[i + 397] ^ (x >> 1);
        i++;
        val2 = i + 1;
        x = (randbuf32[i] & 0x80000000) + (randbuf32[val2] & 0x7fffffff);
        randbuf32[i] = (randbuf32[i + 397] ^ (x >> 1)) ^ RANDMT32XORVALUE;
    }
    for ( ; i < RANDMT32NBUFITEM ; i++) { 
        val1 = i + 1;
        x = (randbuf32[i] & 0x80000000) + (randbuf32[val1] & 0x7fffffff);
        randbuf32[i] = randbuf32[val1] ^ (x >> 1);
        i++;
        val2 = i + 1;
        x = (randbuf32[i] & 0x80000000) + (randbuf32[val2] & 0x7fffffff);
        randbuf32[i] = (randbuf32[val2] ^ (x >> 1)) ^ RANDMT32XORVALUE;
    }
}

int
randmt32(void)
{
    int32_t x;

    if (!randndx) {
        _randbuf32();
    }
    if (!randinit) {
        for (x = 0 ; x < 623 ; x++) {
            randnext32[x] = x + 1;
        }
        randnext32[623] = 0;
        randinit = 1;
    }
    x = randbuf32[randndx];
    x ^= x >> RANDMT32SHIFT1;
    x ^= (x >> RANDMT32SHIFT2) & RANDMT32MASK2;
    x ^= (x >> RANDMT32SHIFT3) & RANDMT32MASK3;
    x ^= x >> RANDMT32SHIFT4;
    randndx = randnext32[randndx];

    return (int)(x & 0x7fffffff);
}

