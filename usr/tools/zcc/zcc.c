#include <ctype.h>
#include <stdio.h>
#include <stdint.h>

int
main(int argc,
     char *argv[])
{
    if (zpcprocopt(argc, argv)) {

        exit(1);
    }
}
