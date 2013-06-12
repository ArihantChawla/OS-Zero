#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <zero/prof.h>
#include <zero/trix.h>
#include <vec/vec.h>
#include <vec/asm/mmx.h>
#include <vec/asm/sse2.h>

#define runtest4(p1, p2, test, nw)                                      \
    do {                                                                \
        sleep(5);                                                       \
        profstarttick(tick);                                            \
        test(p1, p2, nw);                                               \
        profstoptick(tick);                                             \
        fprintf(stderr, "%s: %lld ticks\n", #test, (long long)proftickdiff(tick)); \
    } while (0)

int
main(int argc, char *argv[])
{
    uint8_t *vec[12];
    int     i;
    int     j;
#if 0
    int8_t vec1[8] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x6, 0x07 };
    int8_t vec2[8] = { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0xe, 0x0f };
#endif
    vec[0] = calloc(512, sizeof(uint64_t));
    vec[1] = calloc(512, sizeof(uint64_t));
    vec[2] = calloc(512, sizeof(uint64_t));
    vec[3] = calloc(512, sizeof(uint64_t));
    vec[4] = calloc(512, sizeof(uint64_t));
    vec[5] = calloc(512, sizeof(uint64_t));
    vec[6] = calloc(512, sizeof(uint64_t));
    vec[7] = calloc(512, sizeof(uint64_t));
    vec[8] = calloc(512, sizeof(uint64_t));
    vec[9] = calloc(512, sizeof(uint64_t));
    vec[10] = calloc(512, sizeof(uint64_t));
    vec[11] = calloc(512, sizeof(uint64_t));
    PROFDECLTICK(tick);

    for (i = 0 ; i < 10 ; i += 2) {
        srand(1);
        for (j = 0 ; j < 4096 ; j++) {
            vec[i][j] = rand() & 0xff;
        }
    }
    for (i = 0 ; i < 10 ; i += 2) {
        srand(2);
        for (j = 0 ; j < 4096 ; j++) {
            vec[i][j] = rand() & 0xff;
        }
    }

    runtest4(vec[0], vec[1], vecaddbus, 512);
    runtest4(vec[2], vec[3], vecaddbus_mmx, 512);
    runtest4(vec[4], vec[5], vecaddbus_sse2, 512);
    for (i = 0 ; i < 4096 ; i++) {
        int8_t i1 = vec[3][i];
        int8_t i2 = vec[5][i];
        if (i1 != i2) {
            fprintf(stderr, "vecaddbus_sse2 error: i=%x, %x != %x\n (%x + %x)",
                    i, i2, i1, vec[4][i], vec[7][i]);

            exit(1);
        }
    }
    runtest4(vec[6], vec[7], vecaddbss, 512);
    runtest4(vec[8], vec[9], vecaddbss_sse2, 512);
    for (i = 0 ; i < 4096 ; i++) {
        int i1 = vec[7][i];
        int i2 = vec[9][i];
        if (i1 != i2) {
            fprintf(stderr, "vecaddbs_sse2 error: i=%d, %d != %d (%d + %d)\n",
                    i, i2, i1, vec[0][i], vec[1][i]);

            exit(1);
        }
    }
    runtest4(vec[10], vec[11], vecaddbsu_mmx, 512);

    exit(0);
}

