#include <stdio.h>
#include <stdlib.h>
#include <zcc/zcc.h>

typedef struct zcctoken * zpptokfunc_t(struct zpptoken *, struct zpptoken **);

struct zcctoken *
zpppreproc(struct zpptoken *token)
{
    struct zcctoken *ret = NULL;
    struct zpptoken *next;
    zpptokfunc_t    *func;

#if 0
    while (token) {
        func =  zppfunctab[next->type];
        ret = func(token, &next);
        if (!ret) {
            fprintf(stderr, "invalid token:\n");
#if (ZCCPRINT)
            zppprinttoken(token);
#endif
            exit(1);
        }
    }
#endif

    return ret;
}

