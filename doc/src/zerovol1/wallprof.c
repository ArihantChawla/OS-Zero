#include <zero/prof.h>
#include <stdio.h>

int
main(int argc, char *argv)
{
    PROFCLK(clk);

    profstartclk(clk);
    printf("hello world\n");
    profstopclk(clk);
    printf("%lu microseconds to print\n", profclkdiff(clk));
}
