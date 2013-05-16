#include <stdio.h>
#include <stdlib.h>
#include <zcc/zcc.h>
#if (ZCCPROF)
#include <zero/prof.h>
#endif

extern unsigned long     ntoken;
extern unsigned int      zccnfiles;
static struct zcctoken **tokenqtab;

int
main(int argc, char *argv[])
{
    struct zppinput  *input;
    long              l;
#if (ZCCPRINT)
    struct zpptokenq *qp;
#endif
#if (ZCCPROF)
    PROFDECLCLK(clk);
#endif

#if (ZCCPROF)
    profstartclk(clk);
#endif
    input = zpplex(argc, argv);
    if (!input) {
        fprintf(stderr, "empty input\n");

        exit(1);
    }
#if (ZCCPROF)
    profstopclk(clk);
#if (ZPPTOKENCNT)
    fprintf(stderr, "%lu tokens\n", ntoken);
#endif
    fprintf(stderr, "%ld microseconds\n", profclkdiff(clk));
#endif
#if (ZCCPRINT)
    if (input) {
        l = input->nq;
        qp = *input->qptr;
        while (l--) {
            printqueue(qp);
            qp++;
        }
    }
#endif
    tokenqtab = calloc(zccnfiles, sizeof(struct zcctoken *));
    for (l = 0 ; l < input->nq ; l++) {
        tokenqtab[l] = zpppreproc(input->qptr[l]->head);
    }

    exit(0);
}

