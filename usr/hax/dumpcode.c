#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#define DUMPHEX_NPERLINE 16

void dumphex(void *base, uint8_t *lim);

extern void _start(void);
extern char _etext;

int
main(int argc, char *argv[])
{
    dumphex(_start, (uint8_t *)&_etext);

    exit(0);
}

void
dumphex(void *base, uint8_t *lim)
{
    uint8_t *ptr = base;
    int      x = 0;

    if (ptr < lim) {
        x = 0;
        while (ptr < lim) {
            if (x == DUMPHEX_NPERLINE) {
                printf("\n");
                x = 0;
            }
            if (x > 0) {
                printf("  ");
            }
            if (isprint(*ptr) && !isspace(*ptr)) {
                printf(" %c", *ptr);
            } else {
                printf("%02x", *ptr);
            }
            x++;
            ptr++;
        }
        printf("\n");
    }

    return;
}

