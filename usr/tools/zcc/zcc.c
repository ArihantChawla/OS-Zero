#include <stdio.h>
#include <stdlib.h>
#include <zcc/zcc.h>
#if (ZCCPROF)
#include <zero/prof.h>
#endif

extern unsigned long     ntoken;
extern unsigned int      zccnfiles;
struct zccmach           zccmach;
static struct zcctoken **tokenqtab;

/* initialise machine-dependent parameters */
int
zccinitmach(void)
{
    int ret = 1;

    zccmach.ialn = sizeof(long);
    zccmach.faln = 16;

    return ret;
}

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
    if (!zccinitmach()) {
        fprintf(stderr, "cannot initialise %s\n", argv[0]);

        exit(1);
    }
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
            zppprintqueue(qp);
            qp++;
        }
    }
#endif
#if 0
    tokenqtab = calloc(input->nq + 1, sizeof(struct zcctoken *));
    for (l = 0 ; l < input->nq ; l++) {
        tokenqtab[l] = zpppreproc(input->qptr[l]->head);
    }
#endif

    exit(0);
}

