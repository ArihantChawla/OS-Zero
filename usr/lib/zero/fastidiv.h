#ifndef __ZERO_FASTIDIV_H__
#define __ZERO_FASTIDIV_H__

#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#if defined(__KERNEL__)
#include <kern/util.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/x86/trap.h>
#else /* !defined(__KERNEL__) */
#include <stdlib.h>
#endif

#if (LONGLONGSIZE == 8)
#define FASTULDIVSHIFTMASK 0x3f
#else
#error fastidiv.c not compatible with your system
#endif
#define FASTULDIVADDBIT    0x40
#define FASTULDIVSHIFTBIT  0x80

struct divul {
    unsigned long long magic;
    unsigned long long info;
};

void fastuldiv32gentab(struct divul *duptr, unsigned long lim32);

/* get the high 32 bits of val1 * val2 */
static INLINE unsigned long
_mullhiu32(unsigned long long val1, unsigned long long val2)
{
    unsigned long long val = val1 * val2;
    unsigned long      res = val >> 32;

    return res;
}

/* compute num/div32 with [possible] multiplication + shift operations */
static INLINE unsigned long
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

#endif /* __ZERO_FASTIDIV_H__ */

