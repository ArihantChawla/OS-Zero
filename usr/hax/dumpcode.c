#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void dumpcode(void *base, void *lim);

extern void _start(void);
extern char _etext;

int
main(int argc, char *argv[])
{
    dumpcode(_start, &_etext);

    exit(0);
}

void
dumpcode(void *base, void *lim)
{
    uint8_t *ptr = base;

    if (ptr < (uint8_t *)lim) {
        printf("%2x", *ptr);
        ptr++;
        while (ptr < (uint8_t *)lim) {
            printf(" %02x", *ptr);
            ptr++;
        }
        printf("\n");
    }

    return;
}

