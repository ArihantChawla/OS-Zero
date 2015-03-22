/*
 * 32-bit implementation of the Mersenne Twister MT19937 algorithm
 */

#if !defined(RANDMT32TEST)
#define RANDMT32TEST 0
#endif

/*
 * REFERENCE: https://en.wikipedia.org/wiki/Mersenne_twister
 */

/*
 * API
 * ---
 * void srandmt32(unsigned long seed);
 * - seed the pseudo-random number generator with the argument
 * unsigned long randmt32(void);
 * - get the next PRN in series
 *
 * void srandmt32_r(unsigned long seed);
 * unsigned long randmt32_r(void);
 * - re-entrant versions [for multithreaded processes]
 */

/* Thanks to Craig Robbins for pointing out errors and other help. :) */

/*
 * optimisations
 * -------------
 * - elimination of modulus calculations and in-loop branches by unrolling loops
 */

#define RANDMTXFINEGRAINED 1

#include <stdint.h>
#include <stdlib.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/mtx.h>
#if (RANDMT32TEST)
#include <stdio.h>
#if (RANDMT32PROF)
#include <zero/prof.h>
#endif
#endif

#define RANDMT32NBUFITEM   624            // # of buffer values
#define RANDMT32MAGIC      397
/* magic numbers */
#define RANDMT32MULTIPLIER 0x6c078965UL
#define RANDMT32MATRIX     0x9908b0dfUL
/* shift counts */
#define RANDMT32SHIFT      30
#define RANDMT32SHIFT1     11
#define RANDMT32SHIFT2     7
#define RANDMT32SHIFT3     15
#define RANDMT32SHIFT4     18
/* bitmasks */
#define RANDMT32MASK2      0x9d2c5680UL
#define RANDMT32MASK3      0xefc60000UL

static unsigned long randbuf32[RANDMT32NBUFITEM] ALIGNED(PAGESIZE);
static volatile long randlkbuf[RANDMT32NBUFITEM];
static unsigned long randndx = RANDMT32NBUFITEM + 1;
static volatile long randmtx;

void
srandmt32(unsigned long seed)
{
    unsigned long val;
    unsigned long tmp;
    long          l;

    tmp = seed & 0xffffffffUL;
    randbuf32[0] = tmp;
    val = RANDMT32MULTIPLIER * (tmp ^ (tmp >> RANDMT32SHIFT)) + 1;
    val &= 0xffffffffUL;
    randbuf32[1] = val;
    for (l = 2 ; l < RANDMT32NBUFITEM ; l++) {
        tmp = val;
        val = RANDMT32MULTIPLIER * (tmp ^ (tmp >> RANDMT32SHIFT)) +  l;
        val &= 0xffffffffUL;
        randbuf32[l] = val;
    }
    randndx = l;

    return;
}

void
srandmt32_r(unsigned long seed)
{
    unsigned long val;
    unsigned long tmp;
    long          l;

    mtxlk(&randmtx);
    tmp = seed & 0xffffffffUL;
    randbuf32[0] = tmp;
    val = RANDMT32MULTIPLIER * (tmp ^ (tmp >> RANDMT32SHIFT)) + 1;
    val &= 0xffffffffUL;
    randbuf32[1] = val;
    for (l = 2 ; l < RANDMT32NBUFITEM ; l++) {
        tmp = val;
        val = RANDMT32MULTIPLIER * (tmp ^ (tmp >> RANDMT32SHIFT)) +  l;
        val &= 0xffffffffUL;
        randbuf32[l] = val;
    }
    randndx = l;
    mtxunlk(&randmtx);

    return;
}

void
_randbuf32(void)
{
    unsigned long mask[2] = { 0UL, RANDMT32MATRIX };
    unsigned long x;
    unsigned long val1;
    unsigned long tmp1;
    unsigned long tmp2;
    unsigned long tmp3;
    long          l;

    if (randndx == RANDMT32NBUFITEM + 1) {
        srandmt32(5489UL);
    }
    for (l = 0 ; l < RANDMT32NBUFITEM - RANDMT32MAGIC ; l++) {
        val1 = l + 1;
        tmp1 = randbuf32[l & 0x80000000UL];
        tmp2 = randbuf32[val1] & 0x7fffffffUL;
        tmp3 = randbuf32[l + RANDMT32MAGIC];
        x = tmp1 | tmp2;
        randbuf32[l] = tmp3 ^ (x >> 1) ^ mask[x & 0x01];
    }
    for ( ; l < RANDMT32NBUFITEM - 1 ; l++) {
        val1 = l + 1;
        tmp1 = randbuf32[l] & 0x80000000UL;
        tmp2 = randbuf32[val1] & 0x7fffffffUL;
        tmp3 = randbuf32[l + RANDMT32MAGIC - RANDMT32NBUFITEM];
        x = tmp1 | tmp2;
        randbuf32[l] = tmp3 ^ (x >> 1) ^ mask[x & 0x01];
    }
    tmp1 = randbuf32[RANDMT32NBUFITEM - 1] & 0x80000000UL;
    tmp2 = (randbuf32[0] & 0x7fffffffUL);
    tmp3 = randbuf32[RANDMT32MAGIC - 1];
    x = tmp1 | tmp2;
    randbuf32[RANDMT32NBUFITEM - 1] = tmp3 ^ (x >> 1) ^ RANDMT32MATRIX;
    randndx = 0;

    return;
}

unsigned long
randmt32(void)
{
    unsigned long x;

    if (randndx >= RANDMT32NBUFITEM) {
        _randbuf32();
    }
    x = randbuf32[randndx];
    x ^= x >> RANDMT32SHIFT1;
    x ^= (x << RANDMT32SHIFT2) & RANDMT32MASK2;
    x ^= (x << RANDMT32SHIFT3) & RANDMT32MASK3;
    x ^= x >> RANDMT32SHIFT4;
    randndx++;

    return x;
}

unsigned long
randmt32_r(void)
{
    unsigned long x;
    unsigned long ndx;

    mtxlk(&randmtx);
    ndx = randndx;
    if (ndx >= RANDMT32NBUFITEM) {
        _randbuf32();
        ndx = randndx;
    }
    randndx++;
    x = randbuf32[ndx];
    mtxunlk(&randmtx);
    x ^= x >> RANDMT32SHIFT1;
    x ^= (x << RANDMT32SHIFT2) & RANDMT32MASK2;
    x ^= (x << RANDMT32SHIFT3) & RANDMT32MASK3;
    x ^= x >> RANDMT32SHIFT4;

    return x;
}

#if (RANDMT32TEST)
int
main(void)
{
    int i;
#if (RANDMT32PROF)
    PROFDECLCLK(clk);
    unsigned long *buf;
#endif

#if (RANDMT32PROF)
    buf = malloc(65536 * sizeof(unsigned long));
    if (!buf) {
        fprintf(stderr, "randmt32: cannot allocate buffer");

        exit(1);
    }
    profstartclk(clk);
    for (i = 0 ; i < 65536 ; i++) {
        buf[i] = randmt32_r();
    }
    profstopclk(clk);
    fprintf(stderr, "%lu microseconds\n", profclkdiff(clk));
#else
    printf("65536 outputs of randmt32()\n");
    for (i = 0; i < 65536; i++) {
        printf("%8lx ", randmt32());
        if ((i & 3) == 0x03) printf("\n");
    }
#endif

    return 0;
}
#endif

