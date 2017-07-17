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
#define ZPM_BITS_UNIT    0x00
/* optional */
#define ZPM_BITCNT_BIT   0x04
#define ZPM_TRAIL_BIT    0x01
#define ZPM_BITPOP_BIT   0x02
/* instructions */
#define ZPM_NOT          0x00 // 2's complement (reverse all bits)
#define ZPM_AND          0x01 // logical bitwise AND
#define ZPM_OR           0x02 // logical bitwise OR
#define ZPM_XOR          0x03 // logical bitwise XOR (exclusive OR)
/* optional operations; HAVE_CNT_BITS */
#define ZPM_CLZ          ZPM_BITCNT_BIT // 0x04 count leading zero bits
#define ZPM_CTZ          (ZPM_BITCNT_BIT | ZPM_TRAIL_BIT) // 0x05 trailing zeroes
#define ZPM_BPOP         (ZPM_BITCNT_BIT | ZPM_BITPOP_BIT) // 0x06 count 1-bits

/*
 * SHL %r1/i, %r2 - arg2 <<= arg2 - N/A - shift left
 * SHR %r1/i, %r2 - arg2 >>= arg2 - N/A - shift right logical
 * SAR %r1/i, %r2 - arg2 >>>= arg2 - N/A - shift right arithmetic
 * ROL %r1/i, %r2 - arg2 <<= arg2 - N/A - shift left
 * ROR %r1/i, %r2 - arg2 <<= arg2 - N/A - shift left

/* shifter */
#define ZPM_SHIFT_UNIT   0x00
#define ZPM_RIGHT_BIT    0x01
#define ZPM_SIGNEXT_BIT  0x02
#define ZPM_ROT_BIT      0x04
#define ZPM_RCARRY_BIT   0x02
/* optional */
#define ZPM_FIELD_BIT    0x08
#define ZPM_FROT_BITS    (ZPM_FIELD_BIT | ZPM_ROT_BIT)
/* instructions */
#define ZPM_SHL          0x00
#define ZPM_SHR          ZPM_RIGHT_BIT // 0x01
#define ZPM_SAR          ZPM_SIGNEXT_BIT // 0x02
// NOTE: 0x03 is reserved
#define ZPM_ROL          ZPM_ROT_BIT // 0x04
#define ZPM_ROR          (ZPM_ROT_BIT | ZPM_RIGHT_BIT) // 0x05
#define ZPM_RCL          (ZPM_ROT_BIT | ZPM_RCARRY_BIT) // 0x06
#define ZPM_RCR          (ZPM_ROT_BIT | ZPM_RCARRY_BIT | ZPM_RIGHT_BIT) // 0x07
/* optional operations */
#define ZPM_BFSHL        ZPM_FIELD_BIT // 0x08
#define ZPM_BFSHR        (ZPM_FIELD_BIT | ZPM_RIGHT_BIT) // 0x09
#define ZPM_BFSAR        (ZPM_FIELD_BIT | ZPM_SIGNEXT_BIT) // 0x0a
#define ZPM_BFROL        ZPM_FROT_BITS // 0x0c
#define ZPM_BFROR        (ZPM_FROT_BITS | ZPM_RIGHT_BIT) // 0d
#define ZPM_BFRCL        (ZPM_FROT_BITS | ZPM_RCARRY_BIT) // 0x0e
#define ZPM_BFRCR        (ZPM_FROT_BITS | ZPM_RCARRY_BIT | ZPM_RIGHT_BIT) // 0x0e

/* arithmetic operations */
/* arithmetic unit */
#define ZPM_ARITH_UNIT   0x01
#define ZPM_DEC_BIT      0x01
#define ZPM_ADD_BIT      0x02
#define ZPM_MSW_BIT      0x01
#define ZPM_ADDINV_BIT   0x04
#define ZPM_SUB_BITS     0x05
#define ZPM_CMP_BITS     0x06
/* instructions */
#define ZPM_INC          0x00
#define ZPM_DEC          ZPM_DEC_BIT // 0x01
#define ZPM_ADD          ZPM_ADD_BIT // 0x02
#define ZPM_ADF          (ZPM_ADD_BIT | ZPM_MSW_BIT) // 0x03
#define ZPM_ADI          (ZPM_ADDINV_BIT) // // 0x04 a + ~b
#define ZPM_SUB          ZPM_SUB_BITS // 0x05 a + ~b + 1
#define ZPM_CMP          ZPM_CMP_BITS

/* flow control; branch and subroutine operations */
#define ZPM_FLOW_UNIT    0x02
#define ZPM_ZERO_BIT     0x01
#define ZPM_NOTZERO_BIT  0x02
#define ZPM_CALL_BITS    0x03
#define ZPM_OVERFLOW_BIT 0x04
#define ZPM_CARRY_BITS   0x06
#define ZPM_LESS_BIT     0x08
#define ZPM_GREATER_BITS 0x0a
#define ZPM_RET_BITS     0x0c
#define ZPM_TASK_BIT     0x01
#define ZPM_TASK_BITS    0x0e
#define ZPM_EXIT_BIT     0x01
/* instructions */
#define ZPM_JMP          0x04
#define ZPM_BZ           ZPM_ZERO_BIT // 0x01
#define ZPM_BNZ          ZPM_NOTZERO_BIT // 0x02
#define ZPM_CALL         ZPM_CALL_BITS
#define ZPM_BO           (ZPM_OVERFLOW_BIT) // 0x04
#define ZPM_BNO          (ZPM_OVERFLOW_BIT | ZPM_ZERO_BIT) // 0x05
#define ZPM_BC           ZPM_CARRY_BITS
#define ZPM_BNC          (ZPM_CARRY_BITS | ZPM_ZERO_BIT) // 0x07
#define ZPM_BLT          ZPM_LESS_BIT // 0x08
#define ZPM_BLE          (ZPM_LESS_BIT | ZPM_ZERO_BIT) // 0x09
#define ZPM_BGT          ZPM_GREATER_BITS // 0x0a
#define ZPM_BGE          (ZPM_GREATER_BITS | ZPM_ZERO_BIT) // 0x0b
#define ZPM_RET          ZPM_RET_BITS
#define ZPM_TRET         (ZPM_RET_BITS | ZPM_TASK_BIT)
/* optional operations */
#define ZPM_TASK         ZPM_TASK_BITS // launch new process, thread, fiber
#define ZPM_TEXIT        (ZPM_TASK_BITS | ZPM_EXIT_BIT) // exit task

/* multiplier */
#define ZPM_MUL_UNIT     0x03
#define ZPM_SIGN_BIT     0x01
#define ZPM_WIDTH_BIT    0x02
#define ZPM_HIGH_BIT     0x04
#define ZPM_LIMIT_BITS   0x06
#define ZPM_CEIL_BIT     0x01
#define ZPM_FLOOR_BIT    0x02
#define ZPM_ExTRA_BIT    0x08
#define ZPM_MUL          0x00
#define ZPM_MULS         ZPM_MSIGN_BIT // 0x01
#define ZPM_MULW         ZPM_MWIDTH_BIT // 0x02
#define ZPM_MULSW        (ZPM_MWIDTH_BIT | ZPM_MSIGN_BIT) // 0x03
#define ZPM_MULHI        ZPM_MHIGH_BIT // 0x04
#define ZPM_MULSHI       (ZPM_MHIGH_BIT | ZPM_MSIGN_BIT) // 0x05
#define ZPM_MFLOOR       ZPM_MLIMIT_BITS // 0x06
#define ZPM_MCEIL        (ZPM_MLIMIT_BITS | ZPM_MCEIL_BIT) // 0x07
/* optional instructions */
#define ZPM_MAC          ZPM_EXTRA_BIT // multiply-accumulate

/* divider */
#define ZPM_DIV_UNIT     0x04
#define ZPM_REM_BIT      0x01 // remainder-flag
/* optional */
#define ZPM_ROUND_BIT    0x02
#define ZPM_TRUNC_BIT    0x01
#define ZPM_REC_BIT      0x04 // reciprocal division
#define ZPM_CREC_BIT     0x01 // calculate reciprocal
/* instructions */
#define ZPM_DIV          0x00 // division, result in ZPM_RET_LO
#define ZPM_REM          ZPM_REM_BIT // 0x01 remainder of division in ZPM_RET_HI
/* optional operations */
#define ZPM_SETRND       ZPM_ROUND_BIT // 0x02
#define ZPM_SETRNC       (ZPM_ROUND_BIT | ZPM_TRUNC_BIT) // 0x03
#define ZPM_DREC         ZPM_REC_BIT // 0x04 reciprocal division
#define ZPM_DCREC        (ZPM_REC_BIT | ZPM_CREC_BIT) // 0x05

/* load-store operations */
#define ZPM_XFER_UNIT    0x05
#define ZPM_LOAD_BIT     0x01
#define ZPM_STORE_BIT    0x02 // write operation
#define ZPM_SCPY_BIT     0x04
#define ZPM_XCPY_BIT     0x08
/* instructions */
#define ZPM_CPY          0x00
#define ZPM_LDR          (ZPM_LOAD_BIT) // 0x01
#define ZPM_STR          (ZPM_STORE_BIT) // 0x02
#define ZPM_SCPY         ZPM_SCPY_BIT // 0x04
#define ZPM_SLDR         (ZPM_SCPY_BIT | ZPM_LOAD_BIT) // 0x05
#define ZPM_SSTR         (ZPM_SCPY_BIT | ZPM_STORE_BIT) // 0x06
#define ZPM_XCPY         ZPM_XCPY_BIT // 0x08
#define ZPM_XLDR         (ZPM_XCPY_BIT | ZPM_LOAD_BIT) // 0x09
#define ZPM_XSTR         (ZPM_XCPY_BIT | ZPM_STORE_BIT) // 0x0a

/* stack operations */
#define ZPM_STACK_UNIT   0x06
#define ZPM_POP_BIT      0x01
#define ZPM_ALLREGS_BIT  0x02
#define ZPM_SREG_BIT     0x04
#define ZPM_XREG_BIT     0x08
#define ZPM_STK_BITS     0x0c
#define ZPM_SYSSTK_BIT   0x01
#define ZPM_STKSIZE_BITS 0x0e
#define ZPM_PSH          0x00
#define ZPM_POP          ZPM_POP_BIT // 0x01
#define ZPM_PSHA         ZPM_ALLREGS_BIT // 0x02
#define ZPM_POPA         (ZPM_ALLREGS_BIT | ZPM_POP_BIT) // 0x03
#define ZPM_SPSH         ZPM_SREG_BIT // 0x04
#define ZPM_SPOP         (ZPM_SREG_BIT | ZPM_POP_BIT) // 0x05
#define ZPM_SPSHA        (ZPM_SREG_BIT | ZPM_ALLREGS_BIT) // 0x06
#define ZPM_SPOPA        (ZPM_SREG_BIT | ZPM_ALLREGS_BIT | ZPM_POP_BIT) // 0x07
#define ZPM_XPSH         ZPM_XREG_BIT // 0x08
#define ZPM_XPOP         (ZPM_XREG_BIT | ZPM_POP_BIT) // 0x09
#define ZPM_XPSHA        (ZPM_XREG_BIT | XPM_ALLREGS_BIT) // 0x0a
#define ZPM_XPOPA        (ZPM_XREG_BIT | XPM_ALLREGS_BIT | ZPM_POP_BIT) // 0x0b
/* optional operations; HAVE_SET_STK */
#define ZPM_STK          ZPM_STK_BITS // 0x0c
#define ZPM_SSTK         (ZPM_STK_BITS | ZPM_SYSSTK_BITS) // 0x0d
#define ZPM_STKSZ        ZPM_STKSIZE_BITS // 0x0e
#define ZPM_SSTKSZ       (ZPM_STKSIZE_BITS | ZPM_SYSSTK_BIT) // 0x0f

/* I/O operations */
#define ZPM_IO_UNIT      0x07
#define ZPM_IOWRITE_BIT  0x01
/* optional */
#define ZPM_IOPCHK_BIT   0x02
#define ZPM_IOPSET_BIT   0x04
/* instructions */
#define ZPM_IOR          0x00
#define ZPM_IOW          (ZPM_IOWRITE_BIT) // 0x01
/* optional instructions */
#define ZPM_IORCHK       ZPM_IOPCHK_BIT
#define ZPM_IORWCHK      (ZPM_IOPCHK_BIT | ZPM_IOWRITE_BIT)
#define ZPM_IOPRSET      ZPM_IOPSET_BITS
#define ZPM_IOPRWSET     (ZPM_IOPSET_BITS | ZPM_IOWRITE_BIT)

/* system operations */
#define ZPM_SYS_UNIT     0x08
#define ZPM_RESET_BIT    0x01
#define ZPM_TRAP_BIT     0x02
#define ZPM_EVENT_BIT    0x04
#define ZPM_SEND_BIT     0x01
#define ZPM_ALLOW_BIT    0x01
#define ZPM_INTR_BIT     0x08
#define ZPM_SYS_BITS     0x0a
#define ZPM_SYSRET_BIT   0x01
/* instructions */
#define ZPM_BOOT         0x00 // execute bootstrap code from ROM
#define ZPM_RST          ZPM_RESET_BIT // 0x01
#define ZPM_TSET         ZPM_TRAP_BIT // 0x02
#define ZPM_TCLR         (ZPM_TRAP_BIT | ZPM_RESET_BIT) // 0x03
#define ZPM_WFE          ZPM_EVENT_BIT // 0x04
#define ZPM_EV           (ZPM_EVENT_BIT | ZPM_SEND_BIT) // 0x05
#define ZPM_WFI          (ZPM_EVENT_BIT | ZPM_TRAP_BIT) // 0x06
#define ZPM_SMI          (ZPM_EVENT_BIT | ZPM_TRAP_BIT | ZPM_SEND_BIT) // 0x07
#define ZPM_CLI          ZPM_INTR_BIT // 0x08
#define ZPM_STI          (ZPM_INTR_BIT | ZPM_ALLOW_BIT) // 0x09
#define ZPM_SCALL        (ZPM_SYS_BITS) // 0x0a
#define ZPM_SRET         (ZPM_SYS_BITS | ZPM_SYSRET_BIT) // 0x0b

/* OPTIONAL UNITS */

/* bitfield operations */
// NOTE: use width == 0, pos == 0 to indicate operations on whole registers
#define ZPM_BITFLD_UNIT  0x09
/* shift and rotate operations */
#define ZPM_BFRIGHT_BIT  0x01
#define ZPM_BFCOUNT_BIT  0x04
#define ZPM_BFTRAIL_BIT  0x01
#define ZPM_BFPOP_BIT    0x02
#define ZPM_BFCPY_BIT    0x08
#define ZPM_BFLOAD_BIT   0x01
#define ZPM_BFSTORE_BIT  0x02
/* instructions */
#define ZPM_BFNOT        0x00
#define ZPM_BFAND        0x01
#define ZPM_BFOR         0x02
#define ZPM_BFXOR        0x03
#define ZPM_BFCLZ        ZPM_BFCOUNT_BIT // 0x04
#define ZPM_BFCTZ        (ZPM_BFCOUNT_BIT | ZPM_BFTRAIL_BIT) // 0x05
#define ZPM_BFPOP        (ZPM_BFCOUNT_BIT | ZPM_BFPOP_BIT) // 0x06
#define ZPM_BFSET        0x07
#define ZPM_BFCPY        ZPM_BFCPY_BIT // 0x08
#define ZPM_BFLDR_BIT    (ZPM_BFCPY_BIT | ZPM_BFLOAD_BIT) // 0x09
#define ZPM_BFSTR_BIT    (ZPM_BFCPY_BIT | ZPM_BFSTORE_BIT) // 0x0a

/* memory-management unit */
#define ZPM_MEM_UNIT     0x0a
#define ZPM_NOCACHE_BIT  0x01
#define ZPM_WRPROT_BIT   0x02
#define ZPM_WRCOMB_BIT   0x02
#define ZPM_PGWRITE_BIT  0x01
#define ZPM_CACHE_BIT    0x04
#define ZPM_CACHELK_BIT  0x01
#define ZPM_CACHEREL_BIT 0x02
#define ZPM_WRTHRU_BIT   0x02
#define ZPM_CACHEWR_BIT  0x01
#define ZPM_PAGE_BIT     0x08
#define ZPM_PGREL_BIT    0x01
#define ZPM_WIRE_BIT     0x02
#define ZPM_PGAVAIL_BITS 0x0c
#define ZPM_FLUSH_BIT    0x01
#define ZPM_TLS_BITS     0x0e
#define ZPM_TWRITE_BIT   0x01
/* NOTE: 0x00 reserved */
#define ZPM_PGINIT       0x00
#define ZPM_PGNOC        (ZPM_NOCACHE_BIT) // 0x01
#define ZPM_PGWP         (ZPM_WRPROT_BIT) // 0x02
#define ZPM_PGWRC        (ZPM_WRCOMB_BIT | ZPM_PGWRITE_BIT) // 0x03
#define ZPM_PGWB         ZPM_CACHE_BIT // 0x04
#define ZPM_LKCL         (ZPM_CACHE_BIT | ZPM_CACHELK_BIT) // 0x05
#define ZPM_RELCL        (ZPM_CACHE_BIT | ZPM_CACHEREL_BIT) // 0x06
#define ZPM_PGWRT        (ZPM_CACHE_BIT | ZPM_WRTHRU_BIT | ZPM_CACHEWR_BIT)
#define ZPM_PGMAP        (ZPM_PAGE_BIT) // 0x08
#define ZPM_PGUMAP       (ZPM_PAGE_BIT | ZPM_PGREL_BIT) // 0x09
#define ZPM_PGWIRE       (ZPM_PAGE_BIT | ZPM_WIRE_BIT) // 0x0a
#define ZPM_PGUNW        (ZPM_PAGE_BIT | ZPM_WIRE_BIT | ZPM_PGREL_BIT) // 0x0b
#define ZPM_PGPRES       ZPM_PGAVAIL_BITS // 0x0c present
#define ZPM_PGINV        (ZPM_PGAVAIL_BITS | ZPM_FLUSH_BIT) // 0x0d
#define ZPM_TLDR         (ZPM_TLS_BITS) // 0x0e
#define ZPM_TSTR         (ZPM_TLS_BITS | ZPM_TWRITE_BIT) // 0x0f

/* task management; multitasking */
#define ZPM_TASK_UNIT    0x0b
#define ZPM_TCONFIG_BIT  0x01
#define ZPM_TCTX_BIT     0x02
#define ZPM_TSAVE_BIT    0x01
#define ZPM_TPROF_BIT    0x04
#define ZPM_TPINFO_BIT   0x01
#define ZPM_TPSAVE_BIT   0x01
#define ZPM_TINIT        0x00
#define ZPM_TCTL_BIT     0x08
#define ZPM_TSYS_BIT     0x01
#define ZPM_TINTR_BIT    0x02
#define ZPM_TID_BIT      0x04
#define ZPM_TEVENT_BIT   0x04
#define ZPM_TSEND_BIT    0x01
#define ZPM_TRECV_BIT    0x02
#define ZPM_TCONF        ZPM_TCONFIG_BIT // 0x01
#define ZPM_TCTX         ZPM_TCTX_BIT // 0x02
#define ZPM_TSAVE        (ZPM_TCTX_BIT | ZPM_TSAVE_BIT) // 0x03
#define ZPM_TPROF        ZPM_TPROF_BIT // 0x04
#define ZPM_TPREAD       (ZPM_TPROF_BIT | ZPM_TPINFO_BIT) // 0x05
#define ZPM_TPSTOP       (ZPM_TPROF_BIT | ZPM_TPSTOP_BIT) // 0x06
// 0x07
#define ZPM_TPHOLD       (ZPM_TPROF_BIT | ZPM_TPSTOP_BIT | ZPM_TPSAVE_BIT)
#define ZPM_TYIELD       ZPM_TCTL_BIT // 0x08
#define ZPM_TSRING       (ZPM_TCTL_BIT | ZPM_TSYS_BIT) // 0x09
#define ZPM_TINTR        (ZPM_TCTL_BIT | ZPM_TINTR_BIT) // 0x0a
#define ZPM_TSTOP        (ZPM_TCTL_BIT | ZPM_TID_BIT | ZPM_TSTOP_BIT) // 0x0b
#define ZPM_TSIG         (ZPM_TCTL_BIT | ZPM_TEVENT_BIT) // 0x0c
#define ZPM_TSEND        (ZPM_TCTL_BIT | ZPM_TEVENT_BIT | ZPM_TSEND_BIT) // 0x0d
#define ZPM_TRECV        (ZPM_TCTL_BIT | ZPM_TEVENT_BIT | ZPM_TSEND_BIT) // 0x0e
#define ZPM_IRET         0x0f // return from interrupt handler

/* inter-processor atomic operations (bus locked) */
#define ZPM_ATOM_UNIT    0x0c
#define ZPM_SET_BIT      0x01
#define ZPM_FLIP_BIT     0x02
#define ZPM_LDLINK_BIT   0x02
#define ZPM_CSTORE_BIT   0x01
#define ZPM_FADD_BIT     0x04
#define ZPM_FSUB_BIT     0x01
#define ZPM_FINC_BIT     0x02
#define ZPM_FDEC_BIT     0x01
#define ZPM_FETCH_BIT    0x08
#define ZPM_COMPARE_BIT  0x02
#define ZPM_CSWAP_BIT    0x04
#define ZPM_CSWAP2_BIT   0x01
#define ZPM_CLR          0x00
#define ZPM_SET          ZPM_SET_IT // 0x01
#define ZPM_CHG          ZPM_FLIP_BIT // 0x02
#define ZPM_LLSC         (ZPM_LDLINK_BIT | ZPM_CSTORE_BIT) // 0x03
#define ZPM_FADD         ZPM_FADD_BIT // 0x04
#define ZPM_FINC         (ZPM_FADD_BIT | ZPM_FSUB_BIT) // 0x05
#define ZPM_FDEC         (ZPM_FADD_BIT | ZPM_FINC_BIT) // 0x06
#define ZPM_FDEC         (ZPM_FADD_BIT | ZPM_FDEC_BIT) // 0x07
#define ZPM_BTAC         ZPM_FETCH_BIT // 0x08
#define ZPM_BTAS         (ZPM_FETCH_BIT | ZPM_SET_BIT) // 0x09
#define ZPM_BCCLR        (ZPM_FETCH_BIT | ZPM_COMPARE_BIT) // 0x0a
#define ZPM_BCSET        (ZPM_FETCH_BIT | ZPM_COMPARE_BIT | ZPM_SET_BIT) // 0b
#define ZPM_CAS          (ZPM_FETCH_BIT | ZPM_SWAP_BIT) // 0x0c
#define ZPM_CAS2         (ZPM_FETCH_BIT | ZPM_SWAP_BIT | ZPM_CSWAP2_BIT) // 0x0d

/* processor parameters */
#define ZPM_COPROC_UNIT  0x0f // coprocessor; FPU, VEC, ...
#define ZPM_NALU_MAX     256 // max number of ALU operations

#endif /* __ZPM_ISA_H__ */

