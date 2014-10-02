#ifndef __ZVM_ZVM_H__
#define __ZVM_ZVM_H__

#include <stddef.h>
#include <stdint.h>
#include <zas/zas.h>

#define ZASMEMSIZE  (128U * 1024U * 1024U)
#define ZASNREG     16

#define ZASREGINDEX 0x04
#define ZASREGINDIR 0x08
#define ZVMARGIMMED 0x00
#define ZVMARGREG   0x01
#define ZVMARGADR   0x02

#define ZVMOPNOP    0x00 // dummy operation
/* logical operations */
#define ZVMOPLOGIC  0x00
#define ZVMOPNOT    0x01 // 2's complement
#define ZVMOPAND    0x02 // logical AND
#define ZVMOPOR     0x03 // logical OR
#define ZVMOPXOR    0x04 // logical exclusive OR
// /* shift operations */
#define ZVMOPSHIFT  0x01
#define ZVMOPSHR    0x01 // logical shift right (fill with zero)
#define ZVMOPSAR    0x02 // arithmetic shift right (fill with sign)
#define ZVMOPSHL    0x03 // shift left (fill with zero)
#define ZVMOPROR    0x04 // rotate right
#define ZVMOPROL    0x05 // rotate left
// /* arithmetic operations */
#define ZVMOPARITH  0x02
#define ZVMOPINC    0x01 // increment by one
#define ZVMOPDEC    0x02 // decrement by one
#define ZVMOPADD    0x03 // addition
#define ZVMOPSUB    0x04 // subtraction
#define ZVMOPCMP    0x05 // compare
#define ZVMOPMUL    0x06 // multiplication
#define ZVMOPDIV    0x07 // division
#define ZVMOPMOD    0x08 // modulus
// /* branch instructions */
#define ZVMOPBRANCH 0x03
#define ZVMOPJMP    0x01 // unconditional jump to given address
#define ZVMOPBZ     0x02 // branch if zero
#define ZVMOPBNZ    0x03 // branch if not zero
#define ZVMOPBLT    0x04 // branch if less than
#define ZVMOPBLE    0x05 // branch if less than or equal to
#define ZVMOPBGT    0x06 // branch if greater than
#define ZVMOPBGE    0x07 // branch if greater than or equal to
#define ZVMOPBO     0x08 // branch if overflow
#define ZVMOPBNO    0x09 // branch if no overflow
#define ZVMOPBC     0x0a // branch if carry
#define ZVMOPBNC    0x0b // branch if no carry
// /* stack operations */
#define ZVMOPSTACK  0x04
#define ZVMOPPOP    0x01 // pop from stack
#define ZVMOPPUSH   0x02 // push to stack
#define ZVMOPPUSHA  0x03 // push all registers to stack
// /* load-store */
#define ZVMOPLDSTR  0x05
#define ZVMOPMOV    0x01 // load/store 32-bit longword
#define ZVMOPMOVB   0x02 // load/store 8-bit byte
#define ZVMOPMOVW   0x03 // load/store 16-bit word
#define ZVMOPMOVQ   0x03 // load/store 64-bit quadword
// /* function calls */
#define ZVMOPFUNC   0x06
#define ZVMOPCALL   0x01 // call subroutine
#define ZVMOPENTER  0x02 // subroutine prologue
#define ZVMOPLEAVE  0x03 // subroutine epilogue
#define ZVMOPRET    0x04 // return from subroutine
// /* machine status word manipulation */
#define ZVMOPMSW    0x07
#define ZVMOPLMSW   0x01 // load machine status word
#define ZVMOPSMSW   0x02 // store machine status word
// /* machine state */
#define ZVMOPMACH   0x08
#define ZVMOPRESET  0x01 // reset into well-known state
#define ZVMOPHLT    0x02 // halt execution

#define ZVMNOP      256

/*
 * - unit
 * - operation code (within unit)
 * - flg  - addressing flags (REG, IMMED, INDIR, ...)
 */
struct zvmopcode {
    unsigned int  inst  : 8;
    unsigned int  arg1t : 4;
    unsigned int  arg2t : 4;
    unsigned int  reg1  : 6;
    unsigned int  reg2  : 6;
    unsigned int  size  : 4;
#if (!ZAS32BIT)
    unsigned int  pad   : 32;
#endif
    unsigned long args[EMPTY];
} PACK();

struct zvm {
    uint8_t *physmem;
    size_t   physsize;
};

struct zasop    * zvmfindasm(const uint8_t *str);
struct zastoken * zasprocinst(struct zastoken *token, zasmemadr_t adr,
                              zasmemadr_t *retadr);

#endif /* __ZVM_ZVM_H__ */

