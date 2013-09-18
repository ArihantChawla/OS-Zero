#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DUMPCODE_NPERLINE 16

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
    int      x = 0;

    if (ptr < (uint8_t *)lim) {
        printf("%2x", *ptr);
        ptr++;
        while (ptr < (uint8_t *)lim) {
            x++;
            if (x != DUMPCODE_NPERLINE) {
                if (!(x & 0x03)) {
                    printf("    %02x", *ptr);
                } else {
                    printf(" %02x", *ptr);
                }
            } else {
                printf("\n%02x");
                x = 0;
            }
            ptr++;
        }
        printf("\n");
    }

    return;
}

