#ifndef __ZPC_ZPC32_H__
#define __ZPC_ZPC32_H__

#include <stdint.h>

typedef int32_t zpcreg_t;
typedef double  zpcfreg_t;

/* OPCODE structure */

/* # of registers */
#define ZPCREGS          (1 << ZPCREGBITS)
/* opcode bit-field widths */
#define ZPCOPIDBITS      5
#define ZPCOPFLAGBITS    3
#define ZPCREGBITS       4
#define ZPCOPIMMEDBITS   6
/* flg-bits */
#define ZPCOPLOCKBIT     (1 << 0)
#define ZPCOPFPUBIT      (1 << 1)
#define ZPCOPVECBIT      (1 << 2)
/* adrmode-bits */
#define ZPCARGIMMEDBIT   (1 << 0)
#define ZPCARGINDEXBIT   (1 << 1)
#define ZPCARGINDIRBIT   (1 << 2)
#define ZPCOPARGBIT      (1 << 3)
#define ZPCOPADRMODEBITS 4
#define zpcophasarg(op)  ((op)->argmode & ZPCOPARGBIT)
struct zpcop {
    unsigned id      : ZPCOPIDBITS;             // 5-bit
    unsigned flg     : ZPCOPFLAGBITS;           // 3-bit
    unsigned src     : ZPCREGBITS;              // 4-bit
    unsigned dest    : ZPCREGBITS;              // 4-bit
    unsigned adrmode : ZPCOPADRMODEBITS;        // 4-bit
    unsigned adrreg  : ZPCREGBITS;              // 4-bit
    unsigned immed   : ZPCOPIMMEDBITS;          // 8-bit
    uint32_t arg[EMPTY];                        // optional argument
};

#endif /* #ifndef __ZPC_ZPC32_H__ */

