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
fastu32div32gentab(struct divu32 *duptr, uint32_t lim32)
{
    uint32_t magic = lim32;
    uint32_t info = 0;
    uint32_t div;
    uint32_t val;
    uint32_t shift;
    
    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    for (div = 1 ; div < lim32 ; div++) {
        duptr++;
        if (!powerof2(div)) {
            uint64_t val64;
            uint32_t rem;
            uint32_t rem2;
            uint32_t e;

            lzero32(div, val);
            shift = 31 - val;
            val64 = 1U << shift;
            val64 <<= 32;
            magic = val64 / div;
            rem = val64 % div;
            e = div - rem;
            if (e < (1U << shift)) {
                info = shift;
            } else {
                rem2 = rem;
                magic <<= 1;
                rem2 <<= 1;
                info = shift | FASTU32DIVADDBIT;
                if (rem2 >= div || rem2 < rem) {
                    magic++;
                }
            }
            magic++;
        } else {
            tzero32(div, info);
            magic = 0;
            info |= FASTU32DIVSHIFTBIT;
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
    uint32_t magic = lim16;
    uint16_t info = 0;
    uint32_t div;

    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    for (div = 1 ; div < lim16 ; div++) {
        duptr++;
        if (!powerof2(div)) {
            uint32_t val32;
            uint32_t rem2;
            uint32_t e;
            uint16_t val;
            uint16_t shift;
            uint16_t rem;

            lzero32(div, val);
            val -= 16;
            shift = 15 - val;
            val32 = UINT32_C(1) << shift;
            val32 <<= 16;
            magic = val32 / div;
            rem = val32 % div;
            e = div - rem;
            if (e < (1U << shift)) {
                info = shift;
            } else {
                rem2 = rem;
                magic <<= 1;
                rem2 <<= 1;
                info = shift | FASTU32DIVADDBIT;
                if (rem2 >= div || rem2 < rem) {
                    magic++;
                }
            }
            magic++;
        } else {
            tzero32(div, info);
            magic = 0;
            info |= FASTU32DIVSHIFTBIT;
        }
        duptr->magic = magic;
        duptr->info = info;
    }

    return;
}

