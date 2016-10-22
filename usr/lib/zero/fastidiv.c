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

#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <zero/fastidiv.h>

#if (WORDSIZE == 8)
/*
 * This routine precomputes a lookup table for divisors 1..lim32
 * - table size is stored in item #0 to check for buffer overruns
 */
void
fastu64div32gentab(struct divu64 *duptr, uint64_t lim32)
{
    uint32_t magic = lim32;
    uint32_t info = 0;
    uint32_t div;

    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    for (div = 1 ; div < lim32 ; div++) {
        duptr++;
        if (!powerof2(div)) {
            uint64_t val64;
            uint64_t rem2;
            uint32_t val;
            uint32_t shift;
            uint32_t mul;
            uint32_t rem;
            uint32_t e;

            lzero32(div, val);
            shift = 31 - val;
            val64 = 1ULL << shift;
            val64 <<= 32;
            mul = val64 / div;
            rem = val64 % div;
            e = div - rem;
            if (e < val) {
                info = shift;
            } else {
                rem2 = rem;
                mul <<= 1;
                rem2 <<= 1;
                info = shift | FASTU64DIV32ADDBIT;
                if (rem2 >= div || rem2 < rem) {
                    mul++;
                }
            }
            magic = ++mul;
        } else {
            info = tzerol(div);
            magic = 0;
            info |= FASTU64DIV32SHIFTBIT;
        }
        duptr->magic = magic;
        duptr->info = info;
    }

    return;
}
#endif /* WORDSIZE == 8 */

/*
 * This routine precomputes a lookup table for divisors 1..lim16
 * - table size is stored in item #0 to check for buffer overruns
 */
void
fastu32div16gentab(struct divu16 *duptr, uint32_t lim16)
{
    uint16_t magic = lim16;
    uint16_t info = 0;
    uint16_t div;

    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    for (div = 1 ; div < lim16 ; div++) {
        duptr++;
        if (!powerof2(div)) {
            uint32_t val32 = div;
            uint32_t rem2;
            uint16_t val;
            uint16_t shift;
            uint16_t mul;
            uint16_t rem;
            uint16_t e;

            lzero32(val32, val);
            val -= 16;
            shift = 15 - val;
            val32 = UINT32_C(1) << shift;
            val32 <<= 16;
            mul = val32 / div;
            rem = val32 % div;
            e = div - rem;
            if (e < (1U << shift)) {
                info = shift;
            } else {
                rem2 = rem;
                mul <<= 1;
                rem2 <<= 1;
                info = shift | FASTU32DIV16ADDBIT;
                if (rem2 >= div || rem2 < rem) {
                    mul++;
                }
            }
            magic = ++mul;
        } else {
            info = tzerol(div);
            magic = 0;
            info |= FASTU32DIV16SHIFTBIT;
        }
        duptr->magic = magic;
        duptr->info = info;
    }

    return;
}

#if 0
/*
 * This routine precomputes a lookup table for divisors 1..lim24
 * - table size is stored in item #0 to check for buffer overruns
 */
void
fastu32div24gentab(struct divu32 *duptr, uint32_t lim24)
{
    uint32_t magic = lim24;
    uint32_t info = 0;
    uint32_t div;

    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    for (div = 1 ; div < lim24 ; div++) {
        duptr++;
        if (!powerof2(div)) {
            uint32_t val;
            uint32_t shift;
            uint32_t mul;
            uint32_t rem;
            uint32_t rem2;
            uint32_t e;

            lzero32(div, val);
            shift = 31 - val;
            val = UINT32_C(1) << shift;
            mul = val / div;
            rem = val % div;
            e = div - rem;
            if (e < val) {
                info = shift;
            } else {
                rem2 = rem;
                mul += mul;
                rem2 += rem;
                info = shift | FASTU32DIV24ADDBIT;
                if (rem2 >= div || rem2 < rem) {
                    mul++;
                }
            }
            magic = ++mul;
        } else {
            info = tzerol(div);
            magic = 0;
            info |= FASTU32DIV24SHIFTBIT;
        }
        duptr->magic = magic;
        duptr->info = info;
    }

    return;
}
#endif /* 0 */

