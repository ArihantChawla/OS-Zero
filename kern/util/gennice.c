#include <stdlib.h>
#include <stdio.h>

#define MIN (-64)
#define MAX 63
#define RANGE 40

long nicetab[64];

int
main(int argc, char *argv[])
{
    long   *ptr = &nicetab[64];
    double  dval = (double)MIN;
    double  ofs = -dval / 20.0;
    long    ndx;

    for (ndx = -32 ; ndx < -20 ; ndx++) {
        ptr[ndx] = 0L;
    }
    /* ndx == -20 */
    for ( ; ndx < 0 ; ndx++) {
        ptr[ndx] = (long)dval;
        dval += ofs;
    }
    /* ndx == 0 */
    ptr[ndx] = 0L;
    ndx++;
    ofs = (double)MAX / 19.0;
    dval = ofs;
    for ( ; ndx < 20 ; ndx++) {
        ptr[ndx] = (long)dval;
        dval += ofs;
    }
    printf("/* lookup table to convert nice values to priority offsets */\n");
    printf("/* nice is between -20 and 19 inclusively */\n");
    printf("/* taskniceptr = &tasknicetab[SCHEDNICEHALF]; */\n");
    printf("/* prio += taskniceptr[nice]; */\n");
    printf("long tasknicetab[SCHEDNICERANGE]\n= {\n   ");
    ndx = MIN;
    while (ndx < MAX) {
        printf(" %ld,", ptr[ndx]);
        ndx++;
        if (!(ndx & 0x07)) {
            printf("\n   ");
        }
    }
    printf(" %ld\n};\n", ptr[ndx]);

    return 0;
}

