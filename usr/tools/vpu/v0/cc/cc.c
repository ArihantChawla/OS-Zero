#include <stdio.h>
#include <stdlib.h>
#include <cc/cc.h>
#include <cc/tune.h>
#if (CCPROF)
#include <zero/prof.h>
#endif

#if (CCSTAT)
extern unsigned long    tokcnttab[256];
extern char            *toknametab[256];
#endif

extern unsigned long    ntoken;
extern unsigned int     ccnfiles;
struct ccmach           ccmach;
static struct cctoken **tokenqtab;

/* initialise machine-dependent parameters */
int
ccinitmach(void)
{
    int ret = 1;

    ccmach.ialn = sizeof(long);
    ccmach.faln = 16;

    return ret;
}

int
main(int argc, char *argv[])
{
    struct ccinput  *input;
    long              l;
#if (CCPRINT)
    struct cctokenq *qp;
#endif
#if (CCPROF)
    PROFDECLCLK(clk);
#endif

#if (CCPROF)
    profstartclk(clk);
#endif
    if (!ccinitmach()) {
        fprintf(stderr, "cannot initialise %s\n", argv[0]);

        exit(1);
    }
    input = cclex(argc, argv);
    if (!input) {
        fprintf(stderr, "empty input\n");

        exit(1);
    }
#if (CCPROF)
    profstopclk(clk);
#if (CCTOKENCNT)
    fprintf(stderr, "%lu tokens\n", ntoken);
#endif
    fprintf(stderr, "%ld microseconds\n", profclkdiff(clk));
#endif
#if (CCSTAT)
    for (l = 1 ; l < CC_NTOKTYPE ; l++) {
        fprintf(stderr, "%s: %lu\n", toknametab[l], tokcnttab[l]);
    }
#endif
#if (CCPRINT)
    if (input) {
        l = input->nq;
        qp = *input->qptr;
        while (l--) {
            ccprintqueue(qp);
            qp++;
        }
    }
#endif
#if 0
    tokenqtab = calloc(input->nq + 1, sizeof(struct cctoken *));
    for (l = 0 ; l < input->nq ; l++) {
        tokenqtab[l] = ccpreproc(input->qptr[l]->head);
    }
#endif

    exit(0);
}

