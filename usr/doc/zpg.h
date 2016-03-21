#ifndef __ZPG_H__
#define __ZPG_H__

#include <stddef.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zpg/zas.h>

struct zpgopcode;

/* operations for the LOGIC unit */
void zpgopnot(struct zpgopcode *op);
void zpgopand(struct zpgopcode *op);
void zpgopor(struct zpgopcode *op);
void zpgopxor(struct zpgopcode *op);
void zpgopshl(struct zpgopcode *op);
void zpgopshr(struct zpgopcode *op);
void zpgopsar(struct zpgopcode *op);
void zpgopror(struct zpgopcode *op);
void zpgoprol(struct zpgopcode *op);
void zpgopinc(struct zpgopcode *op);
void zpgopdec(struct zpgopcode *op);
void zpgopadd(struct zpgopcode *op);
void zpgopsub(struct zpgopcode *op);
void zpgopcmp(struct zpgopcode *op);
void zpgopmul(struct zpgopcode *op);
void zpgopdiv(struct zpgopcode *op);
void zpgopmod(struct zpgopcode *op);
void zpgopjmp(struct zpgopcode *op);
void zpgopbz(struct zpgopcode *op);
void zpgopbnz(struct zpgopcode *op);
void zpgopblt(struct zpgopcode *op);
void zpgopble(struct zpgopcode *op);
void zpgopbgt(struct zpgopcode *op);
void zpgopbge(struct zpgopcode *op);
void zpgopbo(struct zpgopcode *op);
void zpgopbno(struct zpgopcode *op);
void zpgopbc(struct zpgopcode *op);
void zpgopbnc(struct zpgopcode *op);
void zpgoppop(struct zpgopcode *op);
void zpgoppush(struct zpgopcode *op);
void zpgoppusha(struct zpgopcode *op);
void zpgopldr(struct zpgopcode *op);
void zpgopstr(struct zpgopcode *op);
#if 0
void zpgopmovl(struct zpgopcode *op);
void zpgopmovb(struct zpgopcode *op);
void zpgopmovw(struct zpgopcode *op);
void zpgopmovq(struct zpgopcode *op);
#endif
void zpgopcall(struct zpgopcode *op);
void zpgopenter(struct zpgopcode *op);
void zpgopleave(struct zpgopcode *op);
void zpgopret(struct zpgopcode *op);
void zpgoplmsw(struct zpgopcode *op);
void zpgopsmsw(struct zpgopcode *op);
void zpgopreset(struct zpgopcode *op);
void zpgophlt(struct zpgopcode *op);

#define ZPG_OP_NOP    0x00 // dummy operation
/* logical operations */
#define ZPG_OP_LOGIC  0x00
#define ZPG_OP_NOT    0x01 // 2's complement
#define ZPG_OP_AND    0x02 // logical AND
#define ZPG_OP_OR     0x03 // logical OR
#define ZPG_OP_XOR    0x04 // logical exclusive OR
/* shift operations */
#define ZPG_OP_SHIFT  0x01
#define ZPG_OP_SHL    0x01 // shift left (fill with zero)
#define ZPG_OP_SHR    0x02 // logical shift right (fill with zero)
#define ZPG_OP_SAR    0x03 // arithmetic shift right (fill with sign)
#define ZPG_OP_ROR    0x04 // rotate right
#define ZPG_OP_ROL    0x05 // rotate left
/* arithmetic operations */
#define ZPG_OP_ARITH  0x02
#define ZPG_OP_INC    0x01 // increment by one
#define ZPG_OP_DEC    0x02 // decrement by one
#define ZPG_OP_ADD    0x03 // addition
#define ZPG_OP_SUB    0x04 // subtraction
#define ZPG_OP_CMP    0x05 // compare
#define ZPG_OP_MUL    0x06 // multiplication
#define ZPG_OP_DIV    0x07 // division
#define ZPG_OP_MOD    0x08 // modulus
/* branch instructions */
#define ZPG_OP_BRANCH 0x03
#define ZPG_OP_JMP    0x01 // unconditional jump to given address
#define ZPG_OP_BZ     0x02 // branch if zero
#define ZPG_OP_BNZ    0x03 // branch if not zero
#define ZPG_OP_BLT    0x04 // branch if less than
#define ZPG_OP_BLE    0x05 // branch if less than or equal to
#define ZPG_OP_BGT    0x06 // branch if greater than
#define ZPG_OP_BGE    0x07 // branch if greater than or equal to
#define ZPG_OP_BO     0x08 // branch if overflow
#define ZPG_OP_BNO    0x09 // branch if no overflow
#define ZPG_OP_BC     0x0a // branch if carry
#define ZPG_OP_BNC    0x0b // branch if no carry
/* stack operations */
#define ZPG_OP_STACK  0x04
#define ZPG_OP_POP    0x01 // pop from stack
#define ZPG_OP_PUSH   0x02 // push to stack
#define ZPG_OP_PUSHA  0x03 // push all registers to stack
/* load-store */
#define ZPG_OP_MEM    0x05
#define ZPG_OP_LDR    0x01
#define ZPG_OP_STR    0x02
/* function calls */
#define ZPG_OP_FUNC   0x06
#define ZPG_OP_CALL   0x01 // call subroutine
#define ZPG_OP_ENTER  0x02 // subroutine prologue
#define ZPG_OP_LEAVE  0x03 // subroutine epilogue
#define ZPG_OP_RET    0x04 // return from subroutine
#define ZPG_OP_THR    0x05 // launch a new thread
/* system operations */
#define ZPG_OP_SYS    0x07
#define ZPG_OP_LMSW   0x01 // load machine status word
#define ZPG_OP_SMSW   0x02 // store machine status word
/* machine state */
#define ZPG_OP_MACH   0x08
#define ZPG_OP_RESET  0x01 // reset into well-known state
#define ZPG_OP_HLT    0x02 // halt execution
/* I/O */
#define ZPG_OP_IO     0x09
#define ZPG_OP_IN     0x01
#define ZPG_OP_OUT    0x02

/* number of virtual machine integral registers */
#define ZPG_NREG      16
#define ZPG_NFREG     16
#define ZPG_NCREG     4

#if 0
/* argument types for instructions */
#define ZPG_ARG_IMMED 0x00
#define ZPG_ARG_REG   0x01
#define ZPG_ARG_ADR   0x02
/* indexing flags for registers */
#if (ZPG_NREG == 16)
#define ZPG_REG_INDEX 0x40
#define ZPG_REG_INDIR 0x80
#elif (ZPG_NREG == 32)
#define ZPG_REG_INDEX 0x80
#define ZPG_REG_INDIR 0x100
#endif
#endif /* 0 */
/* mask of valid register IDs */
#define ZPG_REG_MASK  0x0f

/* number of units and instructions */
#define ZPG_NUNIT     16
#define ZPG_NOP       256

/* machine status word */
#define ZPG_ZF        0x01 // zero
#define ZPG_OF        0x02 // overflow
#define ZPG_CF        0x04 // carry
#define ZPG_IF        0x08 // interrupt pending

/* addressing modes for adrmode-member */
#define ZPG__REG_VAL  0       // %r
#define ZPG__IMM8_VAL 1       // op->imm8
#define ZPG__IMM_VAL  2       // op->args[0]
#define ZPG__REG_ADR  3       // *%r
#define ZPG__IMM_ADR  4       // *op->args[0]
#define ZPG__REG_NDX  5       // op->args[0](%r)

/* zpg instruction format */
struct zpgopcode {
    unsigned int unit    : 4;   // execution unit
    unsigned int code    : 4;   // instruction code
    unsigned int reg1    : 4;   // argument register
    unsigned int reg2    : 4;   // argument register
    unsigned int adrmode : 4;   // addressing mode
    unsigned int argsz   : 2;   // argument size is (8 << argsz)
    unsigned int _resv   : 2;   // reserved for future use
    unsigned int imm8    : 8;
#if 0
    unsigned int arg1t : 4; // argument #1 type
    unsigned int arg2t : 4; // argument #2 type
    unsigned int reg1  : 6; // registér #1 + ZPG_REG_INDIR or ZPG_REG_INDEX
    unsigned int reg2  : 6; // registér #2 + ZPG_REG_INDIR or ZPG_REG_INDEX
    unsigned int size  : 4; // instruction size
#if (!ZPG_32BIT)
    unsigned int pad   : 32;
#endif
#endif /* 0 */
    zasword_t    args[EMPTY]; // optional arguments
};

typedef void zpgopfunc_t(struct zpgopcode *);

/* cregs */
#define ZPG_MSW_CREG 0
#define ZPG_PC_CREG  1
#define ZPG_FP_CREG  2
#define ZPG_SP_CREG  3
/* zpg configuration and virtual machine structure */
struct zpg {
    zasword_t       regs[ZPG_NREG]; // virtual registers
    zasword_t       cregs[ZPG_NCREG];
#if !(ZPG_VIRTMEM)
    char           *physmem; // memory base address
    size_t          memsize; // memory size in bytes
#endif
    volatile long   shutdown; // shutdown flag
#if (ZPG_VIRTMEM)
    void          **pagetab;  // virtual memory structure
#endif
} ALIGNED(CLSIZE);

/* external declarations */
extern struct zasop  zpginsttab[ZPG_NOP];
extern struct zasop *zpgoptab[ZPG_NOP];
extern const char   *zpgopnametab[ZPG_NOP];
extern const char   *zpgopnargtab[ZPG_NOP];
extern struct zpg    zpg;

/* function prototypes */
int               zpgmain(int argc, char *argv[]);
void              zpginit(void);
void              zpginitopt(void);
size_t            zpginitmem(void);
void              zpginitio(void);
long              asmaddop(const char *str, struct zasop *op);
struct zasop    * zpgfindasm(const char *str);
struct zastoken * zasprocinst(struct zastoken *token, zasmemadr_t adr,
                              zasmemadr_t *retadr);
int8_t            zpgsigbus(zasmemadr_t adr, long size);
int8_t            zpgsigsegv(zasmemadr_t adr, long reason);
void              asmprintop(struct zpgyyopcode *op);

#define zpggetjmparg(vm, op)			                        \
  zpggetsrcarg(vm, op)
#define zpggetsrcarg(vm, op)                                            \
  (((op)->adr == ZPG_REG_VAL)						\
   ? (((vm)->regs[(op)->reg1])						\
      ? (((op)->adr == ZPG_IMM8_VAL)					\
	 ? ((op)->imm8)							\
           : ((op)->args[0]))))
#define zpggetdestarg(vm, op, retp)                                     \
  (((op)->adr == ZPG_REG_VAL)						\
   ? ((*(retp) = &(vm)->regs[(op)->reg2], *(retp))			\
        ? (((op)->adr == ZPG_IMM8_VAL)                                  \
           ? ((op)->imm8)                                               \
           : ((op)->args[0]))))
#define zpggetldrsrcarg(vm, op)                                         \
  (((op)->adr == ZPG_REG_ADR)						\
   ? (&(vm)->physmem[(op)->reg1])					\
   : (((op)->adr == ZPG_IMM_ADR)					\
      ? (&(vm)->physmem[(op)->args[0]])					\
      : (((op)->adr == ZPG_REG_NDX)					\
	 ? (&(vm)->physmem[(op)->reg1 + (op)->args[0]])			\
	 : NULL)))
#define zpggetstrdestarg(vm, op)                                        \
  (((op)->adr == ZPG_REG_ADR)						\
   ? (&(vm)->physmem[(op)->reg2])					\
   : (((op)->adr == ZPG_IMM_ADR)					\
      ? (&(vm)->physmem[(op)->args[0]])					\
      : (((op)->adr == ZPG_REG_NDX)					\
	 ? (&(vm)->physmem[(op)->reg2 + (op)->args[0]])			\
	 : NULL)))

#if 0
#define zpggetarg(op, arg1t, ptr)                                       \
  (((ptr) = &zpg.regs[(op)->reg1],					\
    ((arg1t) == ZPG_ARG_REG))                                            \
   ? zpg.regs[(op)->reg1]						\
     : (op)->args[0])
#define zpggetarg1(op, arg1t)                                           \
  ((arg1t) == ZPG_ARG_REG						\
   ? zpg.regs[(op)->reg1]						\
   : (op)->args[0])
#define zpggetarg2(op, arg1t, arg2t, ptr)                               \
  (((ptr) = &zpg.regs[(op)->reg2],					\
    ((arg2t) == ZPG_ARG_REG))						\
     ? zpg.regs[op->reg2]                                               \
   : (((arg1t) == ZPG_ARG_REG)						\
      ? (op)->args[0]							\
      : (op)->args[1]))
#define zpggetarg1mov(op, arg1t, arg2t)                                 \
  (((arg1t) == ZPG_ARG_REG)						\
   ? (((op)->reg1 & ZPG_REG_INDEX)                                      \
      ? zpggetmemt(zpg.regs[(op)->reg1 & ZPG_REG_MASK] + op->args[0],   \
		   zasword_t)						\
      : (((op)->reg1 & ZPG_REG_INDIR)                                   \
	 ? zpggetmemt(zpg.regs[(op)->reg1 & ZPG_REG_MASK], zasword_t)   \
	 : zpg.regs[(op)->reg1 & ZPG_REG_MASK]))                        \
   : (((arg1t) == ZPG_ARG_ADR)						\
      ? *(zasword_t *)zpgadrtoptr((op)->args[0])			\
      : (op)->args[0]))
#define zpggetarg2mov(op, arg1t, arg2t, ptr, t)                         \
  (((arg1t) == ZPG_ARG_REG)						\
   ? (((arg2t) == ZPG_ARG_REG)						\
      ? (((op)->reg2 & ZPG_REG_INDEX)                                   \
	 ? (ptr = (t *)zpgadrtoptr(zpg.regs[(op)->reg2 & ZPG_REG_MASK] + (op)->args[0]), \
	    *(ptr))							\
	 : (((op)->reg2 & ZPG_REG_INDIR)                                \
	    ? (ptr = (t *)zpgadrtoptr(zpg.regs[(op)->reg2 & ZPG_REG_MASK]), \
	       *(ptr))							\
	    : (ptr = (t *)&zpg.regs[(op)->reg2],			\
	       *(ptr))))						\
      : (ptr = (t *)zpgadrtoptr((op)->args[0]),				\
	 *(ptr)))							\
   : (((arg2t) == ZPG_ARG_REG)						\
      ? (((op)->reg2 & ZPG_REG_INDEX)                                   \
	 ? (ptr = (t *)zpgadrtoptr(zpg.regs[(op)->reg2 & ZPG_REG_MASK] + (op)->args[1]), \
	    *(ptr))							\
	 : (((op)->reg2 & ZPG_REG_INDIR)                                \
	    ? (ptr = (t *)zpgadrtoptr(zpg.regs[(op)->reg2 & ZPG_REG_MASK]), \
	       *(ptr))							\
	    : (ptr = (t *)&zpg.regs[(op)->reg2],			\
	       *(ptr))))						\
      : (ptr = (t *)zpgadrtoptr((op)->args[1]),				\
	 *(ptr))))
#if 0
#define zpggetarg2mov(op, arg1t, arg2t, ptr, t)                         \
  ((((arg1t) == ZPG_ARG_REG)						\
    ? ((((arg2t) == ZPG_ARG_REG)						\
	? (ptr = (t *)&zpg.regs[(op)->reg2],				\
	   *(ptr))							\
	: (((arg2t == ZPG_ARG_ADR)					\
	    ? (ptr = (t *)&(op)->args[0],				\
	       *(ptr))							\
	    : (ptr = (t *)&zpg.regs[(op)->args[0]],			\
	       *(ptr))))))						\
    : (((arg2t == ZPG_ARG_REG)						\
	? (ptr = (t *)&zpg.regs[(op)->reg2],				\
             *(ptr))                                                    \
	: (((arg2t == ZPG_ARG_ADR)					\
	    ? (ptr = (t *)&(op)->args[1],				\
                 *(ptr))                                                \
	    : (ptr = (t *)&zpg.regs[(op)->args[1]],			\
	       *(ptr))))))))
#endif
#endif /* 0 */

#define zpgsetzf(val)				\
  (!(val)					\
   ? (zpg.cregs[ZPG_MSW_CREG] |= ZPGZF)		\
   : (zpg.cregs[ZPG_MSW_CREG] &= ~ZPGZF))

#endif /* __ZPG_H__ */

