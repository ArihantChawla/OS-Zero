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

#include <zero/fastidiv.h>
#include <zero/trix.h>

/*
 * This routine precomputes a lookup table for divisors 1..lim32
 * - table size is stored in item #0 to check for buffer overruns
 */
void
fastuldiv32gentab(struct divul *duptr, unsigned long lim32)
{
    unsigned long long magic = lim32;
    unsigned long long info = 0;
    unsigned long      div;

    /* store array size into the first item to avoid buffer overruns */
    duptr->magic = magic;
    duptr->info = info;
    for (div = 1 ; div < lim32 ; div++) {
        duptr++;
        if (powerof2(div)) {
            info = tzerol(div);
            magic = 0;
            info |= FASTULDIVSHIFTBIT;
        } else {
            unsigned long      val;
            unsigned long long shift;
            unsigned long long mul;
            unsigned long long rem;
            unsigned long long rem2;
            unsigned long long e;

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
                info = shift | FASTULDIVADDBIT;
                rem2 += rem;
                if (rem2 >= div || rem2 < rem) {
                    mul++;
                }
            }
            magic = ++mul;
        }
        duptr->magic = magic;
        duptr->info = info;
    }

    return;
}

