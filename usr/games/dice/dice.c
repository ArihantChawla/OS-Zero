/* Copyright (C) Tuomo Petteri Venäläinen 2011 */

/* compile with something like:
 * gcc -O -o dice dice.c
 * - vendu
 */

#if !defined(TEST_DICE)
#define TEST_DICE 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zero/randk.h>
#include "dice.h"

void
diceparsecmd(struct diced20 *buf, int argc, char *argv[])
{
    long l;
    
    for (l = 1 ; l < argc ; l++) {
        if (!strcmp(argv[l], "-d4")) {
            buf->nd4 = atol(argv[++l]);
            fprintf(stderr, "%ldxd4\n", buf->nd4);
        } else if  (!strcmp(argv[l], "-d6")) {
            buf->nd6 = atol(argv[++l]);
            fprintf(stderr, "%ldxd6\n", buf->nd6);
        } else if  (!strcmp(argv[l], "-d8")) {
            buf->nd8 = atol(argv[++l]);
            fprintf(stderr, "%ldxd8\n", buf->nd8);
        } else if  (!strcmp(argv[l], "-d10")) {
            buf->nd10 = atol(argv[++l]);
            fprintf(stderr, "%ldxd10\n", buf->nd10);
        } else if  (!strcmp(argv[l], "-d12")) {
            buf->nd12 = atol(argv[++l]);
            fprintf(stderr, "%ldxd12\n", buf->nd12);
        } else if  (!strcmp(argv[l], "-d20")) {
            buf->nd20 = atol(argv[++l]);
            fprintf(stderr, "%ldxd20\n", buf->nd20);
        }
    }

    return;
}
    
/*
 * initialise random number generator with current time
 */
void
diceinit(unsigned int seed)
{
    if (!seed) {
        srand(time(NULL));
    } else {
        srand(seed);
    }

    return;
}

unsigned long
dicerolld20(struct diced20 *rollbuf)
{
    long retval = 0;

    if (rollbuf->nd4) {
        retval += rollbuf->nd4 * diceroll(DICE_1D4);
    }
    if (rollbuf->nd6) {
        retval += rollbuf->nd6 * diceroll(DICE_1D6);
    }
    if (rollbuf->nd8) {
        retval += rollbuf->nd8 * diceroll(DICE_1D8);
    }
    if (rollbuf->nd10) {
        retval += rollbuf->nd10 * diceroll(DICE_1D10);
    }
    if (rollbuf->nd12) {
        retval += rollbuf->nd12 * diceroll(DICE_1D12);
    }
    if (rollbuf->nd20) {
        retval += rollbuf->nd20 * diceroll(DICE_1D20);
    }

    return retval;
}

#if (TEST)
int
main(int argc, char *argv[])
{
    struct diced20 rollbuf = { 0, 0, 0, 0, 0, 0 };
    int            retval = 0;

    diceinit(0);
    srand(time(NULL));
    srandk(time(NULL));
    diceparsecmd(&rollbuf, argc, argv);
    retval = dicerolld20(&rollbuf);
    fprintf(stderr, "DICE: %lu\n", retval);

    fprintf(stderr, "%dd%d : %d\n",
            8, 6, dicerolln(8, 6));
    fprintf(stderr, "%dd%d : %d\n",
            4, 32, dicerolln(4, 32));
    fprintf(stderr, "%dd%d : %d\n",
            8, 64, dicerolln(8, 64));

    fprintf(stderr, "1d4: %d\n", diceroll(DICE_1D4));
    fprintf(stderr, "1d6: %d\n", diceroll(DICE_1D6));
    fprintf(stderr, "1d8: %d\n", diceroll(DICE_1D8));
    fprintf(stderr, "1d10: %d\n", diceroll(DICE_1D10));
    fprintf(stderr, "1d12: %d\n", diceroll(DICE_1D12));
    fprintf(stderr, "1d20: %d\n", diceroll(DICE_1D20));

    exit(retval);
}
#endif

