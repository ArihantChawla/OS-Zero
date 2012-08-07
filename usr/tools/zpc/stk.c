#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <zpc/zpc.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#if (ZPCX11)
#include "x11.h"
#endif

void zpcprintqueue(struct zpctoken *queue);

extern uint8_t     zpcoperchartab[256];

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
    struct zpcstkitem *item = zpcinputitem;
    char              *cp = (item) ? item->scur : NULL;

    if ((item) && (*str)) {
        if (cp != item->str && (!isxdigit(*str) || !isxdigit(cp[-1]))) {
            *cp++ = ' ';
        }
        while (*str) {
            if (cp == item->str + item->slen) {
                item->str = realloc(item->str, item->slen << 1);
                cp = item->str + item->slen;
                if (item->str) {
                    item->slen <<= 1;
                } else {
                    fprintf(stderr, "failed to expand stack item string\n");
                    
                    exit(1);
                }
            }
            *cp++ = *str++;
        }
        *cp = '\0';
        item->scur = cp;
    }

    return;
}

void
stkenterinput(void)
{
    struct zpcstkitem *item = zpcinputitem;

    if (item->scur != item->str) {
        item->tokq = zpctokenize(item->str);
        item->parseq = zpcparse(item->tokq);
        memmove(&zpcregstk[1], &zpcregstk[0],
                (NREGSTK - 1) * sizeof(struct zpctoken *));
        zpcregstk[0] = item->tokq;
#if 0
        if (item->parseq) {
            zpcregstk[0] = item->parseq;
        } else {
            zpcregstk[0] = item->tokq;
        }
#endif
//        stkaddinput();
    }
    item->scur = item->str;

    return;
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
