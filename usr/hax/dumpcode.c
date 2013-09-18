#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DUMPHEX_NPERLINE 16

void dumphex(void *base, void *lim);

extern void _start(void);
extern char _etext;

int
main(int argc, char *argv[])
{
    dumphex(_start, &_etext);

    exit(0);
}

void
dumphex(void *base, void *lim)
{
    uint8_t *ptr = base;
    uint8_t *line;
    int      x = 0;

    if (ptr < (uint8_t *)lim) {
        line = ptr;
        printf("%2x", *ptr);
        ptr++;
        while (ptr < (uint8_t *)lim) {
            x++;
            if (x != DUMPHEX_NPERLINE) {
                if (!(x & 0x03)) {
                    printf("  %02x", *ptr);
                } else {
                    printf("%02x", *ptr);
                }
            } else {
                printf("    ");
                while (line < ptr) {
                    if (isprint(*line)) {
                        printf(" %c", *line);
                    } else {
                        printf("%02x", *line);
                    }
                    line++;
                }
                printf("\n%02x");
                x = 0;
            }
            ptr++;
        }
        printf("\n");
    }

    return;
}

