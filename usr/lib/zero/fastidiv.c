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

#if (WORDSIZE == 8) && 0
/*
 * This routine precomputes a lookup table for divisors 1..lim64
 * - table size is stored in item #0 to check for buffer overruns
 */
void
fastu64divgentab(struct divu64 *duptr, uint32_t lim64)
{
    uint64_t magic = lim64;
    uint64_t info = 0;
    uint64_t div;
    uint32_t val;
    uint32_t shift;
    
    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    duptr++;
    for (div = 2 ; div < lim64 ; div++) {
        duptr++;
        shift = 63;
        lzero64(div, val);
        shift -= val;
        if (!powerof2(div)) {
            uint64_t val64;
            uint64_t lim;
            uint64_t val;
            uint64_t rem;
            uint64_t e;

            lim = 1U << shift;
            magic = val64 / div;
            rem = val64 % div;
            e = div - rem;
            if (e < lim) {
                info = shift;
            } else {
                val = rem;
                magic <<= 1;
                val <<= 1;
                info |= FASTU32DIVADDBIT;
                if (val >= div || val < rem) {
                    magic++;
                }
            }
            magic++;
        } else {
            info = shift;
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
    uint32_t info = 0;
    uint32_t div;
    uint32_t val;
    uint32_t shift;

    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    for (div = 1 ; div < lim16 ; div++) {
        duptr++;
        lzero32(div, val);
        val -= 16;
        shift = 15 - val;
        if (!powerof2(div)) {
            uint32_t val32;
            uint32_t res32;
            uint32_t rem;
            uint32_t lim;
            uint32_t e;

            lim = UINT32_C(1) << shift;
            val32 = lim;
            val32 <<= 16;
            magic = val32 / div;
            /* elimnated rem = val32 % div */
            res32 = magic;
            res32 *= div;
            val32 -= res32;
            e = div - val32;
            if (e < lim) {
                info = shift;
            } else {
                rem = val32;
                magic <<= 1;
                rem <<= 1;
                info = shift | FASTU32DIVADDBIT;
                if (rem >= div || rem < val32) {
                    magic++;
                }
            }
            magic++;
        } else {
            info = shift;
            magic = 0;
            info |= FASTU32DIVSHIFTBIT;
        }
        duptr->magic = magic;
        duptr->info = info;
    }

    return;
}

