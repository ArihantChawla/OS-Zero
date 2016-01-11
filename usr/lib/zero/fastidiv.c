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

#if !defined(__KERNEL__)
#include <stdlib.h>
#endif
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/fastidiv.h>
#include <zero/trix.h>
#if defined(__KERNEL__)
#include <kern/util.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/x86/trap.h>
#endif

#if (LONGLONGSIZE == 8)
#define FASTULDIVSHIFTMASK 0x3f
#else
#error fastidiv.c not compatible with your system
#endif
#define FASTULDIVADDBIT    0x40
#define FASTULDIVSHIFTBIT  0x80

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

/* get the high 32 bits of val1 * val2 */
INLINE unsigned long
_mullhiu32(unsigned long long val1, unsigned long long val2)
{
    unsigned long long val = val1 * val2;
    unsigned long      res = val >> 32;

    return res;
}

/* compute num/div32 with [possible] multiplication + shift operations */
INLINE unsigned long
fastuldiv32(unsigned long long num, uint32_t div32,
            const struct divul *tab)
{
    const struct divul *ulptr = &tab[div32];
    unsigned long long  magic = ulptr->magic;
    unsigned long long  info = ulptr->info;
    unsigned long long  lim = tab->magic;
    unsigned long       res = 0;

    if (lim < div32 || !div32) {
#if defined(__KERNEL__)
        panic(k_curpid, -TRAPDE, 0);
#else
        abort();
#endif
    }
    if (!(info & FASTULDIVSHIFTBIT)) {
        unsigned long long quot = _mullhiu32(magic, num);
        
        res = quot;
        if (info & FASTULDIVADDBIT) {
            num -= quot;
            info &= FASTULDIVSHIFTMASK;
            num >>= 1;
            res += num;
        }
        res >>= info;

        return res;
    } else {
        info &= FASTULDIVSHIFTMASK;
        res = num >> info;
    }
        
    return res;
}

