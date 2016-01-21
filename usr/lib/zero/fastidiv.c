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
#include <zero/fastidiv.h>
#include <zero/trix.h>

#if (FASTIDIVWORDSIZE == 64)
/*
 * This routine precomputes a lookup table for divisors 1..lim32
 * - table size is stored in item #0 to check for buffer overruns
 */
void
fastu64div32gentab(struct divu64 *duptr, uint64_t lim32)
{
    uint64_t magic = lim32;
    uint64_t info = 0;
    uint64_t div;

    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    for (div = 1 ; div < lim32 ; div++) {
        duptr++;
        if (!powerof2(div)) {
            uint64_t val;
            uint64_t shift;
            uint64_t mul;
            uint64_t rem;
            uint64_t rem2;
            uint64_t e;

            lzero32(div, shift);
            shift = 31 - shift;
            val = 1ULL << shift;
            mul = val / div;
            rem = val % div;
            e = div - rem;
            if (e < val) {
                info = shift;
            } else {
                rem2 = rem;
                mul += mul;
                rem2 += rem;
                info = shift | FASTU64DIVADDBIT;
                if (rem2 >= div || rem2 < rem) {
                    mul++;
                }
            }
            magic = ++mul;
        } else {
            info = tzerol(div);
            magic = 0;
            info |= FASTU64DIVSHIFTBIT;
        }
        duptr->magic = magic;
        duptr->info = info;
    }

    return;
}
#endif /* FASTIDIVWORDSIZE == 64 */

/*
 * This routine precomputes a lookup table for divisors 1..lim16
 * - table size is stored in item #0 to check for buffer overruns
 */
void
fastu32div16gentab(struct divu32 *duptr, uint32_t lim16)
{
    uint32_t magic = lim16;
    uint32_t info = 0;
    uint32_t div;

    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    for (div = 1 ; div < lim16 ; div++) {
        duptr++;
        if (!powerof2(div)) {
            uint32_t val;
            uint32_t shift;
            uint32_t mul;
            uint32_t rem;
            uint32_t rem2;
            uint32_t e;

            lzero32(div, shift);
            shift = 31 - shift;
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

/*
 * This routine precomputes a lookup table for divisors 1..lim20
 * - table size is stored in item #0 to check for buffer overruns
 */
void
fastu32div20gentab(struct divu32 *duptr, uint32_t lim20)
{
    uint32_t magic = lim20;
    uint32_t info = 0;
    uint32_t div;

    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    for (div = 1 ; div < lim20 ; div++) {
        duptr++;
        if (!powerof2(div)) {
            uint32_t val;
            uint32_t shift;
            uint32_t mul;
            uint32_t rem;
            uint32_t rem2;
            uint32_t e;

            lzero32(div, shift);
            shift = 31 - shift;
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
                info = shift | FASTU32DIV20ADDBIT;
                if (rem2 >= div || rem2 < rem) {
                    mul++;
                }
            }
            magic = ++mul;
        } else {
            info = tzerol(div);
            magic = 0;
            info |= FASTU32DIV20SHIFTBIT;
        }
        duptr->magic = magic;
        duptr->info = info;
    }

    return;
}

