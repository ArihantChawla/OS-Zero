#if (RANDMT64TEST)
#include <stdio.h>
#endif
#include <stdint.h>
#include <zero/trix.h>

#define RANDMT64NSTATE      312
#define RANDMT64NSTATEHALF  156
#define RANDMT64MATRIX      UINT64_C(0xB5026F5AA96619E9)
#define RANDMT64HIMASK      UINT64_C(0xFFFFFFFF80000000) /* high 33 bits */
#define RANDMT64LOMASK      UINT64_C(0x000000007FFFFFFF) /* low 31 bits */
#define RANDMT64DEFSEED     UINT64_C(5489)
#define RANDMT64TABSEED     UINT64_C(19650218)
#define RANDMT64MULTIPLIER1 UINT64_C(6364136223846793005)
#define RANDMT64MULTIPLIER2 UINT64_C(3935559000370003845)
#define RANDMT64MULTIPLIER3 UINT64_C(2862933555777941757)
#define RANDMT64SHIFT       62
#define RANDMT64SHIFT1      29
#define RANDMT64SHIFT2      17
#define RANDMT64SHIFT3      37
#define RANDMT64SHIFT4      43
#define RANDMT64MASK1       UINT64_C(0x5555555555555555)
#define RANDMT64MASK2       UINT64_C(0x71D67FFFEDA60000)
#define RANDMT64MASK3       UINT64_C(0xFFF7EEE000000000)

/* state vector array */
static uint64_t randmt64state[RANDMT64NSTATE];
/*
 * randmt64curndx == RANDMT64NSTATE + 1 means that
 * randmt64state[RANDMT64NSTATE] is uninitialised
 */
static uint64_t randmt64magic[2] = { UINT64_C(0), RANDMT64MATRIX };
static long     randmt64curndx = RANDMT64NSTATE + 1;
#if (RANDMT64TEST)
static uint64_t randmt64key[4]
= {
    UINT64_C(0x12345),
    UINT64_C(0x23456),
    UINT64_C(0x34567),
    UINT64_C(0x45678)
};
#endif

void
srandmt64(uint64_t seed)
{
    uint64_t val;
    long     ndx;
    uint64_t tmp;
    
    randmt64state[0] = seed;
    tmp = seed;
    for (ndx = 1 ; ndx < RANDMT64NSTATE ; ndx++) {
        val = RANDMT64MULTIPLIER1 * (tmp ^ (tmp >> RANDMT64SHIFT)) + ndx;
        randmt64state[ndx] = val;
        tmp = val;
    }
    randmt64curndx = ndx;

    return;
}

void
srandmt64tab(uint64_t *key, uint64_t keylen)
{
    uint64_t l = 1;
    uint64_t m = 0;
    uint64_t ndx;
    uint64_t val;
    uint64_t tmp;

    srandmt64(RANDMT64TABSEED);
    ndx = max(keylen, RANDMT64NSTATE);
    tmp = randmt64state[0];
    for ( ; (ndx) ; ndx--) {
        val = randmt64state[l];
        val ^= RANDMT64MULTIPLIER2 * (tmp ^ (tmp >> RANDMT64SHIFT));
        val += key[m] + m;
        randmt64state[l] = val;
        m++;
        l++;
        if (m >= keylen) {
            m = 0;
        }
        if (l >= RANDMT64NSTATE) {
            val = randmt64state[RANDMT64NSTATE - 1];
            randmt64state[0] = val;
            l = 1;
        }
        tmp = val;
    }
    tmp = randmt64state[l - 1];
    for (ndx = RANDMT64NSTATE - 1 ; (ndx) ; ndx--) {
        val = randmt64state[l];
        val ^= RANDMT64MULTIPLIER3 * (tmp ^ (tmp >> RANDMT64SHIFT));
        val -= l;
        randmt64state[l] = val;
        l++;
        if (l >= RANDMT64NSTATE) {
            val = randmt64state[RANDMT64NSTATE - 1];
            randmt64state[0] = val;
            l = 1;
        }
        tmp = val;
    }
    randmt64state[0] = UINT64_C(1) << 63;

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

uint64_t
randmt64(void)
{
    uint64_t val;
    long     cur;

    if (randmt64curndx >= RANDMT64NSTATE) {
        if (randmt64curndx == RANDMT64NSTATE + 1) {
            srandmt64(RANDMT64DEFSEED);
        }
        _randbuf64();
    }
    cur = randmt64curndx;
    val = randmt64state[cur];
    val ^= (val >> RANDMT64SHIFT1) & RANDMT64MASK1;
    val ^= (val << RANDMT64SHIFT2) & RANDMT64MASK2;
    cur++;
    val ^= (val << RANDMT64SHIFT3) & RANDMT64MASK3;
    val ^= (val >> RANDMT64SHIFT4);
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
    srandmt64tab(randmt64key, sizeof(randmt64key) / sizeof(uint64_t));
    for (i = 0; i < 65536; i++) {
        printf("%llx\n", (unsigned long long)randmt64());
    }
#endif

    return 0;
}
#endif

