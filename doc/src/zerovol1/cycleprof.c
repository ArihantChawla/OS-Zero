#include <zero/prof.h>
#include <stdio.h>

int
main(int argc, char *argv)
{
    PROFTICK(cnt);

    profstarttick(cnt);
    printf("hello world\n");
    profstoptick(cnt);
    printf("%llu clock cycles to print\n", proftickdiff(cnt));

    return 0;
}

