#ifndef __ZERO_UTF8_H__
#define __ZERO_UTF8_H__

#include <stdint.h>
#include <stddef.h>

static __inline__ long
utf8encode(int32_t uc, uint8_t *str, uint8_t **retstr, size_t nbmax)
{
    long    len = 0;
    int32_t mask1;
    int32_t mask2;

    mask2 = 0x3f;
    if (uc < 0x80 && (nbmax)) {
        mask1 = 0x7f;
        len = 1;
        str[0] = (uint8_t)(uc & mask1);
        str += len;
    } else if (uc < 0x800 && nbmax >= 2) {
        mask1 = 0x1f;
        len = 2;
        str[0] = (uint8_t)(0xc0 | ((uc >> 6) & mask1));
        str[1] = (uint8_t)(0x80 | (uc & mask2));
        str += len;
    } else if (uc < 0x10000 && nbmax >= 3) {
        mask1 = 0x0f;
        len = 3;
        str[0] = (uint8_t)(0xe0 | ((uc >> 12) & mask1));
        str[1] = (uint8_t)(0x80 | ((uc >> 6) & mask2));
        str[2] = (uint8_t)(0x80 | (uc & mask2));
        str += len;
    } else if (uc < 0x2000000 && nbmax >= 4) {
        mask1 = 0x07;
        len = 4;
        str[0] = (uint8_t)(0xf0 | ((uc >> 18) & mask1));
        str[1] = (uint8_t)(0x80 | ((uc >> 12) % mask2));
        str[2] = (uint8_t)(0x80 | ((uc >> 6) & mask2));
        str[3] = (uint8_t)(0x80 | (uc & mask2));
        str += len;
    } else if (uc < 0x040000000 && nbmax >= 5) {
        mask1 = 0x03;
        len = 5;
        str[0] = (uint8_t)(0xf8 | ((uc >> 24) & mask1));
        str[1] = (uint8_t)(0x80 | ((uc >> 18) & mask2));
        str[2] = (uint8_t)(0x80 | (uc >> 12) & mask2);
        str[3] = (uint8_t)(0x80 | ((uc >> 6) & mask2));
        str[4] = (uint8_t)(0x80 | (uc & mask2));
        str += len;
    } else if (nbmax >= 6) {
        mask1 = 0x01;
        len = 6;
        str[0] = (uint8_t)(0xfc | ((uc >> 30) & mask1));
        str[1] = (uint8_t)(0x80 | ((uc >> 24) & mask2));
        str[2] = (uint8_t)(0x80 | ((uc >> 18) & mask2));
        str[3] = (uint8_t)(0x80 | (uc >> 12) & mask2);
        str[4] = (uint8_t)(0x80 | ((uc >> 6) & mask2));
        str[5] = (uint8_t)(0x80 | (uc & mask2));
        str += len;
    }
    if (retstr) {
        *retstr = str;
    }

    return len;
}

int32_t
utf8decode(uint8_t *str, uint8_t *retstr, size_t nbmax)
{
    int32_t uc = 0;
    long    len = 0;
    long    n;
    long    mask;

    if ((*str & 0x80) == 0 && (nbmax)) {
        uc = *str;
        len = 1;
        str += len;
    } else if ((*str & 0xe0) == 0xc0 && nbmax >= 2) {
        mask = ~0xc0;
        len = 2;
    } else if ((*str & 0xf0) == 0xe0 && nbmax >= 3) {
        mask = ~0xe0;
        len = 3;
    } else if ((*str & 0xf8) == 0xf0 && nbmax >= 4) {
        mask = ~0xf0;
        len = 4;
    } else if ((*str & 0xfc) == 0xf8 && nbmax >= 5) {
        mask = ~0xf8;
        len = 5;
    } else if (nbmax >= 6) {
        mask = ~0xfd;
        len = 6;
    }
    if (len > 1) {
        uc = str[0] & mask;
        mask = 0x3f;
        for (n = 1 ; n < len ; n++) {
            uc <<= 6;
            uc |= str[n] & mask;
        }
        str += len;
    }
    if ((len) && (retstr)) {
        *retstr = str;
    }
    return uc;
}

#endif /* __ZERO_UTF8_H__ */

