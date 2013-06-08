#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <zero/prof.h>
#include <vec/asm/mmx.h>

#define vecaddbu(adr1, adr2, nw)                                        \
    do {                                                                \
        uint8_t *_ptr1 = (uint8_t *)(adr1);                             \
        uint8_t *_ptr2 = (uint8_t *)(adr2);                             \
        uint8_t  _val1;                                                 \
        uint8_t  _val2;                                                 \
        uint8_t  _val3;                                                 \
        uint8_t  _val4;                                                 \
        int      _n = nw << 2;                                          \
                                                                        \
        while (_n--) {                                                  \
            _val1 = _ptr1[0];                                           \
            _val2 = _ptr2[0];                                           \
            _val3 = _ptr1[1];                                           \
            _val4 = _ptr2[1];                                           \
            _ptr2[0] = _val1 + _val2;                                   \
            _ptr2[1] = _val3 + _val4;                                   \
            _ptr1 += 2;                                                 \
            _ptr2 += 2;                                                 \
        }                                                               \
    } while (0)


#define runtest(test, nw)                                               \
    do {                                                                \
        sleep(5);                                                       \
        profstarttick(tick);                                            \
        test(vec1, vec2, nw);                                           \
        profstoptick(tick);                                             \
        fprintf(stderr, "%s: %lld ticks\n", #test, (long long)proftickdiff(tick)); \
    } while (0)
int
main(int argc, char *argv[])
{
#if 0
    int8_t vec1[8] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x6, 0x07 };
    int8_t vec2[8] = { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0xe, 0x0f };
#endif
    int8_t *vec1 = malloc(512 * sizeof(uint64_t));
    int8_t *vec2 = malloc(512 * sizeof(uint64_t));
    PROFDECLTICK(tick);

    runtest(vecaddbu, 512);
    runtest(vecaddbu, 512);
    runtest(vecaddbu_mmx, 512);

    exit(0);
}

