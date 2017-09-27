#define BLAH 0

#include <stdio.h>
#include <stdlib.h>

void *malloc(size_t sz);

#define FOO \
    5

int
main(int argc, char *argv[])
{
    static int i = 4, j;

    if (argc == 2) {
        i = atoi(argv[1]);
    }
    while (i--) {
        printf("hello, world\n");
    }

    exit(0);
}

