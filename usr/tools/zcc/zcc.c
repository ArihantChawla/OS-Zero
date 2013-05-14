#include <stdio.h>
#include <stdlib.h>
#include <zcc/zcc.h>
#if (ZCCPROF)
#include <zero/prof.h>
#endif

extern unsigned long ntoken;

int
main(int argc, char *argv[])
{
    struct zccinput  *input;
#if (ZCCPRINT)
    struct zcctokenq *qp;
    long              l;
#endif
#if (ZCCPROF)
    PROFDECLCLK(clk);
#endif

#if (ZCCPROF)
    profstartclk(clk);
#endif
    input = zcclex(argc, argv);
    if (!input) {
        fprintf(stderr, "empty input\n");

        exit(1);
    }
#if (ZCCPROF)
    profstopclk(clk);
#if (ZCCTOKENCNT)
    fprintf(stderr, "%lu tokens\n", ntoken);
#endif
    fprintf(stderr, "%ld microseconds\n", profclkdiff(clk));
#endif
#if (ZCCPRINT) && 0
    if (input) {
        l = input->nq;
        qp = *input->qptr;
        while (l--) {
            printqueue(qp);
            qp++;
        }
    }
#endif

    exit(0);
}

