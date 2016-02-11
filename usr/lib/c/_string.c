#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>

void
_memsetbk(void *ptr, int byte, size_t len)
{
    long          l;
    long         *lptr1;
    long         *lptr2;
    uint8_t      *u8ptr = (uint8_t *)ptr + len;
    size_t        n = (uintptr_t)u8ptr & (sizeof(long) - 1);
    const size_t  delta = 8;

    if (!ptr || !len) {

        return;
    }
    if (len > delta * sizeof(long)) {
        len -= n;
        l = (long)(uint8_t)byte;
        l |= l << 8;
        l |= l << 16;
#if (LONGSIZE == 8)
        l |= l << 32;
#endif
        if (n) {
            while (n--) {
                *u8ptr = (uint8_t)byte;
                u8ptr--;
            }
        }
        lptr1 = (long *)u8ptr;
        n = (rounddownpow2(len, delta * sizeof(long)) / sizeof(long))
            & (delta - 1);
        if (n) {
            len -= n * sizeof(long);
            while (n--) {
                *--lptr1 = l;
            }
        }
        n = rounddownpow2(len, delta * sizeof(long)) / (delta * sizeof(long));
        if (n) {
            len -= n * delta * sizeof(long);
            lptr2 = lptr1 - delta;
            while (n--) {
                lptr1 = lptr2;
                lptr1[0] = l;
                lptr1[1] = l;
                lptr1[2] = l;
                lptr1[3] = l;
                lptr2 = lptr1 - delta;
                lptr1[4] = l;
                lptr1[5] = l;
                lptr1[6] = l;
                lptr1[7] = l;
            }
        }
        n = rounddownpow2(len, sizeof(long)) / sizeof(long);
        if (n) {
            len -= n * sizeof(long);
            while (n--) {
                *--lptr1 = l;
            }
        }
        u8ptr = (uint8_t *)lptr1;
    }
    while (len--) {
        *--u8ptr = (uint8_t)byte;
    }

    return;
}

