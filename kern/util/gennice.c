#include <stdlib.h>
#include <stdio.h>

#define NICE_MIN   (-64)
#define NICE_MAX   63
#define NICE_RANGE 40
#define SLICE_STEP 1

long nicetab[64];
long slicetab[64];

void
gennice(void)
{
    long   *ptr = &nicetab[32];
    double  dval = (double)NICE_MIN;
    double  ofs = -dval / 20.0;
    long    ndx;

    for (ndx = -32 ; ndx < -20 ; ndx++) {
        ptr[ndx] = 0;
    }
    /* ndx == -20 */
    for ( ; ndx < 0 ; ndx++) {
        ptr[ndx] = (long)dval;
        dval += ofs;
    }
    /* ndx == 0 */
    ptr[ndx] = 0L;
    ndx++;
    ofs = (double)NICE_MAX / 19.0;
    dval = ofs;
    for ( ; ndx < 20 ; ndx++) {
        ptr[ndx] = (long)dval;
        dval += ofs;
    }

    return;
}

void
genslice(void)
{
    long *ptr = &slicetab[32];
    long  val = SLICE_STEP;
    long  slice;
    long  ndx;
    long  tmp;

    for (ndx = -32 ; ndx < -20 ; ndx++) {
        ptr[ndx] = 0;
    }
    /* ndx = -20 */
    tmp = 0;
    for ( ; ndx < 20 ; ) {
        if (tmp == 2) {
            val += SLICE_STEP;
            tmp = 0;
        }
        ptr[ndx] = val;
        ndx++;
        tmp++;
    }
    /* ndx == 20 */
    for ( ; ndx < 31 ; ) {
        ptr[ndx] = 0;
        ndx++;
    }

    return;
}

void
printnice(void)
{
    long ndx;
    long n;

    fprintf(stderr, "[-20, 19] -> [%ld, %ld]\n", SLICE_STEP, 20 * SLICE_STEP);
    printf("#ifndef __KERN_NICE_H__\n");
    printf("#define __KERN_NICE_H__\n");
    printf("\n");
    printf("struct nice {\n");
    printf("    long nice;\n");
    printf("    long slice;\n");
    printf("};\n\n");
    printf("/*\n");
    printf(" * lookup table to convert nice values to priority offsets\n");
    printf(" * as well as slices in ticks\n");
    printf(" *\n");
    printf(" * nice is between -20 and 19 inclusively\n");
    printf(" * taskniceptr = &tasknicetab[SCHEDNICEHALF]\n");
    printf(" * prio += taskniceptr[nice];\n");
    printf(" * slice = tasksliceptr[nice];\n");
    printf(" */\n");
    printf("static struct nice k_schednicetab[SCHEDNICERANGE]\n= {\n   ");
    n = 0;
    for (ndx = 0 ; ndx < 64 ; ndx++) {
        printf(" { %3ld, %3ld }", nicetab[ndx], slicetab[ndx]);
        if (ndx != 63) {
            printf(",");
        }
        if (n == 3) {
            printf("\n   ");
            n = 0;
        } else {
            n++;
        }
    }
    printf("\n};\n\n");
    printf("#endif /* __KERN_NICE_H__ */\n\n");

    return;
}

int
main(int argc, char *argv[])
{
    gennice();
    genslice();
    printnice();

    exit(0);
}

