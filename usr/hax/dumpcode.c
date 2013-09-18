#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void dumpcode(void *base, void *lim);

extern void _start(void);
extern char _etext;

int
main(int argc, char *argv[])
{
    dumpcode((void *)_start, &_etext);

    exit(0);
}

void
dumpcode(void *base, void *lim)
{
    uint8_t *ptr = base;

    while (ptr < (uint8_t *)lim) {
        printf("%2x", *ptr);
        ptr++;
    }

    return;
}

