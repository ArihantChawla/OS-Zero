/*
 * REFERENCE: https://en.wikipedia.org/wiki/Mersenne_twister
 */

#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>

void srand32(uint32_t seed);

#define RAND32NBUFITEM   624
#define RAND32MULTIPLIER 0x6c078965
#define RAND32XORVALUE   0x9908b0df
#define RAND32SHIFT      30
#define RAND32SHIFT1     11
#define RAND32SHIFT2     7
#define RAND32SHIFT3     15
#define RAND32SHIFT4     18
#define RAND32MASK2      0x9d2c5680
#define RAND32MASK3      0xefc60000

static uint32_t     randbuf32[RAND32NBUFITEM] ALIGNED(PAGESIZE);
static uint32_t     randnext32[RAND32NBUFITEM];
static uint32_t     randndx;
static volatile int randinit;

void
srand32(uint32_t seed)
{
    int32_t tmp;
    int     i;

    if (!seed) {
        seed++;
    }
    randbuf32[0] = seed;
    tmp = RAND32MULTIPLIER * (seed ^ (seed >> RAND32SHIFT)) + 1;
    randbuf32[1] = tmp;
    for (i = 2 ; i < RAND32NBUFITEM; i++) {
        tmp = RAND32MULTIPLIER * (tmp ^ (tmp >> RAND32SHIFT)) + i;
        randbuf32[i] = tmp;
    }
}

void
_randbuf32(void)
{
    int      i;
    uint32_t x;
    
    for (i = 0 ; i < 623 - 397 ; i++) {
        x = (randbuf32[i] & 0x80000000) + (randbuf32[(i + 1) % RAND32NBUFITEM]);
        randbuf32[i] = (randbuf32[i + 397]) ^ (x >> 1);
        i++;
        x = (randbuf32[i] & 0x80000000) + (randbuf32[(i + 1) % RAND32NBUFITEM]);
        randbuf32[i] = ((randbuf32[i + 397]) ^ (x >> 1)) ^ RAND32XORVALUE;
    }
    for ( ; i < RAND32NBUFITEM ; i++) {
        x = (randbuf32[i] & 0x80000000) + (randbuf32[(i + 1) % RAND32NBUFITEM]);
        randbuf32[i] = (randbuf32[(623 - 397 + i)]) ^ (x >> 1);
        i++;
        x = (randbuf32[i] & 0x80000000) + (randbuf32[(i + 1) % RAND32NBUFITEM]);
        randbuf32[i] = ((randbuf32[(623 - 397 + i)]) ^ (x >> 1)) ^ RAND32XORVALUE;
    }
}

int
rand(void)
{
    uint32_t x;

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
    x = (x >> RAND32SHIFT2) & RAND32MASK2;
    x = (x >> RAND32SHIFT3) & RAND32MASK3;
    x ^= x >> RAND32SHIFT4;
    randndx = randnext32[randndx];

    return (int)x;
}

