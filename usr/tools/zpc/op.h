#ifndef __ZPC_OP_H__
#define __ZPC_OP_H__

#define ZPCOPNOP  0
#define ZPCOPNOT  1
#define ZPCOPAND  2
#define ZPCOPOR   3
#define ZPCOPXOR  4
#define ZPCOPSHL  5
#define ZPCOPSHR  6
#define ZPCOPSAR  7
#define ZPCOPROL  8
#define ZPCOPROR  9
#define ZPCOPINC  10
#define ZPCOPDEC  11
#define ZPCOPADD  12
#define ZPCOPSUB  13
#define ZPCOPCMP  14
#define ZPCOPMUL  15
#define ZPCOPDIV  16
#define ZPCOPMOD  17
#define ZPCOPLEA  18
#define ZPCOPLDA  19
#define ZPCOPSTA  20
#define ZPCOPPSH  21
#define ZPCOPPOP  22
#define ZPCJMP    23
#define ZPCBE     24    // branch if equal
#define ZPCBZ     ZPCBE // branch if zero
#define ZPCBLT    25    // branch if less than
#define ZPCBLE    26    // branch if less than or equal
#define ZPCBGT    27    // branch if greater than
#define ZPCBGE    28    // branch if greater than or equal
#define ZPCOPCALL 29    // call subroutine
#define ZPCOPRET  30    // return from subroutine
#define ZPCOPIN   31    // read from I/O port
#define ZPCOPOUT  32    // write to I/O port
/* privileged mode instructions */
#define ZPCOPSETB 62    // set or clear bit in machine register
#define ZPCOPLDR  63    // load machine-register

#endif /* __ZPC_OP_H__ */

