#include <stdio.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>
#include <zero/prof.h>
#include <zero/trix.h>

#define TEST 1

/*
 * WARNING: some of the techniques here haven't been verified yet... I have
 * been gathering information off the Internet. TODO: reference links.
 */

#if 0
#include "../const.h"
#include "../prof.h"
#endif

#define PROF_ITERATIONS 1024
#define TABLE_SIZE      PROF_ITERATIONS
#define STRING_LENGTH   256
static unsigned long table1[TABLE_SIZE];
static unsigned long table2[TABLE_SIZE];
static unsigned long table3[TABLE_SIZE];
static unsigned long long ulltable1[TABLE_SIZE];
static float         ftable1[TABLE_SIZE];

size_t
_strlen(const char *s)
{
    size_t len;

    len = 0;
    while (*s) {
        len++;
        s++;
    }

    return len;
}

size_t
_strlen2(const char *s)
{
    size_t len;
    const char *cp;
    unsigned long ul;

    cp = s - 1;
    do {
        cp++;
        if (!modpow2((uintptr_t)cp, sizeof(ul))) {
            do {
                ul = *((unsigned long *)cp);
                cp += sizeof(ul) / sizeof(char);
            } while (!haszero32(ul));
        }
    } while (*cp);
    len = cp - s;

    return len;
}

void
initrand(void)
{
    int i;

    srand(1);
    for (i = 0 ; i < TABLE_SIZE ; i++) {
        table1[i] = rand();
        table2[i] = rand();
    }

    return;
}

#define _RANDSTRWORD() \
    ((rand() << 24) \
     | ((rand() & 0xff) << 16) \
     | ((rand() & 0xff) << 8) \
     | (rand() & 0xff))
void
initstr(void)
{
    int i;

    srand(1);
    for (i = 0 ; i < TABLE_SIZE - 1; i++) {
        table1[i] = 0xffffffff;
    }
    table1[i] = 0;

    return;
}

void
profmin(void)
{
    int i1, i2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    initrand();
    sleep(1);
    i2 = INT_MAX;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0 ; i1 < TABLE_SIZE ; i1++) {
        table3[i1] = min(table1[i1], table2[i1]);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "min\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profmin2(void)
{
    int i1, i2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    initrand();
    sleep(1);
    i2 = INT_MAX;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0 ; i1 < TABLE_SIZE ; i1++) {
        table3[i1] = min2(table1[i1], table2[i1]);
#if defined(TEST)
        if (table3[i1] != min(table1[i1], table2[i1])) {
            fprintf(stderr, "MIN2 FAILED\n");
        }
#endif
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "min2\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

#if 0
void
profmin3(void)
{
    int i1, i2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    initrand();
    sleep(1);
    i2 = INT_MAX;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0 ; i1 < TABLE_SIZE ; i1++) {
        table3[i1] = min3(table1[i1], table2[i1]);
#if defined(TEST)
        if (table3[i1] != min(table1[i1], table2[i1])) {
            fprintf(stderr, "MIN3 FAILED\n");
        }
#endif
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "min3\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}
#endif

void
profmax(void)
{
    int i1, i2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    initrand();
    sleep(1);
    i2 = INT_MAX;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0 ; i1 < PROF_ITERATIONS ; i1++) {
        table3[i1] = max(table1[i1], table2[i2]);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "max\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profmax2(void)
{
    int i1, i2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    initrand();
    sleep(1);
    i2 = INT_MAX;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0 ; i1 < PROF_ITERATIONS ; i1++) {
        table3[i1] = max2(table1[i1], table2[i2]);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "max2\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profmax3(void)
{
    int i1, i2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    initrand();
    sleep(1);
    i2 = INT_MAX;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0 ; i1 < PROF_ITERATIONS ; i1++) {
        table3[i1] = max3(table1[i1], table2[i2]);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "max3\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profswap(void)
{
    int i1, i2, i3, i4;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    i2 = INT_MAX;
    i3 = INT_MAX;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0 ; i1 < PROF_ITERATIONS ; i1++) {
        i4 = i2;
        i2 = i3;
        i3 = i2;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "swap\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profswapi(void)
{
    int i1, i2, i3;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    i2 = INT_MAX;
    i3 = 0;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0 ; i1 < PROF_ITERATIONS ; i1++) {
        swapi(i3, i2);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "swapi\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profifabsf(void)
{
    uint32_t u32;
    float f1;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (f1 = 0.0 ; f1 < (float)PROF_ITERATIONS ; f1 += 1.0) {
        u32 = ifabsf(f1);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "ifabsf\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profifabs(void)
{
    uint64_t u64;
    double d1;;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (d1 = 0.0 ; d1 < (double)PROF_ITERATIONS ; d1 += 1.0) {
        u64 = ifabs(d1);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "ifabs\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profalign1(void)
{
    int i1, i2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    i2 = 8;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0 ; i1 < PROF_ITERATIONS ; i1++) {
        table1[i1] = align1(i1, i2);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "align1\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profalign2(void)
{
    int i1, i2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    i2 = 8;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0 ; i1 < PROF_ITERATIONS ; i1++) {
        table1[i1] = align2(i1, i2);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "align2\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

#define roundup1(a, b2) (((a) + ((b2) - 0x01)) & ~((b2) + 0x01))
#define roundup2(a, b2) (((a) + ((b2) - 0x01)) & -(b2))

void
profroundup1(void)
{
    int i1, i2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    i2 = 8;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0 ; i1 < PROF_ITERATIONS ; i1++) {
        table1[i1] = roundup1(i1, i2);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "roundup1\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profroundup2(void)
{
    int i1, i2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    i2 = 8;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0 ; i1 < PROF_ITERATIONS ; i1++) {
        table1[i1] = roundup2(i1, i2);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "roundup2\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profuavg(void)
{
    int i1, i2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    i2 = 8;
    profstartclk(clk);
    profstarttick(tick);
    for (i1 = 0, i2 = PROF_ITERATIONS ;
         i1 < PROF_ITERATIONS ;
         i1++, i2--) {
        table1[i1] = uavg(i1, i2);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "uavg\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profbkt(void)
{
    unsigned long ul1, ul2, ul3;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul1 = 0 ; ul1 < 32 ; ul1++) {
        ul2 = 1UL << (ul1 - 1);
        ul3 = 0;
        while (ul2) {
            ul3++;
            ul2 >>= 1;
        }
        table1[ul1] = ul3;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "bkt\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick),
            (long double)profclkdiff(clk));

    return;
}

void
profbkt2(void)
{
    unsigned long ul1, ul2, ul3, ul4;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul1 = 0 ; ul1 < 32 ; ul1++) {
        ul2 = 1UL << ul1;
        _lzero32(ul2, ul4);
        ul3 = CHAR_BIT * sizeof(unsigned long) - ul4 - 1;
        table1[ul1] = ul3;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "bkt2\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick),
            (long double)profclkdiff(clk));

    return;
}

void
profbktb(void)
{
    unsigned long ul1, ul2, ul3, ul4;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul1 = 0 ; ul1 < 32 ; ul1++) {
        ul2 = 1UL << ul1;
        //        lzero32(ul2, ul4);
        _lzero32(ul2, ul4);
        ul3 = CHAR_BIT * sizeof(unsigned long) - ul4 - 1;
        table1[ul1] = ul3;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "bktb\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick),
            (long double)profclkdiff(clk));

    return;
}

void
profbkt3(void)
{
    unsigned long ul1, ul2, ul3;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul1 = 0 ; ul1 < 32 ; ul1++) {
        ul2 = 1UL << ul1;
        tzero32(ul2, ul3);
        table1[ul1] = ul3;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "bkt3\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick),
            (long double)profclkdiff(clk));

    return;
}

void
profbkt4(void)
{
    unsigned long ul1, ul2, ul3;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul1 = 0 ; ul1 < 32 ; ul1++) {
        ul2 = 1UL << ul1;
        tzero32b(ul2, ul3);
        table1[ul1] = ul3;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "bkt4\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick),
            (long double)profclkdiff(clk));

    return;
}

/* if (a < b) x = c; else x = d; */

void
profcond(void)
{
    unsigned long ul1, ul2, ul3, ul4;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initrand();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    ul3 = 0;
    ul4 = 1;
    for (ul1 = 0 ; ul1 < PROF_ITERATIONS ; ul1++) {
        ul2 = table1[ul1];
        ul3 = table2[ul1];
        if (ul2 < ul3) {
            ul4 = ul2;
        } else {
            ul4 = ul3;
        }
        table1[ul1] = ul4;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "cond\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profcond2(void)
{
    unsigned long ul1, ul2, ul3, ul4;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initrand();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    ul3 = 0;
    ul4 = 1;
    for (ul1 = 0 ; ul1 < PROF_ITERATIONS ; ul1++) {
        ul2 = table1[ul1];
        ul3 = table2[ul1];
        ul4 = (ul2 < ul3) ? ul2 : ul3;
        table1[ul1] = ul4;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "cond2\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profcond3(void)
{
    unsigned long ul1, ul2, ul3, ul4;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initrand();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    ul3 = 0;
    ul4 = 1;
    for (ul1 = 0 ; ul1 < PROF_ITERATIONS ; ul1++) {
        ul2 = table1[ul1];
        ul3 = table2[ul1];
        ul4 = condltset(ul2, ul3, ul2, ul3);
        table1[ul1] = ul4;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "cond3\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
proftzero(void)
{
    unsigned long ul1, ul2, ul3, ul4;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initrand();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    ul3 = 0;
    ul4 = 1;
    for (ul1 = 0 ; ul1 < PROF_ITERATIONS ; ul1++) {
        ul2 = table1[ul1];
        tzero32(ul2, ul4);
        table3[ul1] = ul4;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "tzero\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
proftzerob(void)
{
    unsigned long ul1, ul2, ul3, ul4;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initrand();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    ul3 = 0;
    ul4 = 1;
    for (ul1 = 0 ; ul1 < PROF_ITERATIONS ; ul1++) {
        ul2 = table1[ul1];
        tzero32b(ul2, ul4);
        table3[ul1] = ul4;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "tzerob\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
proflzero(void)
{
    unsigned long ul1, ul2, ul3, ul4;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initrand();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    ul3 = 0;
    ul4 = 1;
    for (ul1 = 0 ; ul1 < PROF_ITERATIONS ; ul1++) {
        ul2 = table1[ul1];
        _lzero32(ul2, ul4);
        table3[ul1] = ul4;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "lzero\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
proflzero32(void)
{
    unsigned long ul1, ul2, ul3, ul4;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initrand();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    ul3 = 0;
    ul4 = 1;
    for (ul1 = 0 ; ul1 < PROF_ITERATIONS ; ul1++) {
        ul2 = table1[ul1];
        _lzero32(ul2, ul4);
        table3[ul1] = ul4;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "lzero32\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
proflzero64(void)
{
    unsigned long ul1, ul2, ul3, ul4;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initrand();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    ul3 = 0;
    ul4 = 1;
    for (ul1 = 0 ; ul1 < PROF_ITERATIONS ; ul1++) {
        ul2 = table1[ul1];
        _lzero64(ul2, ul4);
        table3[ul1] = ul4;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "lzero32\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
proflabs(void)
{
    unsigned long ul;
    long l;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0, l = -32767 ; ul < PROF_ITERATIONS ; ul++, l++) {
        table1[ul] = _labs(l);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "labs\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profonebits(void)
{
    unsigned long ul, tmp;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0 ; ul < PROF_ITERATIONS ; ul++) {
        table1[ul] = onebits_32(ul, tmp);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "one\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profhaszero(void)
{
    unsigned long ul;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0 ; ul < PROF_ITERATIONS ; ul++) {
        table1[ul] = haszero(ul);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "haszero\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profhaszero2(void)
{
    unsigned long ul;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0 ; ul < PROF_ITERATIONS ; ul++) {
        table1[ul] = haszero2(ul);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "haszero2\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profhaszero32(void)
{
    unsigned long ul;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0 ; ul < PROF_ITERATIONS ; ul++) {
        table1[ul] = haszero32(ul);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "haszero\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (long double)PROF_ITERATIONS,
            (long double)profclkdiff(clk) / (long double)PROF_ITERATIONS);

    return;
}

void
profbytepar(void)
{
    uint8_t *u8p1, *u8p2;
    unsigned long ul;
    uint8_t u8;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    u8p1 = (uint8_t *)table1;
    u8p2 = (uint8_t *)table2;
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0 ; ul < sizeof(unsigned long) * PROF_ITERATIONS ; ul++) {
        bytepar(*u8p1, u8);
        *u8p2 = u8;
        u8p1++;
        u8p2++;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "bytepar\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));

    return;
}

void
profbytepar2(void)
{
    uint8_t *u8p1, *u8p2;
    unsigned long ul;
    uint8_t u8;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    u8p1 = (uint8_t *)table1;
    u8p2 = (uint8_t *)table2;
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0 ; ul < sizeof(unsigned long) * PROF_ITERATIONS ; ul++) {
        bytepar2(*u8p1, u8);
        *u8p2 = u8;
        u8p1++;
        u8p2++;
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "bytepar2\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));

    return;
}

void
initftab(void)
{
    unsigned long ul;
    int i;

    srand(1);
    for (ul = 0 ; ul < TABLE_SIZE ; ul++) {
        i = rand();
        ftable1[ul] = *((float *)(&i));
    }
}

/* FIXME: little-endian. */
#define _dtohi32(d)    (*(((uint32_t *)&(d)) + 1))
/*
 * IEEE 32-bit
 * 0..22  - mantissa
 * 23..30 - exponent
 * 31     - sign
 */
/*
 * IEEE 64-bit
 * 0..51  - mantissa
 * 52..62 - exponent
 * 63     - sign
 */
/* convert elements of float to integer. */
#define _fgetm(f)      (_ftou32(f) & 0x007fffff)
#define _fgete(f)      ((_ftou32(f) >> 23) & 0xff)
#define _fgets(f)      (_ftou32(f) >> 31)
/* convert elements of double to integer. */
#define _dgetm(d)      (_dtou64(d) & UINT64_C(0x000fffffffffffff))
#define _dgete(d)      ((_dtohi32(d) >> 20) & 0x7ff)
#define _dgets(d)      (_dtohi32(d) >> 31)

void
proffgetm(void)
{
    unsigned long ul;
    float f1;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initftab();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0 ; ul < PROF_ITERATIONS ; ul++) {
        f1 = ftable1[ul];
        table1[ul] = _fgetm(f1);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "fgetm\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));
}

void
proffgete(void)
{
    unsigned long ul;
    float f1;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initftab();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0 ; ul < PROF_ITERATIONS ; ul++) {
        f1 = ftable1[ul];
        table1[ul] = _fgete(f1);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "fgete\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));
}

void
proffgets(void)
{
    unsigned long ul;
    float f1;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initftab();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0 ; ul < PROF_ITERATIONS ; ul++) {
        f1 = ftable1[ul];
        table1[ul] = _fgets(f1);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "fgets\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));
}

void
initdtab(void)
{
    unsigned long ul;
    unsigned long long ull1, ull2;

    srand(1);
    for (ul = 0 ; ul < TABLE_SIZE ; ul++) {
        ull1 = rand();
        ull2 = rand();
        ulltable1[ul] = ull1 << 32 | ull2;
    }
}

void
profdgetm(void)
{
    double d1;
    unsigned long ul;
    unsigned long long ull;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initdtab();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0 ; ul < PROF_ITERATIONS ; ul++) {
        ull = ulltable1[ul];
        *(unsigned long long *)&d1 = ull;
        ulltable1[ul] = _dgetm(d1);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "dgetm\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));
}

void
profdgete(void)
{
    double d1;
    unsigned long ul;
    unsigned long long ull;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initdtab();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0 ; ul < PROF_ITERATIONS ; ul++) {
        ull = ulltable1[ul];
        *(unsigned long long *)&d1 = ull;
        ulltable1[ul] = _dgete(d1);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "dgete\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));
}

void
profdgets(void)
{
    double d1;
    unsigned long ul;
    unsigned long long ull;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initdtab();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul = 0 ; ul < PROF_ITERATIONS ; ul++) {
        ull = ulltable1[ul];
        *(unsigned long long *)&d1 = ull;
        ulltable1[ul] = _dgets(d1);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "dgets\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));
}

void
profsatu8(void)
{
    unsigned long ul1, ul2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initrand();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul1 = 0 ; ul1 < PROF_ITERATIONS ; ul1++) {
        ul2 = table1[ul1];
        table1[ul1] = satu8(ul2);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "satu8\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));
}

void
profsatu16(void)
{
    unsigned long ul1, ul2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initrand();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul1 = 0 ; ul1 < PROF_ITERATIONS ; ul1++) {
        ul2 = table1[ul1];
        table1[ul1] = satu16(ul2);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "satu16\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));
}

void
profsatu32(void)
{
    unsigned long ul1, ul2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initrand();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul1 = 0 ; ul1 < PROF_ITERATIONS ; ul1++) {
        ul2 = table1[ul1];
        table1[ul1] = satu32(ul2);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "satu32\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));
}

void
profsat8b(void)
{
    unsigned long ul1, ul2;
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initrand();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    for (ul1 = 0 ; ul1 < PROF_ITERATIONS ; ul1++) {
        ul2 = table1[ul1];
        table1[ul1] = sat8b(ul2);
    }
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "sat8b\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));
}

void
profstrlen(void)
{
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initstr();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    _strlen((const char *)table1);
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "strlen\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));
}

void
profstrlen2(void)
{
    PROFDECLCLK(clk);
    PROFDECLTICK(tick);

    initstr();
    sleep(2);
    profstartclk(clk);
    profstarttick(tick);
    _strlen2((const char *)table1);
    profstoptick(tick);
    profstopclk(clk);
    fprintf(stderr, "strlen\t\t%Lf\t%Lf\n",
            (long double)proftickdiff(tick) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS),
            (long double)profclkdiff(clk) / (sizeof(unsigned long) * (long double)PROF_ITERATIONS));
}

int
main(int argc, char *argv[])
{
    unsigned long ul1, ul2;

    ul1 = 0xffff0000;
    ul2 = 0x000000ff;
    fprintf(stderr, "FIXME: mergebits: %lx - %lx - %0.8lx\n", ul1, ul2, mergebits(ul1, ul2, 0xff0000ff));

    profbkt();
    profbkt2();
    profbkt3();
    profbkt4();
    profcond();
    profcond2();
    profcond3();
    fprintf(stderr, "opt\t\ttick/op\tusec/op\n");
    profmin();
    profmin2();
    //    profmin3();
    profmax();
    profmax2();
    profmax3();
    profswap();
    profswapi();
    profifabsf();
    profifabs();
    profalign1();
    profalign2();
    profroundup1();
    profroundup2();
    profuavg();
    proftzero();
    proftzerob();
    proflzero();
    proflzero32();
    proflzero64();
    proflabs();
    profonebits();
    profhaszero();
    profhaszero2();
    profhaszero32();
    profbytepar();
    profbytepar2();
    proffgetm();
    proffgete();
    proffgets();
    profdgetm();
    profdgete();
    profdgets();
    profsat8b();
    profsatu8();
    profsatu16();
    profsatu32();
    profstrlen();
    profstrlen2();

    return 0;
}

