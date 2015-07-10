#if (RANDMT64TEST)
#include <stdio.h>
#endif
#include <stdint.h>

#define RANDMT64NSTATE     312
#define RANDMT64NSTATEHALF 156
#define RANDMT64MATRIX     UINT64_C(0xB5026F5AA96619E9)
#define RANDMT64HIMASK     UINT64_C(0xFFFFFFFF80000000) /* high 33 bits */
#define RANDMT64LOMASK     UINT64_C(0x000000007FFFFFFF) /* low 31 bits */

/* state vector array */
static uint64_t randmt64state[RANDMT64NSTATE];
/*
 * randmt64curndx == RANDMT64NSTATE + 1 means that
 * randmt64state[RANDMT64NSTATE] is uninitialised
 */
static uint64_t randmt64magic[2] = { UINT64_C(0), RANDMT64MATRIX };
static long     randmt64curndx = RANDMT64NSTATE + 1;

void
srandmt64(uint64_t seed)
{
    uint64_t val;
    long     ndx;
    uint64_t tmp;
    
    randmt64state[0] = seed;
    tmp = seed;
    for (ndx = 1 ; ndx < RANDMT64NSTATE ; ndx++) {
        val = UINT64_C(6364136223846793005) * (tmp ^ (tmp >> 62)) + ndx;
        randmt64state[ndx] = val;
        tmp = val;
    }
    randmt64curndx = ndx;

    return;
}

void
_randbuf64(void)
{
    uint64_t val;
    long     ndx;
    uint64_t tmp1;
    uint64_t tmp2;

    for (ndx = 0 ; ndx < RANDMT64NSTATEHALF ; ndx++) {
        tmp1 = randmt64state[ndx] & RANDMT64HIMASK;
        tmp2 = randmt64state[ndx + 1] & RANDMT64LOMASK;
        val = tmp1 | tmp2;
        tmp1 = randmt64state[ndx + RANDMT64NSTATEHALF];
        tmp2 = randmt64magic[(int)(val & UINT64_C(1))];
        randmt64state[ndx] = tmp1 ^ (val >> 1) ^ tmp2;
    }
    for ( ; ndx < RANDMT64NSTATE - 1 ; ndx++) {
        tmp1 = randmt64state[ndx] & RANDMT64HIMASK;
        tmp2 = randmt64state[ndx + 1] & RANDMT64LOMASK;
        val = tmp1 | tmp2;
        tmp1 = randmt64state[ndx - RANDMT64NSTATEHALF];
        tmp2 = randmt64magic[(int)(val & UINT64_C(1))];
        randmt64state[ndx] = tmp1 ^ (val >> 1) ^ tmp2;
    }
    tmp1 = randmt64state[RANDMT64NSTATE - 1] & RANDMT64HIMASK;
    tmp2 = randmt64state[0] & RANDMT64LOMASK;
    val = tmp1 | tmp2;
    tmp1 = randmt64state[RANDMT64NSTATEHALF - 1];
    tmp2 = randmt64magic[(int)(val & UINT64_C(1))];
    randmt64state[RANDMT64NSTATE - 1] = tmp1 ^ (val >> 1) ^ tmp2;
    randmt64curndx = 0;

    return;
}

uint64_t randmt64(void)
{
    uint64_t val;
    long     cur;

    if (randmt64curndx >= RANDMT64NSTATE) {
        if (randmt64curndx == RANDMT64NSTATE + 1) {
            srandmt64(UINT64_C(5489));
        }
        _randbuf64();
    }
    cur = randmt64curndx;
    val = randmt64state[cur];
    val ^= (val >> 29) & UINT64_C(0x5555555555555555);
    val ^= (val << 17) & UINT64_C(0x71D67FFFEDA60000);
    cur++;
    val ^= (val << 37) & UINT64_C(0xFFF7EEE000000000);
    val ^= (val >> 43);
    randmt64curndx = cur;

    return val;
}

#if (RANDMT64TEST)
int
main(void)
{
    int i;
#if (RANDMT64PROF)
    PROFDECLCLK(clk);
    unsigned long *buf;
#endif

#if (RANDMT64PROF) && 0
    buf = malloc(65536 * sizeof(unsigned long));
    if (!buf) {
        fprintf(stderr, "randmt64: cannot allocate buffer");

        exit(1);
    }
    profstartclk(clk);
    for (i = 0 ; i < 65536 ; i++) {
        buf[i] = randmt64_r();
    }
    profstopclk(clk);
    fprintf(stderr, "%lu microseconds\n", profclkdiff(clk));
#else
//    srandmt64(UINT64_C(0x5555555555555555));
    for (i = 0; i < 65536; i++) {
        printf("%llx\n", (unsigned long long)randmt64());
    }
#endif

    return 0;
}
#endif

