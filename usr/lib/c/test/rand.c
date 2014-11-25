#include <stdio.h>
#include <stdlib.h>
#include <zero/param.h>
#include <zero/prof.h>
#include <zero/randk.h>
#include <zero/randlfg.h>
#include <zero/randlfg2.h>
#include <zero/randmt32.h>

int
main(int argc, char *argv[])
{
    long  l;
    void *ptr = malloc(1024 * PAGESIZE);
    int  *ip = ptr;
    PROFDECLCLK(clk);

    profstartclk(clk);
    for (l = 0 ; l < 256 * PAGESIZE / sizeof(int) ; l++) {
        ip[l] = randk();
    }
    profstopclk(clk);
    printf("KNUTH: %ld microseconds\n", profclkdiff(clk));
    for (l = 0 ; l < 256 * PAGESIZE / sizeof(int) ; l++) {
        fprintf(stderr, "%d\n", ip[l]);
    }
    profstartclk(clk);
    ip += PAGESIZE / sizeof(int);
    for (l = 0 ; l < 256 * PAGESIZE / sizeof(int) ; l++) {
        ip[l] = randlfg();
    }
    profstopclk(clk);
    printf("LFG: %ld microseconds\n", profclkdiff(clk));
    for (l = 0 ; l < 256 * PAGESIZE / sizeof(int) ; l++) {
        fprintf(stderr, "%d\n", ip[l]);
    }
    profstartclk(clk);
    ip += PAGESIZE / sizeof(int);
    for (l = 0 ; l < 256 * PAGESIZE / sizeof(int) ; l++) {
        ip[l] = randlfg2();
    }
    profstopclk(clk);
    printf("LFG2: %ld microseconds\n", profclkdiff(clk));
    for (l = 0 ; l < 256 * PAGESIZE / sizeof(int) ; l++) {
        fprintf(stderr, "%d\n", ip[l]);
    }
    profstartclk(clk);
    for (l = 0 ; l < 256 * PAGESIZE / sizeof(int) ; l++) {
        ip[l] = randmt32();
    }
    profstopclk(clk);
    printf("MT32: %ld microseconds\n", profclkdiff(clk));
    for (l = 0 ; l < 256 * PAGESIZE / sizeof(int) ; l++) {
        fprintf(stderr, "%d\n", ip[l]);
    }

    exit(0);
}

