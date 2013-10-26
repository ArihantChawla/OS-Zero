#if (EVTEST)
#include <stdlib.h>
#include <stdio.h>
#endif
#include <kern/ev.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#if (EVPROF)
#include <zero/prof.h>
#endif

struct evkbdqchar evkbdqueue ALIGNED(PAGESIZE);
#if (EVPROF)
long              nread;
long              nwrite;
long              nrcycle;
long              nwcycle;
#endif

int
main(int argc, char *argv[])
{
    int i;
    int j;
#if (EVPROF)
    PROFDECLCLK(clk);
    PROFDECLTICK(tck);
#endif

    for ( ; ; ) {
        nread = 0;
        nwrite = 0;
        nrcycle = 0;
        nwcycle = 0;
#if (EVPROF) && 0
        profstartclk(clk);
#endif
        for (i = 0 ; i < 256 ; i++) {
#if (!EVPROF)
            fprintf(stderr, "QUEUE: %d\n", i);
#endif
#if (EVPROF)
            profstarttick(tck);
#endif
            evqkbdchar(&evkbdqueue, i);
#if (EVPROF)
            profstoptick(tck);
            nwrite++;
            nwcycle += proftickdiff(tck);
#endif
        }
        for (i = 0 ; i < 256 ; i++) {
#if (EVPROF)
            profstarttick(tck);
#endif
            j = evdeqkbdchar(&evkbdqueue);
#if (EVPROF)
            profstoptick(tck);
            nread++;
            nrcycle += proftickdiff(tck);
#endif
            if (j != i) {
                fprintf(stderr, "FAILURE: %d (%d)\n", i, j);

                exit(1);
            }
        }
#if (EVPROF) && 0
        profstopclk(clk);
        fprintf(stderr, "%ld\n", profclkdiff(clk));
#endif
#if (EVPROF)
        fprintf(stderr, "%ld cycles per read\n", nrcycle / nread);
        fprintf(stderr, "%ld cycles per write\n", nwcycle / nwrite);
#endif
    }

    exit(0);
}

