/*
 * This code is based on libdivide:
 *
 * https://github.com/ridiculousfish/libdivide/
 *
 *  Copyright (C) 2010 ridiculous_fish
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty.  In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 *  libdivide@ridiculousfish.com
 */

#include <zero/param.h>
#include <zero/trix.h>

#if (LONGLONGSIZE == 8)
#define FASTULDIVSHIFTMASK 0x3f
#else
#error fastidiv.c not compatible with your system
#endif
#define FASTULDIVADDBIT    0x40
#define FASTULDIVSHIFTBIT  0x80

void
fastuldivgentab(struct divul *duptr, unsigned long lim)
{
    unsigned long long magic = 0;
    unsigned long long info = 0;
    unsigned long      div;

    for (div = 0 ; div < lim ; div++) {
        if (powerof2(div)) {
            info = tzerol(div);
            magic = 0;
            info |= FASTULDIVSHIFTBIT;
        } else {
            unsigned long      val;
            unsigned long long floor2;
            unsigned long long mul;
            unsigned long long rem;
            unsigned long long rem2;
            unsigned long long e;

            lzero32(div, floor2);
            val = 1ULL << floor2;
            mul = val / div;
            rem = val % div;
            e = div - rem;
            if (e < val) {
                info = floor2;
            } else {
                rem2 = rem;
                mul += mul;
                rem2 += rem;
                if (rem2 >= div || rem2 < rem) {
                    mul++;
                }
                info = floor2 | FASTULDIVADDBIT;
            }
            magic = ++mul;
        }
    }
    duptr->magic = magic;
    duptr->info = info;

    return;
}

static unsigned long
_mull32(unsigned long long val1, unsigned long long val2)
{
    unsigned long long val = val1 * val2;
    unsigned long      res = val >> 32;

    return res;
}

unsigned long
fastuldiv(unsigned long long num, const struct divul *denom)
{
    unsigned long long info = denom->info;
    unsigned long      res;

    if (info & FASTULDIVSHIFTBIT) {
        info &= FASTULDIVSHIFTMASK;
        res = num >> info;

        return res;
    } else {
        unsigned long long quot = _mull32(denom->magic, num);

        res = quot;
        if (info & FASTULDIVADDBIT) {
            num -= quot;
            info &= FASTULDIVSHIFTMASK;
            num >>= 1;
            res += num;
        }
        res >>= info;
    }

    return res;
}

