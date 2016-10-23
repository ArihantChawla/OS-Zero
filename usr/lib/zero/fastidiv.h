#ifndef __ZERO_FASTIDIV_H__
#define __ZERO_FASTIDIV_H__

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

#if (LONGSIZE == 8)
#define FASTU64DIV32SHIFTMASK 0x3fUL
#define FASTU64DIV32ADDBIT    0x40UL
#define FASTU64DIV32SHIFTBIT  0x80UL
#elif (LONGLONGSIZE == 8)
#define FASTU64DIV32SHIFTMASK 0x3fULL
#define FASTU64DIV32ADDBIT    0x40ULL
#define FASTU64DIV32SHIFTBIT  0x80ULL
#else
#warning fastu64div32() not supported by your system
#endif
#define FASTU32DIV16SHIFTMASK 0x0f
#define FASTU32DIV16ADDBIT    0x10
#define FASTU32DIV16SHIFTBIT  0x20
#define FASTU32DIV24SHIFTMASK 0x1f
#define FASTU32DIV24ADDBIT    0x20
#define FASTU32DIV24SHIFTBIT  0x40
    
struct divu16 {
    uint16_t magic;
    uint16_t info;
};

struct divu32 {
    uint32_t magic;
    uint32_t info;
};

struct divu64 {
    uint64_t magic;
    uint64_t info;
};

#if (LONGSIZE == 8 || LONGLONGSIZE == 8)
void fastu64div32gentab(struct divu32 *duptr, uint64_t lim32);
#endif
void fastu32div16gentab(struct divu16 *duptr, uint32_t lim16);
//void fastu32div24gentab(struct divu32 *duptr, uint32_t lim24);

/* get the high 32 bits of val1 * val2 */
static INLINE uint32_t
_mullhiu32(uint32_t val1, uint32_t val2)
{
    uint64_t v1 = val1;
    uint64_t v2 = val2;
    uint64_t res = v1 * 2;

    return (uint32_t)(res >> 32);
}

/* get the high 16 bits of val1 * val2 */
static INLINE uint16_t
_mullhiu16(uint16_t val1, uint16_t val2)
{
    uint32_t v1 = val1;
    uint32_t v2 = val2;
    uint32_t res = v1 * v2;

    return (uint16_t)(res >> 16);
}

#if (WORDSIZE == 8)

/* compute num/div32 with [possible] multiplication + shift operations */
static INLINE uint64_t
fastu64div32(uint64_t num, uint32_t div32,
             const struct divu32 *tab)
{
    const struct divu32 *ulptr = &tab[div32];
    uint32_t             magic = ulptr->magic;
    uint32_t             info = ulptr->info;
    uint32_t             lim = tab->magic;
    uint64_t             res = 0;

    if (!(info & FASTU64DIV32SHIFTBIT)) {
        uint64_t quot = _mullhiu32(magic, num);
        
        res = quot;
        if (info & FASTU64DIV32ADDBIT) {
            num -= quot;
            info &= FASTU64DIV32SHIFTMASK;
            num >>= 1;
            res += num;
        }
        res >>= info;

        return res;
    } else {
        info &= FASTU64DIV32SHIFTMASK;
        res = num >> info;
    }
        
    return res;
}

#endif /* WORDSIZE == 8 */

/* compute num/div16 with [possible] multiplication + shift operations */
static INLINE uint32_t
fastu32div16(uint32_t num, uint32_t div16,
             const struct divu16 *tab)
{
    const struct divu16 *ulptr = &tab[div16];
    uint32_t             magic = ulptr->magic;
    uint32_t             info = ulptr->info;
    uint32_t             lim = tab->magic;
    uint32_t             res = 0;

    if (lim < div16 || !div16) {
#if defined(__KERNEL__)
        panic(k_curpid, -TRAPDE, 0);
#else
        abort();
#endif
    }
    if (!(info & FASTU32DIV16SHIFTBIT)) {
        uint32_t quot = _mullhiu16(magic, num);
        
        res = quot;
        if (info & FASTU32DIV16ADDBIT) {
            num -= quot;
            info &= FASTU32DIV16SHIFTMASK;
            num >>= 1;
            res += num;
        }
        res >>= info;

        return res;
    } else {
        info &= FASTU32DIV16SHIFTMASK;
        res = num >> info;
    }
        
    return res;
}

#if 0
/* compute num/div16 with [possible] multiplication + shift operations */
static INLINE uint32_t
fastu32div24(uint32_t num, uint32_t div24,
             const struct divu32 *tab)
{
    const struct divu32 *ulptr = &tab[div24];
    uint32_t             magic = ulptr->magic;
    uint32_t             info = ulptr->info;
    uint32_t             lim = tab->magic;
    uint32_t             res = 0;

    if (lim < div24 || !div24) {
#if defined(__KERNEL__)
        panic(k_curpid, -TRAPDE, 0);
#else
        abort();
#endif
    }
    if (!(info & FASTU32DIV24SHIFTBIT)) {
        uint32_t quot = _mullhiu24(magic, num);
        
        res = quot;
        if (info & FASTU32DIV24ADDBIT) {
            num -= quot;
            info &= FASTU32DIV24SHIFTMASK;
            num >>= 1;
            res += num;
        }
        res >>= info;

        return res;
    } else {
        info &= FASTU32DIV24SHIFTMASK;
        res = num >> info;
    }
        
    return res;
}
#endif

#endif /* __ZERO_FASTIDIV_H__ */

