#ifndef __ZPC_ZPC_H__
#define __ZPC_ZPC_H__

#include <zero/cdefs.h>

#define ZPCNOPIDBITS      5
#define ZPCNOPFLAGBITS    3
#define ZPCNREGBITS       4
#define ZPCOPIMMEDBITS    6

#define ZPCADRIMMED       (1 << 0)
#define ZPCADRREG         (1 << 1)
#define ZPCADRINDEX       (1 << 2)
#define ZPCADRINDIR       (1 << 3)
#define ZPCARG0BIT        (1 << 4)
#define ZPCARG1BIT        (1 << 5)
#define ZPCADRARGSMASK    (ZPCARG0BIT | ZPCARG1BIT)
#define ZPCOPADRMODEBITS  6

#if defined(ZPC32BIT)
struct zpcop {
    unsigned id      : ZPCNOPIDBITS;            // 5-bit
    unsigned flg     : ZPCNOPFLAGBITS;          // 3-bit
    unsigned src     : ZPCNREGBITS;             // 4-bit
    unsigned dest    : ZPCNREGBITS;             // 4-bit
    unsigned adrmode : ZPCOPADRMODEBITS;        // 6-bit
    unsigned adrreg  : ZPCNREGBITS;             // 4-bit
    unsigned immed   : ZPCOPIMMEDBITS;          // 6-bit
    uint32_t args[EMPTY];                       // address/immediate arguments
};
#endif

#endif /* __ZPC_ZPC_H__ */

