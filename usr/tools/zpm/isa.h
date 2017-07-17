#ifndef __ZPM_ISA_H__
#define __ZPM_ISA_H__

/* INSTRUCTION SET ARCHITECTURE */

/* ALU (arithmetic-logical unit) instructions */

/*
 * NOT %r - ~arg - N/A - bitwise negation
 * AND %r1, %r2 - arg2 &= arg1 - N/A - bitwise conjunction
 * OR %r1, %r2 - arg2 |= arg1 - N/A - bitwise disjunction
 * XOR %r1, %r2 - arg2 ^= arg1 - N/A - bitwise exclusive disjunction
 * CLZ %r1 - clz(arg1) - N/A - count high zero-bits
 * CTZ %r1 - ctz(arg1) - N/A - count low zero-bits
 * BPOP %r1 - bitpop(arg1) - N/A - count 1-bits
 */

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

/*
 * SHL %r1/i, %r2 - arg2 <<= arg2 - N/A - shift left
 * SHR %r1/i, %r2 - arg2 >>= arg2 - N/A - shift right logical
 * SAR %r1/i, %r2 - arg2 >>>= arg2 - N/A - shift right arithmetic
 * ROL %r1/i, %r2 - arg2 <<= arg2 - N/A - shift left
 * ROR %r1/i, %r2 - arg2 <<= arg2 - N/A - shift left

/* shifter */
#define ZPM_SHIFT_UNIT  0x01
#define ZPM_SHRIGHT_BIT 0x01
#define ZPM_SHSIGN_BIT  0x02
#define ZPM_ROT_BIT     0x04
#define ZPM_RCARRY_BIT  0x02
#define ZPM_SHFIELD_BIT 0x08
#define ZPM_BFROT_MASK  (ZPM_SHFIELD_BIT | ZPM_ROT_BIT)
#define ZPM_SHL         0x00
#define ZPM_SHR         ZPM_SHRIGHT_BIT // 0x01
#define ZPM_SAR         ZPM_SHSIGN_BIT // 0x02
#define ZPM_SHLDR       0x03 // load shift [count] register
#define ZPM_ROL         ZPM_ROT_BIT // 0x04
#define ZPM_ROR         (ZPM_ROT_BIT | ZPM_SHRIGHT_BIT) // 0x05
#define ZPM_RCL         (ZPM_ROT_BIT | ZPM_RCARRY_BIT) // 0x06
#define ZPM_RCR         (ZPM_ROT_BIT | ZPM_RCARRY_BIT | ZPM_SHRIGHT_BIT) // 0x07
#define ZPM_BFSHL       ZPM_SHFIELD_BIT // 0x08
#define ZPM_BFSHR       (ZPM_SHFIELD_BIT | ZPM_SHRIGHT_BIT) // 0x09
#define ZPM_BFSAR       (ZPM_SHFIELD_BIT | ZPM_SHSIGN_BIT) // 0x0a
#define ZPM_BFROL       ZPM_BFROT_MASK // 0x0c
#define ZPM_BFROR       (ZPM_BFROT_MASK | ZPM_SHRIGHT_BIT) // 0d
#define ZPM_BFRCL       (ZPM_BFROT_MASK | ZPM_CARRY_BIT) // 0x0e
#define ZPM_BFRCR       (ZPM_BFROT_MASK | ZPM_CARRY_BIT | ZPM_RIGHT_BIT) // 0x0e

/* arithmetic operations */
/* arithmetic unit */
#define ZPM_ARITH_UNIT  0x02
#define ZPM_MSW_BIT     0x01
#define ZPM_DEC_BIT     0x01
#define ZPM_INC_BIT     0x02
#define ZPM_ADD_BIT     0x02
#define ZPM_SUB_BIT     0x04
#define ZPM_ADDINV_BIT  0x04
#define ZPM_INC         0x00
#define ZPM_DEC         ZPM_DEC_BIT // 0x01
#define ZPM_ADD         ZPM_ADD_BIT // 0x02
#define ZPM_ADF         (ZPM_ADD_BIT | ZPM_MSW_BIT) // 0x03
#define ZPM_ADI         (ZPM_ADD_INV_BIT) // // 0x04 a + ~b
#define ZPM_SUB         (ZPM_ADD_INV_BIT | ZPM_INC_BIT) // 0x06 a + ~b + 1
#define ZPM_CMP         (ZPM_ADD_INV_BIT | ZPM_INC_BIT | ZPM_MSW_BIT) // 0x07

/* multiplier */
#define ZPM_MUL_UNIT    0x03
#define ZPM_MWIDTH_BIT  0x01
#define ZPM_MHIGH_BIT   0x02
#define ZPM_MDUAL_BIT   0x02
#define ZPM_MSIGN_BIT   0x04
#define ZPM_MFUSE_BIT   0x08
#define ZPM_MTRUNC_BIT  0x01
#define ZPM_MLIMIT_BIT  0x02
#define ZPM_MLOW_BIT    0x01
#define ZPM_MSHIFT_BIT  0x04
#define ZPM_MRIGHT_BIT  0x01
#define ZPM_MFSIGN_BIT  0x02
#define ZPM_MSHIFT_MASK (ZPM_MFUSE_BIT | ZPM_MSHIFT_BIT)
#define ZPM_MUL         0x00 // multiplication
#define ZPM_MULW        ZPM_MWIDTH_BIT // 0x01 both words of result in %r0:%r1
#define ZPM_MULHI       ZPM_MHIGH_BIT // 0x02 return high-word of result
#define ZPM_MUL2        (ZPM_MDUAL_BIT | ZPM_MWIDTH_BIT) // 0x03
#define ZPM_MULS        ZPM_MSIGN_BIT // 0x04
#define ZPM_MULWS       (ZPM_MSIGN_BIT | ZPM_MWIDTH_BIT) // 0x05
#define ZPM_MULHIS      (ZPM_MSIGN_BIT | ZPM_MHIGH_BIT) // 0x06
#define ZPM_MUL2S       (ZPM_MSIGN_BIT | ZPM_MDUAL_BIT | ZPM_MWIDTH_BIT) // 0x07
#define ZPM_MAC         ZPM_MFUSE_BIT // 0x08 multiply-accumulate
#define ZPM_MAT         (ZPM_MFUSE_BIT | ZPM_MTRUNC_BIT) // 0x09 multiply-trunc
#define ZPM_MCEIL       (ZPM_FUSE_BIT | ZPM_MLIMIT_BIT) // 0x0a
#define ZPM_MFLOOR      (ZPM_FUSE_BIT | ZPM_MLIMIT_BIT | ZPM_MLOW_BIT) // 0x0b
#define ZPM_MSHL        ZPM_MSHIFT_MASK // 0x0c multiply-shift-left
#define ZPM_MSHR        (ZPM_MSHIFT_MASK | ZPM_MRIGHT_BIT) // 0x0d multiply-shl
#define ZPM_MSAR        (ZPM_MFUSE_MASK | ZPM_MSIGN_BIT) // 0x0e

/* divider */
#define ZPM_DIV_UNIT    0x04
#define ZPM_REM_BIT     0x01 // remainder-flag
#define ZPM_REC_BIT     0x02 // reciprocal
#define ZPM_RDIV_BIT    0x01
#define ZPM_ROUND_BIT   0x04
#define ZPM_RUP_BIT     0x01
#define ZPM_RDOWN_BIT   0x02
#define ZPM_RTRUNC_BIT  0x01
#define ZPM_DIV         0x00 // division, result in ZPM_RET_LO
#define ZPM_REM         ZPM_REM_BIT // 0x01 remainder of division in ZPM_RET_HI
#define ZPM_REC         ZPM_REC_BIT // 0x02 precompute reciprocal for division
#define ZPM_RDIV        (ZPM_REC_BIT | ZPM_RDIV_BIT) // 0x03
#define ZPM_ROUND       (ZPM_ROUND_BIT) // 0x04 exact rounding
#define ZPM_RUP         (ZPM_ROUND_BIT | ZPM_RUP_BIT) // 0x05 ceiling
#define ZPM_RDOWN       (ZPM_ROUND_BIT | ZPM_RDOWN_BIT) // 0x06 floor
#define ZPM_RTRUNC      (ZPM_ROUND_BIT | ZPM_RMODE_BIT | ZPM_RTRUNC_BIT) // 0x07

/* load-store and stack operations */
#define ZPM_XFER_UNIT   0x05
#define ZPM_LOAD_BIT    0x01
#define ZPM_STORE_BIT   0x02 // write operation
#define ZPM_STACK_BIT   0x04 // stack operation
#define ZPM_ALLREGS_BIT 0x02 // choose all general-purpose registers
#define ZPM_SCPY_BIT    0x08
#define ZPM_SLOAD_BIT   0x01
#define ZPM_SSTORE_BIT  0x02
#defein ZPM_SSTACK_MASK (ZPM_SCPY_BIT | ZPM_STACK_BIT)
#define ZPM_CPY         0x00
#define ZPM_LDR         (ZPM_LOAD_BIT) // 0x01
#define ZPM_STR         (ZPM_STORE_BIT) // 0x02
#define ZPM_PSH         ZPM_STACK_BIT // 0x04
#define ZPM_POP         (ZPM_STACK_BIT | ZPM_LOAD_BIT) // 0x05
#define ZPM_PSHA        (ZPM_STACK_BIT | ZPM_ALLREGS_BIT) // 0x06
#define ZPM_POPA        (ZPM_STACK_BIT | ZPM_ALLREGS_BIT | ZPM_LOAD_BIT) // 0x07
#define ZPM_SCPY        ZPM_SCPY_BIT // 0x08
#define ZPM_SLDR        (ZPM_SCPY_BIT | ZPM_SLOAD_BIT) // 0x09
#define ZPM_SCPY        (ZPM_SCPY_BIT | ZPM_SSTORE_BIT) // 0x0a
#define ZPM_SPSH        ZPM_SSTACK_MASK // 0x0c
#define ZPM_SPOP        (ZPM_SSTACK_MASK | ZPM_LOAD_BIT) // 0x0d
#define ZPM_SPSHA       (ZPM_STACK_MASK | ZPM_ALLREGS_BIT) // 0x0e
/* 0x0f */
#define ZPM_SPSHA       (ZPM_STACK_MASK | ZPM_ALLREGS_BIT | ZPM_LOAD_BIT)

/* I/O operations */
#define ZPM_IO_UNIT     0x06
#define ZPM_IOWRITE_BIT 0x01
#define ZPM_IO16_BIT    0x02
#define ZPM_IO32_BIT    0x04
#define ZPM_IO64_BIT    0x08
#define ZPM_IOPERM_BIT  0x04
#define ZPM_IOSET_BIT   0x08
#define ZPM_IOREAD_BIT  0x02
#define ZPM_IOR8        0x00
#define ZPM_IOW8        (ZPM_IOWRITE_BIT) // 0x01
#define ZPM_IOR16       (ZPM_IO16_BIT) // 0x02
#define ZPM_IOW16       (ZPM_IO16_BIT | ZPM_IOWRITE_BIT) // 0x03
#define ZPM_IOR32       (ZPM_IO32_BIT) // 0x04
#define ZPM_IOW32       (ZPM_IO32_BIT | ZPM_IOWRITE_BIT) // 0x05
#define ZPM_IOROCHK     (ZPM_IOPERM_BIT | ZPM_IOREAD_BIT) // 0x06
// 0x07
#define ZPM_IORWCHK     (ZPM_IOPERM_BIT | ZPM_IOREAD_BIT | ZPM_IOWRITE_BIT)
#define ZPM_IOR64       (ZPM_64_BIT) // 0x08
#define ZPM_IOW64       (ZPM_64_BIT | ZPM_IOWRITE_BIT) // 0x09
#define ZPM_IOSETRO     (ZPM_IOSET_BIT | ZPM_IOREAD_BIT) // 0x0a
// 0x0b
#define ZPM_IOSETRW     (ZPM_IOSET_BIT | ZPM_IOREAD_BIT | ZPM_IOWRITE_BIT)

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
#define ZPM_BZ          ZPM_ZERO_BIT // 0x01
#define ZPM_BNZ         ZPM_NOTZERO_BIT // 0x02
#define ZPM_CALL        0x03
#define ZPM_BO          (ZPM_OVER_BIT) // 0x04
#define ZPM_BNO         (ZPM_OVER_BIT | ZPM_ZERO_BIT) // 0x05
#define ZPM_BC          (ZPM_CARRY_BIT | ZPM_NOTZERO_BIT) // 0x06
#define ZPM_BNC         (ZPM_CARRY_BIT | ZPM_NOCARRY_BIT | ZPM_ZERO_BIT) // 0x07
#define ZPM_BLT         (ZPM_LESS_BIT) // 0x08
#define ZPM_BLE         (ZPM_LESS_BIT | ZPM_EQUAL_BIT) // 0x09
#define ZPM_BGT         (ZPM_GREATER_BIT | ZPM_UNEQUAL_BIT) // 0x0a
// 0x0b
#define ZPM_BGE         (ZPM_GREATER_BIT | ZPM_UNEQUAL_BIT | ZPM_EQUAL_BIT)
#define ZPM_TASK        0x0c // launch new process, thread, fiber
#define ZPM_RET         0x0d // return from subroutine
#define ZPM_TRET        0x0e // return from thread
#define ZPM_TEXIT       0x0f // exit task

/* system operations */
#define ZPM_SYS_UNIT    0x08
#define ZPM_RESET_BIT   0x01
#define ZPM_XCPY_BIT    0x04
#define ZPM_XCPY_BIT    0x01
#define ZPM_XLOAD_BIT   0x01
#define ZPM_XSTORE_BIT  0x02
#define ZPM_SEVENT_BIT  0x08
#define ZPM_SSEND_BIT   0x01
#define ZPM_SINTRON_BIT 0x01
#define ZPM_STRAP_BIT   0x02
#define ZPM_SINTR_BIT   0x04
#define ZPM_SYS_MASK    0x0e
#define ZPM_SYSRET_BIT  0x01
#define ZPM_SBOOT       0x00 // execute bootstrap code from ROM
#define ZPM_SRST        ZPM_RESET_BIT // 0x01
#define ZPM_STSET       ZPM_STRAP_BIT // 0x02
#define ZPM_STCLR       (ZPM_STRAP_BIT | ZPM_RESET_BIT) // 0x03
#define ZPM_XCPY        ZPM_XCPY_BIT // 0x04
#define ZPM_SLDR        (ZPM_XCPY_BIT | ZPM_XLOAD_BIT) // 0x05
#define ZPM_SSTR        (ZPM_XCPY_BIT | ZPM_XSTORE_BIT) // 0x06
#define ZPM_WFE         ZPM_SEVENT_BIT // 0x08
#define ZPM_SEV         (ZPM_SEVENT_BIT | ZPM_SSEND_BIT) // 0x09
#define ZPM_WFI         (ZPM_SEVENT_BIT | ZPM_STRAP_BIT) // 0x0a
#define ZPM_SMI         (ZPM_SEVENT_BIT | ZPM_STRAP_BIT | ZPM_SSEND_BIT) // 0x0b
#define ZPM_CLI         (ZPM_SEVENT_BIT | ZPM_SINTR_BIT) // 0x0c
// 0x0d
#define ZPM_STI         (ZPM_SEVENT_BIT | ZPM_SINTR_BIT | ZPM_SINTRON_BIT)
#define ZPM_SCALL       (ZPM_SYS_MASK) // 0x0e
#define ZPM_SRET        (ZPM_SYS_MASK | ZPM_SYSRET_BIT) // 0x0f

/* OPTIONAL UNITS */

/* bitfield operations */
// NOTE: use width == 0, pos == 0 to indicate operations on whole registers
#define ZPM_BITFLD_UNIT 0x09
/* shift and rotate operations */
#define ZPM_BFRIGHT_BIT 0x01
#define ZPM_BFONE_BIT   0x01
#define ZPM_BFSIGN_BIT  0x02
#define ZPM_BFZERO_BIT  0x02
#define ZPM_BFCOUNT_BIT 0x04
#define ZPM_BFTRAIL_BIT 0x01
#define ZPM_BFPOP_BIT   0x02
#define ZPM_BFCPY_BIT   0x08
#define ZPM_BFLOAD_BIT  0x01
/* instructions */
#define ZPM_BFNOT       0x00
#define ZPM_BFAND       0x01
#define ZPM_BFOR        0x02
#define ZPM_BFXOR       0x03
#define ZPM_BFCLZ       ZPM_BFCOUNT_BIT // 0x04
#define ZPM_BFCTZ       (ZPM_BFCOUNT_BIT | ZPM_BFTRAIL_BIT) // 0x05
#define ZPM_BFPOP       (ZPM_BFCOUNT_BIT | ZPM_BFPOP_BIT) // 0x06
#define ZPM_BFSET       0x07
#define ZPM_BFCPY       ZPM_BFCPY_BIT // 0x08
#define ZPM_BFLDR_BIT   (ZPM_BFCPY_BIT | ZPM_BFLOAD_BIT) // 0x09
#define ZPM_BFSTR_BIT   (ZPM_BFCPY_BIT | ZPM_BFSTORE_BIT) // 0x0a

/* memory-management unit */
#define ZPM_MEM_UNIT    0x0a
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
#define ZPM_TLS_MASK    0x0e
#define ZPM_TWRITE_BIT  0x01
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
#define ZPM_TLDR        (ZPM_TLS_MASK) // 0x0e
#define ZPM_TSTR        (ZPM_TLS_MASK | ZPM_TWRITE_BIT) // 0x0f

/* task management; multitasking */
#define ZPM_TASK_UNIT   0x0b
#define ZPM_TCONFIG_BIT 0x01
#define ZPM_TCTX_BIT    0x02
#define ZPM_TSAVE_BIT   0x01
#define ZPM_TPROF_BIT   0x04
#define ZPM_TPINFO_BIT  0x01
#define ZPM_TPSAVE_BIT  0x01
#define ZPM_TINIT       0x00
#define ZPM_TCTL_BIT    0x08
#define ZPM_TSYS_BIT    0x01
#define ZPM_TINTR_BIT   0x02
#define ZPM_TID_BIT     0x04
#define ZPM_TEVENT_BIT  0x04
#define ZPM_TSEND_BIT   0x01
#define ZPM_TRECV_BIT   0x02
#define ZPM_TCONF       ZPM_TCONFIG_BIT // 0x01
#define ZPM_TCTX        ZPM_TCTX_BIT // 0x02
#define ZPM_TSAVE       (ZPM_TCTX_BIT | ZPM_TSAVE_BIT) // 0x03
#define ZPM_TPROF       ZPM_TPROF_BIT // 0x04
#define ZPM_TPREAD      (ZPM_TPROF_BIT | ZPM_TPINFO_BIT) // 0x05
#define ZPM_TPSTOP      (ZPM_TPROF_BIT | ZPM_TPSTOP_BIT) // 0x06
// 0x07
#define ZPM_TPHOLD      (ZPM_TPROF_BIT | ZPM_TPSTOP_BIT | ZPM_TPSAVE_BIT)
#define ZPM_TYIELD      ZPM_TCTL_BIT // 0x08
#define ZPM_TSRING      (ZPM_TCTL_BIT | ZPM_TSYS_BIT) // 0x09
#define ZPM_TINTR       (ZPM_TCTL_BIT | ZPM_TINTR_BIT) // 0x0a
#define ZPM_TSTOP       (ZPM_TCTL_BIT | ZPM_TID_BIT | ZPM_TSTOP_BIT) // 0x0b
#define ZPM_TSIG        (ZPM_TCTL_BIT | ZPM_TEVENT_BIT) // 0x0c
#define ZPM_TSEND       (ZPM_TCTL_BIT | ZPM_TEVENT_BIT | ZPM_TSEND_BIT) // 0x0d
#define ZPM_TRECV       (ZPM_TCTL_BIT | ZPM_TEVENT_BIT | ZPM_TSEND_BIT) // 0x0e
#define ZPM_IRET        0x0f // return from interrupt handler

/* inter-processor atomic operations (bus locked) */
#define ZPM_ATOM_UNIT   0x0c
#define ZPM_BSET_BIT    0x01
#define ZPM_BFLIP_BIT   0x02
#define ZPM_LDLINK_BIT  0x02
#define ZPM_CSTORE_BIT  0x01
#define ZPM_FADD_BIT    0x04
#define ZPM_FSUB_BIT    0x01
#define ZPM_FINC_BIT    0x02
#define ZPM_FDEC_BIT    0x01
#define ZPM_FETCH_BIT   0x08
#define ZPM_COMPARE_BIT 0x02
#define ZPM_CSWAP_BIT   0x04
#define ZPM_CSWAP2_BIT  0x01
#define ZPM_BCLR        0x00
#define ZPM_BSET        ZPM_BSET_BIT // 0x01
#define ZPM_BCHG        ZPM_BFLIP_BIT // 0x02
#define ZPM_LLSC        (ZPM_LDLINK_BIT | ZPM_CSTORE_BIT) // 0x03
#define ZPM_FADD        ZPM_FADD_BIT // 0x04
#define ZPM_FINC        (ZPM_FADD_BIT | ZPM_FSUB_BIT) // 0x05
#define ZPM_FDEC        (ZPM_FADD_BIT | ZPM_FINC_BIT) // 0x06
#define ZPM_FDEC        (ZPM_FADD_BIT | ZPM_FDEC_BIT) // 0x07
#define ZPM_BTAC        ZPM_FETCH_BIT // 0x08
#define ZPM_BTAS        (ZPM_FETCH_BIT | ZPM_BSET_BIT) // 0x09
#define ZPM_BCCLR       (ZPM_FETCH_BIT | ZPM_COMPARE_BIT) // 0x0a
#define ZPM_BCSET       (ZPM_FETCH_BIT | ZPM_COMPARE_BIT | ZPM_BSET_BIT) // 0x0b
#define ZPM_CAS         (ZPM_FETCH_BIT | ZPM_SWAP_BIT) // 0x0c
#define ZPM_CAS2        (ZPM_FETCH_BIT | ZPM_SWAP_BIT | ZPM_CSWAP2_BIT) // 0x0d

/* processor parameters */
#define ZPM_COPROC_UNIT 0x0f // coprocessor; FPU, VEC, ...
#define ZPM_NALU_MAX    256 // max number of ALU operations

#endif /* __ZPM_ISA_H__ */

