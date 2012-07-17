#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <zero/param.h>
#include <zero/trix.h>

int
main(int argc, char *argv[])
{
    unsigned long *bitmap = calloc(262144, sizeof(uint8_t));
    long           nbit = 262144 << 3;

    fprintf(stderr, "#0: %ld\n", bfindzerol(bitmap, 0, nbit));
    setbit(bitmap, 0);
    fprintf(stderr, "#1: %ld\n", bfindzerol(bitmap, 0, nbit));
    fprintf(stderr, "#2: %ld\n", bfindzerol(bitmap, 1, nbit));
    setbit(bitmap, 1);
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 0, nbit));
    fprintf(stderr, "#4: %ld\n", bfindzerol(bitmap, 1, nbit));
    memset(bitmap, 0xff, 5);
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 0, nbit));
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 40, nbit));
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 41, nbit));
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 48, nbit));
    memset(bitmap, 0xff, 262144);
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 0, nbit));
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 40, nbit));
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 41, nbit));
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 48, nbit));
    memset(bitmap, 0, 262144);
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 0, nbit));
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 40, nbit));
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 41, nbit));
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, 48, nbit));
    fprintf(stderr, "#3: %ld\n", bfindzerol(bitmap, nbit, nbit));

    exit(0);
}
