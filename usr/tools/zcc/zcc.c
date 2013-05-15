#include <stdio.h>
#include <stdlib.h>
#include <zcc/zcc.h>
#if (ZCCPROF)
#include <zero/prof.h>
#endif

extern unsigned long     ntoken;
extern unsigned int      zccnfiles;
static struct zcctoken **tokenqtab;

typedef struct zcctoken * zcctokfunc_t(struct zpptoken *,
                                       struct zcctoken **);

static struct zcctoken  * zccprocglobal(struct zpptoken *token,
                                        struct zcctoken **tailret)
{
    return NULL;
}
static struct zcctoken  * zccprocmacro(struct zpptoken *token,
                                       struct zcctoken **tailret)
{
    return NULL;
}
static struct zcctoken  * zccprocfunc(struct zpptoken *token,
                                      struct zcctoken **tailret)
{
    return NULL;
}

#define ZCC_GLOBAL_CTX  0x00
#define ZCC_MACRO_CTX   0x01
#define ZCC_FUNC_CTX    0x02
static zcctokfunc_t *zccctxfunc[3] = {
    zccprocglobal,
    zccprocmacro,
    zccprocfunc
};

struct zcctoken *
zccpreproc(struct zpptoken *token)
{
    long             ctx = ZCC_GLOBAL_CTX;
    struct zcctoken *head = NULL;
    struct zcctoken *tail = NULL;
    zcctokfunc_t    *func;
    struct zcctoken *tok;

    while (token) {
        func =  zccctxfunc[ctx];
        tok = func(token, &tail);
        if (tok) {
            if (tok->type == ZPP_MACRO_TOKEN) {
                ctx = ZCC_MACRO_CTX;
            } else if (tok->type == ZPP_FUNC_TOKEN) {
                ctx = ZCC_FUNC_CTX;
            } else {
                ctx = ZCC_GLOBAL_CTX;
            }
            token = token->next;
        } else {
            fprintf(stderr, "invalid token:\n");
#if (ZCCPRINT)
            zppprinttoken(token);
#endif
            exit(1);
        }
    }

    return head;
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
    for (l = 0 ; l < zccnfiles ; l++) {
        tokenqtab[l] = zccpreproc(input->qptr[l]->head);
    }

    exit(0);
}

