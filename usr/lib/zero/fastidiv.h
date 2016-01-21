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

#if defined(__x86_64__) && defined(__amd64__)
#define FASTIDIVWORDSIZE 64
#elif (defined(__i386__) || defined(__i486__)                           \
       || defined(__i586__) || defined(__i686__)                        \
       || defined(__arm__))
#define FASTIDIVWORDSIZE 32
#endif

#if (LONGLONGSIZE == 8)
#define FASTU64DIV32SHIFTMASK 0x3f
#define FASTU64DIV32ADDBIT    0x40
#define FASTU64DIV32SHIFTBIT  0x80
#else
#warn fastu64div32() not supported by your system
#endif
#define FASTU32DIV16SHIFTMASK 0x0f
#define FASTU32DIV16ADDBIT    0x10
#define FASTU32DIV16SHIFTBIT  0x20
#define FASTU32DIV20SHIFTMASK 0x1f
#define FASTU32DIV20ADDBIT    0x20
#define FASTU32DIV20SHIFTBIT  0x40
    
struct divu32 {
    uint32_t magic;
    uint32_t info;
};

struct divu64 {
    uint64_t magic;
    uint64_t info;
};

#if (FASTIDIVWORDSIZE == 64)
void fastu64div32gentab(struct divu64 *duptr, uint64_t lim32);
#endif
void fastu32div16gentab(struct divu32 *duptr, uint32_t lim16);
void fastu32div20gentab(struct divu32 *duptr, uint32_t lim20);

/* get the high 32 bits of val1 * val2 */
static INLINE uint64_t
_mullhiu32(uint64_t val1, uint64_t val2)
{
    uint64_t val = val1 * val2;
    uint64_t res = val >> 32;

    return res;
}

/* get the high 16 bits of val1 * val2 */
static INLINE uint32_t
_mullhiu16(uint32_t val1, uint32_t val2)
{
    uint32_t val = val1 * val2;
    uint32_t res = val >> 16;

    return res;
}

/* get the high 20 bits of val1 * val2 */
static INLINE uint32_t
_mullhiu20(uint32_t val1, uint32_t val2)
{
    uint32_t val = val1 * val2;
    uint32_t res = val >> 12;

    return res;
}

/* compute num/div32 with [possible] multiplication + shift operations */
static INLINE uint64_t
fastu64div32(uint64_t num, uint32_t div32,
             const struct divu64 *tab)
{
    const struct divu64 *ulptr = &tab[div32];
    uint64_t             magic = ulptr->magic;
    uint64_t             info = ulptr->info;
    uint64_t             lim = tab->magic;
    uint64_t             res = 0;

    if (lim < div32 || !div32) {
#if defined(__KERNEL__)
        panic(k_curpid, -TRAPDE, 0);
#else
        abort();
#endif
    }
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

/* compute num/div16 with [possible] multiplication + shift operations */
static INLINE uint32_t
fastu32div16(uint32_t num, uint32_t div16,
             const struct divu32 *tab)
{
    const struct divu32 *ulptr = &tab[div16];
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

/* compute num/div16 with [possible] multiplication + shift operations */
static INLINE uint32_t
fastu32div20(uint32_t num, uint32_t div20,
             const struct divu32 *tab)
{
    const struct divu32 *ulptr = &tab[div20];
    uint32_t             magic = ulptr->magic;
    uint32_t             info = ulptr->info;
    uint32_t             lim = tab->magic;
    uint32_t             res = 0;

    if (lim < div20 || !div20) {
#if defined(__KERNEL__)
        panic(k_curpid, -TRAPDE, 0);
#else
        abort();
#endif
    }
    if (!(info & FASTU32DIV20SHIFTBIT)) {
        uint32_t quot = _mullhiu20(magic, num);
        
        res = quot;
        if (info & FASTU32DIV20ADDBIT) {
            num -= quot;
            info &= FASTU32DIV20SHIFTMASK;
            num >>= 1;
            res += num;
        }
        res >>= info;

        return res;
    } else {
        info &= FASTU32DIV20SHIFTMASK;
        res = num >> info;
    }
        
    return res;
}

#endif /* __ZERO_FASTIDIV_H__ */

