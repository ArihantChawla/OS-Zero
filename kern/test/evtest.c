#include <kern/ev.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#if (EVTEST)
#include <stdlib.h>
#include <stdio.h>
#endif

struct evkbdqchar evkbdqueue ALIGNED(PAGESIZE);

int
main(int argc, char *argv[])
{
    int i;
    int j;

    for ( ; ; ) {
        for (i = 0 ; i < 256 ; i++) {
            fprintf(stderr, "QUEUE: %d\n", i);
            evqkbdchar(&evkbdqueue, i);
        }
        for (i = 0 ; i < 256 ; i++) {
            j = evdeqkbdchar(&evkbdqueue);
            if (j != i) {
                fprintf(stderr, "FAILURE: %d (%d)\n", i, j);

                exit(1);
            }
        }
    }

    exit(0);
}

