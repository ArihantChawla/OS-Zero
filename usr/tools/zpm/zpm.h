#ifndef __ZPM_ZPM_H__
#define __ZPM_ZPM_H__

#include <zpm/conf.h>
#include <stdint.h>
#include <endian.h>
#include <zero/cdefs.h>

typedef int8_t   zpmbyte;
typedef uint8_t  zpmubyte;
typedef int16_t  zpmword;
typedef uint16_t zpmuword;
typedef int32_t  zpmlong;
typedef uint32_t zpmulong;
#if defined(ZPM64BIT)
typedef int64_t  zpmquad;
typedef uint64_t zpmuquad;
typedef zpmquad  zpmreg;
typedef zpmuquad zpmureg;
#else
typedef zpmlong  zpmreg;
typedef zpmulong zpmureg;
#endif
typedef zpmureg  zpmadr;

/* INSTRUCTION SET */

/* ALU (arithmetic-logical unit) instructions */

/* bitwise operations */
#define ZPM_BITS_UNIT   0x00
#define ZPM_TRAIL_BIT   0x01
#define ZPM_BITPOP_BIT  0x02
#define ZPM_BITCNT_BIT  0x04
#define ZPM_NOT         0x00 // 2's complement (reverse all bits)
#define ZPM_AND         0x01 // logical bitwise AND
#define ZPM_OR          0x02 // logical bitwise OR
#define ZPM_XOR         0x03 // logical bitwise XOR (exclusive OR)
// count leading zero bits
#define ZPM_CLZ         ZPM_BITCNT_BIT // 0x04
// count trailing zero bits
#define ZPM_CTZ         (ZPM_BITCNT_BIT | ZPM_TRAIL_BIT) // 0x05
// count 1-bits */
#define ZPM_BPOP        (ZPM_BITCNT_BIT | ZPM_BITPOP_BIT) // 0x06

/* shifter */
#define ZPM_SHIFT_UNIT  0x01
#define ZPM_RIGHT_BIT   0x01
#define ZPM_SHIFT_BIT   0x02
#define ZPM_ROT_BIT     0x04
#define ZPM_SIGN_BIT    0x08
#define ZPM_SHL         ZPM_SHIFT_BIT
#define ZPM_SHR         (ZPM_SHIFT_BIT | ZPM_RIGHT_BIT)
#define ZPM_SAR         (ZPM_SHIFT_BIT | ZPM_RIGHT_BIT | ZPM_SIGN_BIT)
#define ZPM_ROL         ZPM_ROT_BIT
#define ZPM_ROR         (ZPM_ROT_BIT | ZPM_RIGHT_BIT)

/* arithmetic operations */
/* arithmetic unit */
#define ZPM_ARITH_UNIT  0x02
#define ZPM_INC_BIT     0x01
#define ZPM_MSW_BIT     0x01
#define ZPM_DEC_BIT     0x02
#define ZPM_ADD_BIT     0x02
#define ZPM_SUB_BIT     0x04
#define ZPM_ADD_INV_BIT 0x04
#define ZPM_CPY         0x00
#define ZPM_INC         ZPM_INC_BIT
#define ZPM_ADD         ZPM_ADD_BIT
#define ZPM_ADF         (ZPM_ADD_BIT | ZPM_MSW_BIT)
#define ZPM_ADI         (ZPM_ADD_INV_BIT) // a + ~b
#define ZPM_SUB         (ZPM_ADD_INV_BIT | ZPM_INC_BIT) // a + ~b + 1
#define ZPM_SBF         (ZPM_ADD_INV_BIT | ZPM_INC_BIT | ZPM_MSW_BIT)
#define ZPM_DEC         (ZPM_SUB_BIT | ZPM_DEC_BIT)
#define ZPM_CMP         ZPM_SBF

/* multiplier */
#define ZPM_MUL_UNIT    0x03
#define ZPM_HIGH_BIT    0x01
#define ZPM_WIDE_BIT    0x02
#define ZPM_MUL         0x00 // multiplication
#define ZPM_MULHI       (ZPM_HIGH_BIT) // return high-word of result
#define ZPM_MULW        (ZPM_WIDE_BIT) // return both words of result in %r0:%r1

/* divider */
#define ZPM_DIV_UNIT    0x04
#define ZPM_REM_BIT     0x01 // remainder-flag
#define ZPM_DIV         0x00 // division, result in ZPM_RET_LO
#define ZPM_REM         ZPM_REM_BIT // remainder of division in ZPM_RET_HI

/* load-store and stack operations */
#define ZPM_MEM_UNIT    0x05
#define ZPM_LOAD_BIT    0x01 // read operation
#define ZPM_MEM_BIT     0x02 // memory transfer
#define ZPM_STACK_BIT   0x04 // stack operation
#define ZPM_GENREGS_BIT 0x08 // choose all general-purpose registers
#define ZPM_CPY         ZPM_LOAD_BIT // register to register copy
#define ZPM_STA         ZPM_MEM_BIT // store accumulator
#define ZPM_LDA         (ZPM_LOAD_BIT | ZPM_MEM_BIT) // load accumulator
#define ZPM_PSH         ZPM_STACK_BIT // push register
#define ZPM_POP         (ZPM_LOAD_BIT | ZPM_STACK_BIT)
#define ZPM_PSHA        (ZPM_STACK_BIT | ZPM_GENREGS_BIT)
#define ZPM_POPA        (ZPM_LOAD_BIT | ZPM_STACK_BIT | ZPM_GENREGS_BIT)

/* I/O operations */
#define ZPM_IO_UNIT     0x06
#define ZPM_OUT_BIT     0x01
#define ZPM_16_BIT      0x02
#define ZPM_32_BIT      0x04
#define ZPM_64_BIT      0x08
#define ZPM_IOR8        0x00
#define ZPM_IOW8        (ZPM_IOW_BIT) // 0x01
#define ZPM_IOR16       (ZPM_16_BIT) // 0x02
#define ZPM_IOW16       (ZPM_16_BIT | ZPM_IOW_BIT) // 0x03
#define ZPM_IOR32       (ZPM_32_BIT) // 0x04
#define ZPM_IOW32       (ZPM_32_BIT | ZPM_IOW_BIT) // 0x05
#define ZPM_IOR64       (ZPM_64_BIT) // 0x08
#define ZPM_IOW64       (ZPM_64_BIT | ZPM_IOW_BIT) // 0x09

/* flow control; branch and subroutine operations */
#define ZPM_FLOW_UNIT   0x07
#define ZPM_ZERO_BIT    0x01
#define ZPM_NOTZERO_BIT 0x02
#define ZPM_NOTOVER_BIT 0x02
#define ZPM_OVER_BIT    0x04
#define ZPM_NOCARRY_BIT 0x02
#define ZPM_CARRY_BIT   0x04
#define ZPM_EQUAL_BIT   0x01
#define ZPM_UNEQUAL_BIT 0x02
#define ZPM_LESS_BIT    0x08
#define ZPM_GREATER_BIT 0x08
#define ZPM_JMP         0x00
#define ZPM_BZ          ZPM_ZERO_BIT
#define ZPM_BNZ         ZPM_NOTZERO_BIT
#define ZPM_CALL        0x03
#define ZPM_BO          (ZPM_OVER_BIT)
#define ZPM_BNO         (ZPM_OVER_BIT | ZPM_ZERO_BIT) // 0x05
#define ZPM_BC          (ZPM_CARRY_BIT | ZPM_NOTZERO_BIT) // 0x06
#define ZPM_BNC         (ZPM_CARRY_BIT | ZPM_NOCARRY_BIT | ZPM_ZERO_BIT) // 0x07
#define ZPM_BLT         (ZPM_LESS_BIT) // 0x08
#define ZPM_BLE         (ZPM_LESS_BIT | ZPM_EQUAL_BIT) // 0x09
#define ZPM_BGT         (ZPM_GREATER_BIT | ZPM_UNEQUAL_BIT) // 0x0a
// 0x0b
#define ZPM_BGE         (ZPM_GREATER_BIT | ZPM_UNEQUAL_BIT | ZPM_EQUAL_BIT)
#define ZPM_TASK        0x0c // launch new task (process, thread, fiber)
#define ZPM_RET         0x0d // return from subroutine
#define ZPM_TRET        0x0e // return from thread
#define ZPM_IRET        0x0f // return from interrupt handler

/* inter-processor atomic operations (bus locked) */
#define ZPM_ATOM_UNIT   0x08
#define ZPM_BSET        0x00 // set bit (OR)
#define ZPM_BCLR        0x01 // clear bit (XOR)
#define ZPM_BCHG        0x02 // flip/toggle bit (NOT)
#define ZPM_BTAS        0x03 // test (fetch) and set bit
#define ZPM_BTAC        0x04 // test (fetch) and clear bit
#define ZPM_BCSET       0x05 // compare and set bit
#define ZPM_BCCLR       0x06 // compare and clear bit
#define ZPM_FINC        0x07 // fetch and increment
#define ZPM_FDEC        0x08 // fetch and decrement
#define ZPM_FADD        0x09 // fetch and add
#define ZPM_FSUB        0x0a // fetch and subtract
#define ZPM_CAS         0x0b // compare and swap
#define ZPM_CAS2        0x0c // dual-word/pointer compare and swap
// NOTE: 0x0d, 0x0e, 0x0f reserved for stuff such as LL/SC

/* bitfield operations */
// NOTE: use width == 0, pos == 0 to indicate operations on whole registers
#define ZPM_BITF_UNIT   0x09
/* shift and rotate operations */
#define ZPM_BFRIGHT_BIT 0x01
#define ZPM_BFONE_BIT   0x01
#define ZPM_BFSIGN_BIT  0x02
#define ZPM_BFZERO_BIT  0x02
#define ZPM_BFROR_BIT   0x04
#define ZPM_BFSHIFT_BIT 0x04
#define ZPM_BFROT_BIT   0x08
/* hamming weight / bit-population */
#define ZPM_BFTRAIL_BIT 0x01
#define ZPM_BFCNT_BIT   0x02
#define ZPM_BFPOP_BIT   0x04
#define ZPM_BFBITS_BIT  0x08
/* fill operations */
#define ZPM_BFONES_BIT  0x01
#define ZPM_BFWRITE_BIT 0x04
#define ZPM_BFFILL_BIT  0x08
#define ZPM_BFXFER_BIT  0x08
/* move operations */
#define ZPM_BFSAVE_BIT  0x01
#define ZPM_BFXFER_MASK 0x0e
/* instructions */
#define ZPM_BFNOT       0x00
#define ZPM_BFAND       0x01
#define ZPM_BFOR        0x02
#define ZPM_BFXOR       0x03
#define ZPM_BFSHL       ZPM_BFSHIFT_BIT // 0x04
#define ZPM_BFSAR       (ZPM_BFSHIFT_BIT | ZPM_BFRIGHT_BIT) // 0x05
#define ZPM_BFSHR       (ZPM_BFSHIFT_BIT | ZPM_BFSIGN_BIT) // 0x06
#define ZPM_BFPOP       (ZPM_BFPOP_BIT | ZPM_BFCNT_BIT | ZPM_BFONE_BIT) // 0x07
#define ZPM_BFROL       (ZPM_BFROT_BIT) // 0x08
#define ZPM_BFROR       (ZPM_BFROT_BIT | ZPM_BFRIGHT_BIT) // 0x09
#define ZPM_BFCLZ       (ZPM_BFBITS_BIT | ZPM_BFCNT_BIT) // 0x0a
// 0x0b
#define ZPM_BFCTZ       (ZPM_BFBITS_BIT | ZPM_BFCNT_BIT | ZPM_BFTRAIL_BIT)
#define ZPM_BFCLR       (ZPM_BFFILL_BIT | ZPM_BFWRITE_BIT) // 0x0c
// 0x0d
#define ZPM_BFSET       (ZPM_BFFILL_BIT | ZPM_BFWRITE_BIT | ZPM_BFONES_BIT)
#define ZPM_BFLOAD      (ZPM_BFXFER_MASK) // 0x0e
#define ZPM_BFSAVE      (ZPM_BFXFER_MASK | ZPM_BSAVE_BIT) // 0x0f

/* system operations */
#define ZPM_SYS_UNIT    0x0a
#define ZPM_RESET_BIT   0x01
#define ZPM_XREG_BIT    0x04
#define ZPM_XLOAD_BIT   0x01
#define ZPM_XCPY_BIT    0x01
#define ZPM_XSTORE_BIT  0x02
#define ZPM_EVENT_BIT   0x08
#define ZPM_SEND_BIT    0x01
#define ZPM_INTRON_BIT  0x01
#define ZPM_CLRTRAP_BIT 0x01
#define ZPM_TRAP_BIT    0x02
#define ZPM_INTR_BIT    0x04
#define ZPM_SYSRTN_MASK 0x0e
#define ZPM_SYSRET_BIT  0x01
#define ZPM_BOOT        0x00 // execute bootstrap code from ROM
#define ZPM_RST         ZPM_RESET_BIT // 0x01
#define ZPM_TRAP        ZPM_TRAP_BIT
#define ZPM_TCLR        (ZPM_TRAP_BIT | ZPM_CLRTRAP_BIT)
#define ZPM_XLDR        ZPM_XREG_BIT // 0x04
#define ZPM_XLD         (ZPM_XREG_BIT | ZPM_XLOAD_BIT) // 0x05
#define ZPM_XST         (ZPM_XREG_BIT | ZPM_XSTORE_BIT) // 0x06
#define ZPM_XFER        (ZPM_XREG_BIT | ZPM_XSTORE_BIT | ZPM_XCPY_BIT) // 0x07
#define ZPM_WEV         ZPM_EVENT_BIT // 0x08
#define ZPM_SEV         (ZPM_EVENT_BIT | ZPM_SEND_BIT) // 0x09
#define ZPM_WTI         (ZPM_EVENT_BIT | ZPM_TRAP_BIT) // 0x0a
#define ZPM_SMI         (ZPM_EVENT_BIT | ZPM_TRAP_BIT | ZPM_SEND_BIT) // 0x0b
#define ZPM_CLI         (ZPM_EVENT_BIT | ZPM_INTR_BIT) // 0x0c
#define ZPM_STI         (ZPM_EVENT_BIT | ZPM_INTR_BIT | ZPM_INTRON_BIT) // 0x0d
#define ZPM_SCALL       (ZPM_SYSRTN_MASK) // 0x0e
#define ZPM_SRET        (ZPM_SYSRTN_MASK | ZPM_SYSRET_BIT) // 0x0f

/* memory-management unit */
#define ZPM_MEM_UNIT    0x0b
#define ZPM_NOCACHE_BIT 0x01
#define ZPM_WRITE_BIT   0x01
#define ZPM_INVAL_BIT   0x01
#define ZPM_WRPROT_BIT  0x02
#define ZPM_WRCOMB_BIT  0x02
#define ZPM_WRTHRU_BIT  0x02
#define ZPM_CACHE_BIT   0x04
#define ZPM_PAGE_BIT    0x08
#define ZPM_PGREL_BIT   0x01
#define ZPM_PGWIRE_BIT  0x02
#define ZPM_PGPRES_BIT  0x04
#define ZPM_PGSET_MASK  0x0e
#define ZPM_PGZERO_BIT  0x01
/* NOTE: 0x00 reserved */
#define ZPM_PGINIT      0x00
#define ZPM_PGNOC       (ZPM_NOCACHE_BIT) // 0x01
#define ZPM_PGWP        (ZPM_WRPROT_BIT) // 0x02
#define ZPM_PGWRC       (ZPM_WRCOMB_BIT | ZPM_WRITE_BIT) // 0x03
#define ZPM_PGWB        ZPM_CACHE_BIT // 0x04
#define ZPM_LKCL        (ZPM_CACHE_BIT | ZPM_CLK_BIT) // 0x05
#define ZPM_RELCL       (ZPM_CACHE_BIT | ZPM_REL_BIT) // 0x06
#define ZPM_PGWRT       (ZPM_CACHE | ZPM_WRTHRU_BIT | ZPM_WRITE_BIT) // 0x07
#define ZPM_PGMAP       (ZPM_PAGE_BIT) // 0x08
#define ZPM_PGUMAP      (ZPM_PAGE_BIT | ZPM_PGREL_BIT) // 0x09
#define ZPM_PGWIRE      (ZPM_PAGE_BIT | ZPM_PGWIRE_BIT) // 0x0a
#define ZPM_PGUNW       (ZPM_PAGE_BIT | ZPM_PGWIRE_BIT | ZPM_PGREL_BIT) // 0x0b
#define ZPM_PGPRES      (ZPM_PAGE_BIT | ZPM_PGPRES_BIT) // 0x0c present
#define ZPM_PGINV       (ZPM_PAGE_BIT | ZPM_PGPRES_BIT | ZPM_INVAL_BIT) // 0x0d
#define ZPM_PGSET       ZPM_PGSET_MASK
#define ZPM_PGCLR       (ZPM_PGSET_MASK | ZPM_PGZERO_BIT)
// NOTE: 0x0e, 0x0f reserved

/* task management; multitasking */
#define ZPM_TASK_UNIT   0x0c
#define ZPM_TCONFIG_BIT 0x01
#define ZPM_TCTX_BIT    0x02
#define ZPM_TSAVE_BIT   0x01
#define ZPM_TINIT       0x00
#define ZPM_TCONF       ZPM_TCONFIG_BIT
#define ZPM_TCTX        ZPM_TCTX_BIT
#define ZPM_TSAVE       (ZPM_TCTX_BIT | ZPM_TSAVE_BIT)

/* processor parameters */
#define ZPM_COPROC_UNIT 0x0f // coprocessor; FPU, VEC, ...
#define ZPM_NALU_MAX    256 // max number of ALU operations

/* VIRTUAL MACHINE */

#define zpmclrmsw(vm)   ((vm)->sysregs[ZPM_MSW] = 0)
#define zpmsetcf(vm)    ((vm)->sysregs[ZPM_MSW] |= ZPM_MSW_CF)
#define zpmsetzf(vm)    ((vm)->sysregs[ZPM_MSW] |= ZPM_MSW_ZF)
#define zpmsetof(vm)    ((vm)->sysregs[ZPM_MSW] |= ZPM_MSW_OF)
#define zpmsetif(vm)    ((vm)->sysregs[ZPM_MSW] |= ZPM_MSW_IF)
#define zpmcfset(vm)    ((vm)->sysregs[ZPM_MSW] & ZPM_MSW_CF)
#define zpmzfset(vm)    ((vm)->sysregs[ZPM_MSW] & ZPM_MSW_ZF)
#define zpmofset(vm)    ((vm)->sysregs[ZPM_MSW] & ZPM_MSW_OF)
#define zpmifset(vm)    ((vm)->sysregs[ZPM_MSW] & ZPM_MSW_IF)

#define ZPM_NGENREG     16 // number of registers in group (general, system)

/* accumulator (general-purpose register) IDs */
#define ZPM_REG0        0x00
#define ZPM_REG1        0x01
#define ZPM_REG2        0x02
#define ZPM_REG3        0x03
#define ZPM_REG4        0x04
#define ZPM_REG5        0x05
#define ZPM_REG6        0x06
#define ZPM_REG7        0x07
#define ZPM_REG8        0x08
#define ZPM_REG9        0x09
#define ZPM_REG10       0x0a
#define ZPM_REG11       0x0b
#define ZPM_REG12       0x0c
#define ZPM_REG13       0x0d
#define ZPM_REG14       0x0e
#define ZPM_REG15       0x0f
/* system register IDs */
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define ZPM_RET_LO      0x00 // [dual-word] return value register, low word
#define ZPM_RET_HI      0x01 // [dual-word] return value register, high word
#else
#define ZPM_RET_HI      0x00 // [dual-word] return value register, high word
#define ZPM_RET_LO      0x01 // [dual-word] return value register, low word
#endif
#define ZPM_MSW         0x00 // machine status word
#define ZPM_PC          0x01 // program counter i.e. instruction pointer
#define ZPM_FP          0x02 // frame pointer
#define ZPM_SP          0x03 // stack pointer
#define ZPM_PDB         0x04 // page directory base address register
#define ZPM_TLB         0x05 // thread-local storage base address register
/* values for sysregs[ZPM_MSW] */
#define ZPM_MSW_CF      (1 << 0) // carry-flag
#define ZPM_MSW_ZF      (1 << 1) // zero-flag
#define ZPM_MSW_OF      (1 << 2) // overflow-flag
#define ZPM_MWS_IF      (1 << 3) // interrupts pending
#define ZPM_MSW_SF      (1 << 30) // system-mode
#define ZPM_MSW_LF      (1 << 31) // bus lock flag
/* program segments */
#define ZPM_TEXT        0x00 // code
#define ZPM_RODATA      0x01 // read-only data (string literals etc.)
#define ZPM_DATA        0x02 // read-write (initialised) data
#define ZPM_BSS         0x03 // uninitialised (zeroed) runtime-allocated data
#define ZPM_TLS         0x04 // thread-local storage
#define ZPM_NSEG        0x08
struct zpm {
    zpmreg   genregs[ZPM_NREG];
    zpmureg  sysregs[ZPM_NREG];
    zpmureg *segs[ZPM_NSEG];
    zpmureg  seglims[ZPM_NSEG];
    uint8_t *mem;
};

/* OPCODES */

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define zpmmkopid(unit, inst) ((uint8_t)((unit) | ((inst) << 4)))
#else
#define zpmmkopid(unit, inst) ((uint8_t)(((unit) << 4) | (inst)))
#endif

/* argument type flags */
#define ZPM_INDIR_BIT   0x04 // indirect address
#define ZPM_INDEX_BIT   0x02 // index for address
#define ZPM_IMMED_BIT   0x01 // immediate argument
#define ZPM_REG_ARG     0x00 // register
#define ZPM_ARGT_BITS   3

/*
 * special uses for imm8
 * - shift or rotation count
 * - flags
 */
#define ZPM_CL_LOCK_BIT  0x40
#define ZPM_BUS_LOCK_BIT 0x80 // lock bus for operation

struct zpmop {
    unsigned int unit  : 4;     // execution unit
    unsigned int inst  : 4;     // per-unit instruction
    unsigned int reg1  : 4;     // argument #1 register ID
    unsigned int reg2  : 4;     // argument #2 register ID
    unsigned int arg1t : 3;     // argument #1 type
    unsigned int arg2t : 3;     // argument #2 type
    unsigned int argsz : 2;     // argument size is 8 << argsz bits
    uint8_t      imm8;          // immediate argument or flags
    zpmreg       imm[EMPTY];    // possible immediate argument
};

/* predefined I/O ports */
#define ZPM_STDIN_PORT  0       // keyboard input
#define ZPM_STDOUT_PORT 1       // console or framebuffer output
#define ZPM_STDERR_PORT 2       // console or framebuffer output
#define ZPM_MOUSE_PORT  3       // mouse input
#define ZPM_RTC_PORT    4       // real-time clock
#define ZPM_TMR_PORT    5       // timer interrupt configuration

/* framebuffer graphics interface */
#define ZPM_FB_BASE     (3UL * 1024 * 1024 * 1024)      // base address

/* operation function prototypes */

#endif /* __ZPM_ZPM_H__ */

