#include <stdio.h>

#define STEP 1

int
main(int argc, char *argv[])
{
    long ndx;
    long val = STEP;
    long slice;
    long tmp;

    fprintf(stderr, "[-20, 19] -> [%ld, %ld]\n", STEP, 20 * STEP);
    printf("/* lookup table to convert nice values to slices in 4-ms ticks */\n");
    printf("/* nice is between -20 and 19 inclusively */\n");
    printf("/* tasksliceptr = &taskslicetab[SCHEDNICEHALF]; */\n");
    printf("/* slice = tasksliceptr[nice]; */\n");
    printf("long taskslicetab[SCHEDNICERANGE]\n= {\n   ");
    for (ndx = -32 ; ndx < -20 ; ) {
        printf(" 0,");
        ndx++;
        if (!(ndx & 0x07)) {
            printf("\n   ");
        }
    }
    /* ndx = -20 */
    tmp = 0;
    for ( ; ndx < 20 ; ) {
        if (tmp == 2) {
            val += STEP;
            tmp = 0;
        }
        printf(" %ld,", val);
        ndx++;
        if (!(ndx & 0x07)) {
            printf("\n   ");
        }
        tmp++;
    }
    /* ndx == 20 */
    for ( ; ndx < 31 ; ) {
        printf(" 0,");
        ndx++;
        if (!(ndx & 0x07)) {
            printf("\n   ");
        }
    }
    printf("\n};\n");

    return 0;
}

