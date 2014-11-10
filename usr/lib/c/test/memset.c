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
    
    PROFDECLCLK(clk);
    ptr = malloc(PAGESIZE);
    profinitclk(clk);
    profstartclk(clk);
    memset(ptr, 0, PAGESIZE);
    profstopclk(clk);
    printf("%lu\n", profclkdiff(clk));
    profinitclk(clk);
    profstartclk(clk);
    memset(ptr, 0, PAGESIZE);
    printf("%lu\n", profclkdiff(clk));
    profstopclk(clk);

    return 0;
}

