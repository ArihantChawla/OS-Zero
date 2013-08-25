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

void srandmt32(uint32_t seed);

#define RAND32NBUFITEM   624            // # of buffer values
/* magic numbers */
#define RAND32MULTIPLIER 0x6c078965
#define RAND32XORVALUE   0x9908b0df
/* shift counts */
#define RAND32SHIFT      30
#define RAND32SHIFT1     11
#define RAND32SHIFT2     7
#define RAND32SHIFT3     15
#define RAND32SHIFT4     18
/* bitmasks */
#define RAND32MASK2      0x9d2c5680
#define RAND32MASK3      0xefc60000

static uint32_t     randbuf32[RAND32NBUFITEM] ALIGNED(PAGESIZE);
static uint32_t     randnext32[RAND32NBUFITEM];
static uint32_t     randndx;
static volatile int randinit;

void
srandmt32(uint32_t seed)
{
    uint32_t val;
    int32_t  tmp;
    int      i;

    if (!seed) {
        seed++;
    }
    randbuf32[0] = seed;
    val = seed >> RAND32SHIFT;
    tmp = RAND32MULTIPLIER * (seed ^ val) + 1;
    randbuf32[1] = tmp;
    for (i = 2 ; i < RAND32NBUFITEM; i++) {
        val = tmp >> RAND32SHIFT;
        tmp = RAND32MULTIPLIER * (tmp ^ val) + i;
        randbuf32[i] = tmp;
    }
}

void
_randbuf32(void)
{
    int       i;
    uint32_t  x;
    uint32_t  val1;
    uint32_t  val2;
    
    for (i = 0 ; i < 623 - 397 ; i++) {
        val1 = i + 1;
        x = (randbuf32[i] & 0x80000000) + (randbuf32[val1] & 0x7fffffff);
        randbuf32[i] = randbuf32[i + 397] ^ (x >> 1);
        i++;
        val2 = i + 1;
        x = (randbuf32[i] & 0x80000000) + (randbuf32[val2] & 0x7fffffff);
        randbuf32[i] = (randbuf32[i + 397] ^ (x >> 1)) ^ RAND32XORVALUE;
    }
    for ( ; i < RAND32NBUFITEM ; i++) { 
        val1 = i + 1;
        x = (randbuf32[i] & 0x80000000) + (randbuf32[val1] & 0x7fffffff);
        randbuf32[i] = randbuf32[val1] ^ (x >> 1);
        i++;
        val2 = i + 1;
        x = (randbuf32[i] & 0x80000000) + (randbuf32[val2] & 0x7fffffff);
        randbuf32[i] = (randbuf32[val2] ^ (x >> 1)) ^ RAND32XORVALUE;
    }
}

int
randmt32(void)
{
    int x;

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
    x ^= x >> RAND32SHIFT1;
    x ^= (x >> RAND32SHIFT2) & RAND32MASK2;
    x ^= (x >> RAND32SHIFT3) & RAND32MASK3;
    x ^= x >> RAND32SHIFT4;
    randndx = randnext32[randndx];

    return x;
}

