#ifndef __ZERO_FASTIDIV_H__
#define __ZERO_FASTIDIV_H__

#include <stdio.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#if defined(__KERNEL__)
#include <kern/util.h>
#include <kern/cpu.h>
#include <kern/unit/x86/trap.h>
#else /* !defined(__KERNEL__) */
#include <stdlib.h>
#endif

#define FASTU32DIVSHIFTMASK 0x1f
#define FASTU32DIVADDBIT    0x20
#define FASTU32DIVSHIFTBIT  0x40
    
struct divu16 {
    uint16_t magic;
    uint16_t info;
};

struct divu32 {
    uint32_t magic;
    uint32_t info;
};

#if (LONGSIZE == 8 || LONGLONGSIZE == 8)
void fastu32div32gentab(struct divu32 *duptr, uint32_t lim32);
#endif
void fastu32div16gentab(struct divu16 *duptr, uint32_t lim16);
//void fastu32div24gentab(struct divu32 *duptr, uint32_t lim24);

#if (LONGSIZE == 8 || LONGLONGSIZE == 8)
/* get the high 32 bits of val1 * val2 */
static INLINE uint32_t
_mullhiu32(uint32_t val1, uint32_t val2)
{
    uint64_t v1 = val1;
    uint64_t v2 = val2;
    uint64_t res = v1 * 2;

    res >>= 32;

    return (uint32_t)res;
}
#endif

/* get the high 16 bits of val1 * val2 */
static INLINE uint32_t
_mullhiu16(uint16_t val1, uint16_t val2)
{
    uint32_t v1 = val1;
    uint32_t v2 = val2;
    uint32_t res = v1 * v2;

    res >>= 16;

    return res;
}

/* NOTE: dividing 32-bit by 32-bit is currently broken */
#if (LONGSIZE == 8 || LONGLONGSIZE == 8)
/* compute num/div32 with [possible] multiplication + shift operations */
static INLINE uint32_t
fastu32div32(uint32_t num, uint32_t div32,
             const struct divu32 *tab)
{
    const struct divu32 *ulptr = &tab[div32];
    uint32_t             lim = tab[0].magic;
    uint32_t             magic = ulptr->magic;
    uint32_t             info = ulptr->info;
    uint32_t             res = 0;

    fprintf(stderr, "CALC: %u / %u\n", (unsigned int)num, (unsigned int)div32);
    fprintf(stderr, "DIV == %u, MAGIC == 0x%0.8x, INFO == %u\n",
            (unsigned int)div32, (unsigned int)magic, (unsigned int)info);
    if (div32 == 1) {
        
        return num;
    }
    if (div32 >= lim) {
        res = num / div32;

        return res;
    }
    res = num;
    if (!(info & FASTU32DIVSHIFTBIT)) {
        uint32_t quot = _mullhiu32(magic, num);

        if (info & FASTU32DIVADDBIT) {
            /* calculate ((num - quot) >> 1) + quot */
            num -= quot;
            num >>= 1;
            quot += num;
        }
        res = quot;
    }
    info &= FASTU32DIVSHIFTMASK;
    res >>= info;
        
    return res;
}
#endif

/* compute num/div16 with [possible] multiplication + shift operations */
static INLINE uint32_t
fastu32div16(uint32_t num, uint16_t div16,
             const struct divu16 *tab)
{
    const struct divu16 *ulptr = &tab[div16];
    uint16_t             lim = tab[0].magic;
    uint32_t             magic = ulptr->magic;
    uint32_t             info = ulptr->info;
    uint32_t             res = 0;

    if (div16 == 1) {

        return num;
    } else if (div16 >= lim) {
        res = num / div16;

        return res;
    }
    res = num;
    if (!(info & FASTU32DIVSHIFTBIT)) {
        uint32_t quot = _mullhiu16(magic, num);
        
        if (info & FASTU32DIVADDBIT) {
            /* calculate ((num - quot) >> 1) + quot */
            num -= quot;
            num >>= 1;
            quot += num;
        }
        res = quot;
    }
    info &= FASTU32DIVSHIFTMASK;
    res >>= info;
        
    return res;
}

#endif /* __ZERO_FASTIDIV_H__ */

