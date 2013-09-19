#include <stdlib.h>
#include <stdint.h>
#include <zero/randlfg2.h>
#include <zero/randmt32.h>

#define RAND_J 9739
#define RAND_K 23209
#define RAND_B 1586
#define RAND_N 16384    // power of two
#define RAND_M (RAND_N - 1)

static volatile int  randinit;
static          int *randbuf;
static          long randndx;

int
randlfg2(void)
{
    int retval;
    int val1;
    int val2;
    int i;

    if (!randinit) {
        randbuf = malloc(RAND_N * sizeof(int));
        if (randbuf) {
            srandmt32(1);
            for (i = 0 ; i < RAND_N ; i++) {
                randbuf[i] = randmt32();
            }
            randinit = 1;
        }
    }
    randndx++;
    val1 = randbuf[(randndx - RAND_J) & RAND_M];
    val2 = randbuf[(randndx - RAND_K) & RAND_M];
    retval = val1 ^ val2;
    randbuf[randndx & RAND_M] = retval;

    return retval;
}

