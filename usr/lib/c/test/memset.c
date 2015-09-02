#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/prof.h>

int
main(int argc, char *argv[])
{
    void *ptr;
    long  l;
    long *lptr;
    long  tm;
    
    PROFDECLCLK(clk);
    ptr = calloc(4 * PAGESIZE);
    memset(ptr, 0xff, 4 * PAGESIZE);
    profinitclk(clk);
    profstartclk(clk);
    memset((uint8_t *)ptr + PAGESIZE, 0, PAGESIZE);
    profstopclk(clk);
    tm = profclkdiff(clk);
    printf("%lu microseconds: %e\n",
           tm,
           1000000.0 * (1000000.0 / (double)tm) * (1.0 / PAGESIZE));
    l = PAGESIZE >> LONGSIZELOG2;
    lptr = (long *)ptr;
    while (l--) {
        if (*lptr++ != ~0L) {
            printf("MEMSET #1 (%lx)\n", (uint8_t *)lptr - (uint8_t *)ptr - 1);
            abort();
        }
    }
    l = PAGESIZE >> LONGSIZELOG2;
    lptr = (long *)((uint8_t *)ptr + PAGESIZE);
    while (l--) {
        if (*lptr++ != 0L) {
            printf("MEMSET #2 (%lx)\n", (uint8_t *)lptr - (uint8_t *)ptr - 1);
            abort();
        }
    }
    l = PAGESIZE >> LONGSIZELOG2;
    lptr = (long *)((uint8_t *)ptr + 2 * PAGESIZE);
    while (l--) {
        if (*lptr++ != ~0L) {
            printf("MEMSET #3 (%lx)\n", (uint8_t *)lptr - (uint8_t *)ptr - 1);
            abort();
        }
    }
#if 0
    profinitclk(clk);
    profstartclk(clk);
    memset(ptr, 0, PAGESIZE);
    printf("%lu\n", profclkdiff(clk));
    profstopclk(clk);
#endif

    return 0;
}

