#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void dumpcode(void *base, void *lim);

#define TEXT_WIDTH 80

extern void _start(void);
extern char _etext;

int
main(int argc, char *argv[])
{
    dumpcode((void *)&_start, &_etext);

    exit(0);
}

void
dumpcode(void *base, void *lim)
{
    uint8_t *ptr = base;
    int      x = 0;

    while (ptr < (uint8_t *)lim) {
        printf("%x", *ptr);
        x++;
        if (x == TEXT_WIDTH) {
            printf("\n");
            x = 0;
        }
        ptr++;
    }
    if (x) {
        printf("\n");
    }

    return;
}

