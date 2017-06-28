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
#include <zero/fastudiv.h>

#if (FASTUF16DIVUF16)

/*
 * This routine precomputes a lookup table for divisors 2..lim16
 * - table size is stored in item #0 to check for buffer overruns
 */
void
fastuf16divuf16gentab(struct divuf16 *duptr, uint_fast32_t lim16)
{
    uint_fast32_t magic = lim16;
    uint_fast32_t info = 0;
    uint_fast32_t div;
    uint_fast32_t val;
    uint_fast32_t shift;

    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    duptr++;
    for (div = 2 ; div <= lim16 ; div++) {
        duptr++;
        lzero32(div, val);
        val -= 16;
        shift = 15 - val;
        if (!powerof2(div)) {
            uint_fast32_t val32;
            uint_fast32_t res32;
            uint_fast32_t rem;
            uint_fast32_t lim;
            uint_fast32_t e;

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
        duptr->magic = magic & 0xffff;
        duptr->info = info & 0xffff;
    }

    return;
}

#endif

#if (FASTUF16DIVUF32)

/*
 * This routine precomputes a lookup table for divisors 2..lim16
 * - table size is stored in item #0 to check for buffer overruns
 */
void
fastuf16divuf32gentab(struct divuf32 *duptr, uint_fast32_t lim16)
{
    uint_fast32_t magic = lim16;
    uint_fast32_t info = 0;
    uint_fast32_t div;
    uint_fast32_t val;
    uint_fast32_t shift;

    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    duptr++;
    for (div = 2 ; div <= lim16 ; div++) {
        duptr++;
        lzero32(div, val);
        val -= 16;
        shift = 15 - val;
        if (!powerof2(div)) {
            uint_fast32_t val32;
            uint_fast32_t res32;
            uint_fast32_t rem;
            uint_fast32_t lim;
            uint_fast32_t e;

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
        duptr->magic = magic & 0xffff;
        duptr->info = info & 0xffff;
    }

    return;
}

#endif

/*
 * This routine precomputes a lookup table for divisors 2..lim16
 * - table size is stored in item #0 to check for buffer overruns
 */
void
fastu16divu16gentab(struct divu16 *duptr, uint32_t lim16)
{
    uint32_t magic = lim16;
    uint32_t info = 0;
    uint32_t div;
    uint32_t val;
    uint32_t shift;

    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    duptr++;
    for (div = 2 ; div <= lim16 ; div++) {
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

