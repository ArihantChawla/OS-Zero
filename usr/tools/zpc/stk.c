#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zpc/zpc.h>

void zpcprintqueue(struct zpctoken *queue);

struct zpcstkitem *zpcinputitem;
struct zpctoken   *zpcregstk[NREGSTK];

void
stkaddinput(void)
{
    struct zpcstkitem *item = calloc(1, sizeof(struct zpcstkitem));

    if (item) {
        item->str = calloc(1, STKSTRLEN);
        if (item->str) {
            item->scur = item->str;
            item->slen = STKSTRLEN;
            item->next = zpcinputitem;
            zpcinputitem = item;
        } else {
            fprintf(stderr, "failed to allocate stack item string\n");

            exit(1);
        }
    } else {
        fprintf(stderr, "failed to allocate stack item\n");

        exit(1);
    }

    return;
}

void
stkqueueinput(const char *str)
{
    struct zpcstkitem *top = zpcinputitem;
    char              *cp = (top) ? top->scur : NULL;

    if ((top) && (*str)) {
        while (*str) {
            if (cp == top->str + top->slen) {
                top->str = realloc(top->str, top->slen << 1);
                cp = top->str + top->slen;
                if (top->str) {
                    top->slen <<= 1;
                } else {
                    fprintf(stderr, "failed to expand stack item string\n");
                    
                    exit(1);
                }
            }
            *cp++ = *str++;
        }
        *cp = '\0';
        top->scur = cp;
    }

    return;
}

void
stkenterinput(void)
{
    if (zpcinputitem->scur != zpcinputitem->str) {
        zpcinputitem->tokq = zpctokenize(zpcinputitem->str);
        zpcinputitem->parseq = zpcparse(zpcinputitem->tokq);
        memmove(&zpcregstk[1], &zpcregstk[0],
                (NREGSTK - 1) * sizeof(struct zpctoken *));
        if (zpcinputitem->parseq) {
            zpcregstk[0] = zpcinputitem->parseq;
        } else {
            zpcregstk[0] = zpcinputitem->tokq;
        }
        stkaddinput();
    }
}

void
stkinit(void)
{
    stkaddinput();

    return;
}

void
stkprint(void)
{
    int              i;

    i = NSTKREG;
    while (i--) {
        fprintf(stderr, "%d: ", i);
        zpcprintqueue(zpcregstk[i]);
        if (!zpcregstk[i]) {
            fprintf(stderr, "\n");
        }
    }

    return;
}
