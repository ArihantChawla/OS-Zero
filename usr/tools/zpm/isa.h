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
#define ZPM_AND_BIT      0x01
#define ZPM_OR_BIT       0x02
#define ZPM_XOR_BITS     0x03
/* optional */
#define ZPM_BITCNT_BIT   0x04
#define ZPM_TRAIL_BIT    0x01
#define ZPM_BITPOP_BIT   0x02
#define ZPM_BFEXTRA_BIT  0x08
/* instructions */
#define ZPM_NOT          0x00 // 2's complement (reverse all bits)
#define ZPM_AND          ZPM_AND_BIT // logical bitwise AND
#define ZPM_OR           ZPM_OR_BIT // logical bitwise OR
#define ZPM_XOR          ZPM_XOR_BITS // logical bitwise XOR (exclusive OR)
/* optional operations; HAVE_BITCNT_EXTRA */
#define ZPM_CLZ          ZPM_BITCNT_BIT // 0x04 count leading zero bits
#define ZPM_CTZ          (ZPM_BITCNT_BIT | ZPM_TRAIL_BIT) // 0x05 trailing zeroes
#define ZPM_BPOP         (ZPM_BITCNT_BIT | ZPM_BITPOP_BIT) // 0x06 count 1-bits
/* optional operations; HAVE_BFCNT_EXTRA */
#define ZPM_BFCLZ        ZPM_BFEXTRA_BIT // 0x08
#define ZPM_BFCTZ        (ZPM_BFEXTRA_BIT | ZPM_BFTRAIL_BIT) // 0x09
#define ZPM_BFPOP        (ZPM_BFEXTRA_BIT | ZPM_BFPOP_BIT) // 0x0a
// NOTE: 0x0b is reserved

/*
 * SHL - shift left
 * SHR - shift right logical (zero-fill)
 * SAR - shift right arithmetic (sign-fill)
 * ROL - rotate left
 * ROR - rotate right
 * RCL - rotate left through carry
 * RCR - rotate right through carry
 * BF* - bitfield instructions (within registers)
 */

/* shifter */
#define ZPM_SHIFT_UNIT   0x01
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
/* optional operations; HAVE_SHBITFLD_EXTRA */
#define ZPM_BFSHL        ZPM_FIELD_BIT // 0x08
#define ZPM_BFSHR        (ZPM_FIELD_BIT | ZPM_RIGHT_BIT) // 0x09
#define ZPM_BFSAR        (ZPM_FIELD_BIT | ZPM_SIGNEXT_BIT) // 0x0a
#define ZPM_BFROL        ZPM_FROT_BITS // 0x0c
#define ZPM_BFROR        (ZPM_FROT_BITS | ZPM_RIGHT_BIT) // 0d
#define ZPM_BFRCL        (ZPM_FROT_BITS | ZPM_RCARRY_BIT) // 0x0e
#define ZPM_BFRCR        (ZPM_FROT_BITS | ZPM_RCARRY_BIT | ZPM_RIGHT_BIT) // 0e

/*
 * INC - increment by one
 * DEC - decrement by one
 * ADD - add
 * ADF - add, sets MSW-flags
 * ADI - add inverse (b += ~a)
 * SUB - subtract (b += ~a + 1
 */

/* arithmetic operations */
/* arithmetic unit */
#define ZPM_ARITH_UNIT   0x02
#define ZPM_DEC_BIT      0x01
#define ZPM_ADD_BIT      0x02
#define ZPM_MSW_BIT      0x01
#define ZPM_ADDINV_BIT   0x04
#define ZPM_CMP_BITS     0x05
#define ZPM_SUB_BITS     0x06
#define ZPM_VINC         ZPM_VEC_BIT // 0x08
#define ZPM_SATU_BIT     0x01
#define ZPM_SATS_BIT     0x02
#define ZPM_VSUB_BITS    0x0d
/* instructions */
#define ZPM_INC          0x00
#define ZPM_DEC          ZPM_DEC_BIT // 0x01
#define ZPM_ADD          ZPM_ADD_BIT // 0x02
#define ZPM_ADF          (ZPM_ADD_BIT | ZPM_MSW_BIT) // 0x03
#define ZPM_ADI          (ZPM_ADDINV_BIT) // // 0x04
#define ZPM_CMP          ZPM_CMP_BITS // 0x05
#define ZPM_SUB          ZPM_SUB_BITS // 0x06
#define ZPM_SBB          (ZPM_SUB_BITS | ZPM_MSW_BIT) // 0x07
/* optional [vector] instructions; HAVE_ARITH_VEC */
#define ZPM_VDEC         (ZPM_VEC_BIT | ZPM_DEC_BIT) // 0x09
#define ZPM_VADD         (ZPM_VEC_BIT | ZPM_ADD_BIT) // 0x0a
#define ZPM_VADDUS       (ZPM_VEC_BIT | ZPM_ADD_BIT | ZPM_SATU_BIT) // 0x0b
#define ZPM_VADDSS       (ZPM_VEC_BIT | ZPM_ADD_BIT | ZPM_SATS_BIT) // 0x0c
#define ZPM_VSUB         (ZPM_VSUB_BITS) // 0x0d
#define ZPM_VSUBUS       0x0e
#define ZPM_VSUBSS       0x0f

/* flow control; branch and subroutine operations */
#define ZPM_FLOW_UNIT    0x03
#define ZPM_CALL_BIT     0x01
#define ZPM_BZERO_BIT    0x02
#define ZPM_NOTZERO_BIT  0x01
#define ZPM_ZERO_BIT     0x01
#define ZPM_OVERFLOW_BIT 0x04
#define ZPM_CARRY_BITS   0x06
#define ZPM_LESS_BIT     0x08
#define ZPM_GREATER_BITS 0x0a
#define ZPM_RET_BITS     0x0c
#define ZPM_INTRET_BIT   0x01 // return from interrupt handler
#define ZPM_FCOND_BITS   0x0e // conditional jump or call
/* instructions */
#define ZPM_JMP          0x00
#define ZPM_CALL         ZPM_CALL_BIT // 0x01
#define ZPM_BZ           ZPM_BZERO_BIT // 0x02
#define ZPM_BNZ          ZPM_NOTZERO_BIT // 0x03
#define ZPM_BO           (ZPM_OVERFLOW_BIT) // 0x04
#define ZPM_BNO          (ZPM_OVERFLOW_BIT | ZPM_ZERO_BIT) // 0x05
#define ZPM_BC           ZPM_CARRY_BITS // 0x06
#define ZPM_BNC          (ZPM_CARRY_BITS | ZPM_ZERO_BIT) // 0x07
#define ZPM_BLT          ZPM_LESS_BIT // 0x08
#define ZPM_BLE          (ZPM_LESS_BIT | ZPM_ZERO_BIT) // 0x09
#define ZPM_BGT          ZPM_GREATER_BITS // 0x0a
#define ZPM_BGE          (ZPM_GREATER_BITS | ZPM_ZERO_BIT) // 0x0b
#define ZPM_RET          ZPM_RET_BITS // 0x0c
#define ZPM_IRET         (ZPM_RET_BITS | ZPM_INTRET_BIT) // 0x0d
#define ZPM_CJMP         ZPM_FCOND_BITS // 0x0e
#define ZPM_CCALL        (ZPM_FCOND_BITS | ZPM_CALL_BIT) // 0x0f

/*
 * MUL    - multiply
 * MULS   - signed multiply
 * MULW   - return two result words
 * MULSW  - signed, return two words
 * MULHI  - return high result word
 * MULSHI - signed, return high result word
 * VMUL   - SIMD-multiply
 * MAC    - multiply and add to accumulator (register)
 */

/* multiplier */
#define ZPM_MUL_UNIT     0x04
#define ZPM_MSIGN_BIT    0x01
#define ZPM_MHIGH_BIT    0x02
#define ZPM_MWIDTH_BIT   0x04
/* optional */
#define ZPM_MULEXTRA_BIT 0x08
/* instructions */
#define ZPM_MUL          0x00
#define ZPM_MULS         ZPM_MSIGN_BIT // 0x01
#define ZPM_MULHI        ZPM_MHIGH_BIT // 0x02
#define ZPM_MULSHI       (ZPM_MHIGH_BIT | ZPM_MSIGN_BIT) // 0x03
#define ZPM_MULW         ZPM_MWIDTH_BIT // 0x04
#define ZPM_MULSW        (ZPM_MWIDTH_BIT | ZPM_MSIGN_BIT) // 0x05
// NOTE: 0x06, 0x07 are reserved
/* optional instructions; HAVE_MUL_EXTRA */
#define ZPM_MAC          ZPM_MULEXTRA_BIT // 0x08 multiply-add
#define ZPM_MACS         (ZPM_MULEXTRA_BIT | ZPM_MSIGN_BIT) // 0x09
#define ZPM_MACW         (ZPM_MULEXTRA_BIT | ZPM_MWIDTH_BIT) // 0x0a
// 0x0b
#define ZPM_MACSW        (ZPM_MULEXTRA_BIT | ZPM_MWIDTH_BIT | ZPM_MSIGN_BIT)

/*
 * DIV    - divide
 * REM    - return remainder in second register
 * SETRND - set round-mode
 * SETRNC - set truncate-mode (default)
 * DREC   - divide with reciprocal
 * CREC   - compute [and cache] reciprocal
 */

/* divider */
#define ZPM_DIV_UNIT     0x05
#define ZPM_REM_BIT      0x01 // remainder-flag
/* optional */
#define ZPM_DREC_BIT     0x02 // reciprocal division
#define ZPM_CREC_BIT     0x01 // calculate reciprocal
#define ZPM_ROUND_BIT    0x04
#define ZPM_TRUNC_BIT    0x01
/* instructions */
#define ZPM_DIV          0x00 // division, result in ZPM_RET_LO
#define ZPM_REM          ZPM_REM_BIT // 0x01 remainder of division in ZPM_RET_HI
/* optional operations; HAVE_DIV_EXTRA */
#define ZPM_DREC         ZPM_DREC_BIT // 0x02 reciprocal division
#define ZPM_CREC         (ZPM_DREC_BIT | ZPM_CREC_BIT) // 0x03
#define ZPM_SETRND       ZPM_ROUND_BIT // 0x04
#define ZPM_SETRNC       (ZPM_ROUND_BIT | ZPM_TRUNC_BIT) // 0x05

/*
 * CPY - register to register
 * LDR - memory to register
 * STR - register to memory
 */

/* load-store operations */
#define ZPM_XFER_UNIT    0x06
#define ZPM_LOAD_BIT     0x01
#define ZPM_STORE_BIT    0x02 // write operation
#define ZPM_SCPY_BIT     0x04
#define ZPM_XCPY_BIT     0x08
#define ZPM_XFCOND_BITS  0x0c
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
#define ZPM_CCPY         ZPM_COND_BITS // 0x0c
#define ZPM_CLDR         (ZPM_XFCOND_BITS | ZPM_LOAD_BIT) // 0x0d
#define ZPM_CSTR         (ZPM_XFCOND_BITS | ZPM_STORE_BIT) // 0x0e

/*
 * SREG - system register (msw, fp, sp)
 * XREG - system-only registers
 */

/* stack operations */
#define ZPM_STACK_UNIT   0x07
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
/* optional operations; HAVE_STACK_EXTRA */
#define ZPM_STK          ZPM_STK_BITS // 0x0c
#define ZPM_SSTK         (ZPM_STK_BITS | ZPM_SYSSTK_BIT) // 0x0d
#define ZPM_STKSZ        ZPM_STKSIZE_BITS // 0x0e
#define ZPM_SSTKSZ       (ZPM_STKSIZE_BITS | ZPM_SYSSTK_BIT) // 0x0f

/* I/O operations */
#define ZPM_IO_UNIT      0x08
#define ZPM_IOWRITE_BIT  0x01
/* optional */
#define ZPM_IOPCHK_BIT   0x02
#define ZPM_IOPSET_BIT   0x04
/* instructions */
#define ZPM_IOR          0x00
#define ZPM_IOW          (ZPM_IOWRITE_BIT) // 0x01
/* optional instructions; HAVE_IO_EXTRA */
#define ZPM_IORCHK       ZPM_IOPCHK_BIT
#define ZPM_IORWCHK      (ZPM_IOPCHK_BIT | ZPM_IOWRITE_BIT)
#define ZPM_IOPRSET      ZPM_IOPSET_BITS
#define ZPM_IOPRWSET     (ZPM_IOPSET_BITS | ZPM_IOWRITE_BIT)

/*
 * TSET, TCLR  - set and clear breakpoints
 * SCALL, SRET - invoke and return from system calls
 * WFE, SEV    - wait for and send events
 * WFI, SMI    - wait for and send [inter-processor] interrupts
 */

/* system operations */
#define ZPM_SYS_UNIT     0x09
#define ZPM_RESET_BIT    0x01
#define ZPM_TRAP_BIT     0x02
#define ZPM_INTR_BIT     0x04
#define ZPM_BLOCK_BIT    0x01
#define ZPM_SYS_BITS     0x06
#define ZPM_SYSRET_BIT   0x01
#define ZPM_EVENT_BIT    0x08
#define ZPM_SEND_BIT     0x01
/* instructions */
#define ZPM_BOOT         0x00 // execute bootstrap code from ROM
#define ZPM_RST          ZPM_RESET_BIT // 0x01
#define ZPM_TSET         ZPM_TRAP_BIT // 0x02
#define ZPM_TCLR         (ZPM_TRAP_BIT | ZPM_RESET_BIT) // 0x03
#define ZPM_STI          ZPM_INTR_BIT // 0x04
#define ZPM_CLI          (ZPM_INTR_BIT | ZPM_BLOCK_BIT) // 0x05
#define ZPM_SCALL        ZPM_SYS_BITS // 0x06
#define ZPM_SRET         (ZPM_SYS_BITS | ZPM_SYSRET_BIT) // 0x07
/* optional instructions; HAVE_SYS_EXTRA */
#define ZPM_WFE          ZPM_EVENT_BIT // 0x08
#define ZPM_SEV          (ZPM_EVENT_BIT | ZPM_SEND_BIT) // 0x09
#define ZPM_WFI          (ZPM_EVENT_BIT | ZPM_TRAP_BIT) // 0x0a
#define ZPM_SMI          (ZPM_EVENT_BIT | ZPM_TRAP_BIT | ZPM_SEND_BIT) // 0x0b

/* OPTIONAL UNITS */

/* bitfields are parts of registers */

/* bitfield operations; HAVE_BITFLD_UNIT */
#define ZPM_BITFLD_UNIT  0x0a
/* shift and rotate operations */
#define ZPM_BFCPY_BIT    0x04
#define ZPM_BFLOAD_BIT   0x01
#define ZPM_BFSTORE_BIT  0x02
/* optional */
#define ZPM_BFEXTRA_BIT  0x08
/* instructions */
#define ZPM_BFNOT        0x00
#define ZPM_BFAND        0x01
#define ZPM_BFOR         0x02
#define ZPM_BFXOR        0x03
#define ZPM_BFCPY        ZPM_BFCPY_BIT // 0x04
#define ZPM_BFLDR_BIT    (ZPM_BFCPY_BIT | ZPM_BFLOAD_BIT) // 0x05
#define ZPM_BFSTR_BIT    (ZPM_BFCPY_BIT | ZPM_BFSTORE_BIT) // 0x06

/* memory-management unit; HAVE_MEM_UNIT */
#define ZPM_MEM_UNIT     0x0b
/* page/memory flags */
#define ZPM_MEMEXEC_BIT  0x001 // execute permission
#define ZPM_MEMWRITE_BIT 0x002 // write permission
#define ZPM_MEMREAD_BIT  0x004 // read permission
#define ZPM_MEMSYS_BIT   0x008 // system page
#define ZPM_MEMGLOB_BIT  0x010 // global page (shared)
#define ZPM_MEMWIRE_BIT  0x020 // wired page, i.e. stick in RAM
#define ZPM_MEMNOC_BIT   0x040 // page not cached
#define ZPM_MEMWRT_BIT   0x080 // page cached write-through (vs. write-back)
#define ZPM_MEMWRC_BIT   0x100 // combine write operations for page
#define ZPM_MEMPRES_BIT  0x200 // page present in RAM
#define ZPM_MEMDIRTY_BIT 0x400 // dirty page (has been written to)
#define ZPM_MEMBUSY_BIT  0x800 // page is locked/busy (e.g. being configured)
/* page operations */
#define ZPM_PGSETATR_BIT 0x01 // set page attribute-bits
#define ZPM_PGCLRATR_BIT 0x02 // clear page attribute-bits
#define ZPM_PGINV_BITS   0x03 // invalidate page TLB-entry
/* optional operations */
/* cacheline operations */
#define ZPM_CACHE_BIT    0x04
#define ZPM_CACHELK_BIT  0x01 // lock cacheline
#define ZPM_CACHEREL_BIT 0x02 // release/unlock cacheline
/* thread-local operations */
#define ZPM_TLS_BIT      0x08
#define ZPM_TLOAD_BIT    0x01 // thread-local memory to register
#define ZPM_TSTORE_BIT   0x02 // register to thread-local memory
#define ZPM_TUMAP_BITS   0x03 // unmap thread-local page
/* instructions */
#define ZPM_PGINIT       0x00 // args: vmadr, flg
#define ZPM_PGSETF       ZPM_PGSETATR_BIT // args: vmadr, flg
#define ZPM_PGCLRF       ZPM_PGCLRATR_BIT // args: vmadr, flg
#define ZPM_PGINV        ZPM_PGINV_BITS // args: vmadr
/* optional operations; HAVE_CACHE_EXTRA */
#define ZPM_CFTC         ZPM_CACHE_BIT // args: cladr (prefetch)
#define ZPM_CLK          (ZPM_CACHE_BIT | ZPM_CACHELK_BIT) // args: cladr (lock)
#define ZPM_CREL         (ZPM_CACHE_BIT | ZPM_CACHEREL_BIT) // args: cladr
// NOTE: 0x07 is reserved
/* optional operations; HAVE_TLS_EXTRA */
#define ZPM_TPGINIT      ZPM_TLS_BIT // args: flg, if PRES then physadr
#define ZPM_TLDR         (ZPM_TLS_BIT | ZPM_TLOAD_BIT) // args: ofs, reg
#define ZPM_TSTR         (ZPM_TLS_BIT | ZPM_TSTORE_BIT) // args: reg, ofs
#define ZPM_TPGUMAP      (ZPM_TLS_BIT | ZPM_TUMAP_BITS) // args: vmadr

/*
 * TCONF        - set task attributes
 * TLOAD, TSAVE - load and save task context
 * TP*          - task profiling
 * TYIELD       - release processor
 * TSWTCH       - switch to task
 * TSTOP        - stop task
 * TKILL        - kill task
 * TSIG         - send signal to task

/* task management; multitasking; HAVE_TASK_UNIT */
#define ZPM_TASK_UNIT    0x0c
#define ZPM_TCONFIG_BIT  0x01
#define ZPM_TCTX_BIT     0x02
#define ZPM_TSAVE_BIT    0x01
#define ZPM_TPROF_BIT    0x04
#define ZPM_TPINFO_BIT   0x01
#define ZPM_TPSAVE_BIT   0x01
#define ZPM_TPSTOP_BIT   0x02
#define ZPM_TCTL_BIT     0x08
#define ZPM_TSWITCH_BIT  0x01
#define ZPM_TSTOP_BIT    0x01
#define ZPM_TKILL_BIT    0x01
#define ZPM_STOP_BIT     0x02
#define ZPM_TSIG_BIT     0x04
#define ZPM_TEVENT_BIT   0x04
#define ZPM_TSEND_BIT    0x01
#define ZPM_TRECV_BIT    0x02
/* instructions */
#define ZPM_TINIT        0x00
#define ZPM_TCONF        ZPM_TCONFIG_BIT // 0x01
#define ZPM_TLOAD        ZPM_TCTX_BIT // 0x02
#define ZPM_TSAVE        (ZPM_TCTX_BIT | ZPM_TSAVE_BIT) // 0x03
#define ZPM_TPROF        ZPM_TPROF_BIT // 0x04
#define ZPM_TPREAD       (ZPM_TPROF_BIT | ZPM_TPINFO_BIT) // 0x05
#define ZPM_TPSTOP       (ZPM_TPROF_BIT | ZPM_TPSTOP_BIT) // 0x06
#define ZPM_TPHOLD       (ZPM_TPROF_BIT | ZPM_TPSTOP_BIT | ZPM_TPSAVE_BIT) // 07
/* optional instructions; HAVE_TCTL_EXTRA */
#define ZPM_TYIELD       ZPM_TCTL_BIT // 0x08
#define ZPM_TSWTCH       (ZPM_TCTL_BIT | ZPM_TSWITCH_BIT) // 0x09
#define ZPM_TSTOP        (ZPM_TCTL_BIT | ZPM_TSTOP_BIT) // 0x0a
#define ZPM_TKILL        (ZPM_TCTL_BIT | ZPM_TSTOP_BIT | ZPM_TKILL_BIT) // 0x0b
#define ZPM_TSIG         (ZPM_TCTL_BIT | ZPM_TSIG_BIT) // 0x0c
#define ZPM_TSEV         (ZPM_TCTL_BIT | ZPM_TEVENT_BIT | ZPM_TSEND_BIT) // 0x0d
#define ZPM_TREV         (ZPM_TCTL_BIT | ZPM_TEVENT_BIT | ZPM_TRECV_BIT) // 0x0e
#define ZPM_TRET         0x0f // return from task; exit if toplevel

/* inter-processor atomic operations (bus locked); HAVE_ATOM_UNIT */
#define ZPM_ATOM_UNIT    0x0d
#define ZPM_SET_BIT      0x01
#define ZPM_FLIP_BIT     0x02
#define ZPM_LLSC_BITS    0x03
#define ZPM_FADD_BIT     0x04
#define ZPM_FSUB_BIT     0x01
#define ZPM_FINC_BIT     0x02
#define ZPM_FDEC_BITS    (ZPM_FADD_BIT | 0x03)
#define ZPM_FETCH_BIT    0x08
#define ZPM_COMPARE_BIT  0x02
#define ZPM_CSWAP_BIT    0x04
#define ZPM_CSWAP2_BIT   0x01
#define ZPM_CLR          0x00
#define ZPM_SET          ZPM_SET_BIT // 0x01
#define ZPM_CHG          ZPM_FLIP_BIT // 0x02
#define ZPM_LLSC         ZPM_LLSC_BITS // 0x03
#define ZPM_FADD         ZPM_FADD_BIT // 0x04
#define ZPM_FSUB         (ZPM_FADD_BIT | ZPM_FSUB_BIT) // 0x05
#define ZPM_FINC         (ZPM_FADD_BIT | ZPM_FINC_BIT) // 0x06
#define ZPM_FDEC         ZPM_FDEC_BITS // 0x07
#define ZPM_BTAC         ZPM_FETCH_BIT // 0x08
#define ZPM_BTAS         (ZPM_FETCH_BIT | ZPM_SET_BIT) // 0x09
#define ZPM_BCCLR        (ZPM_FETCH_BIT | ZPM_COMPARE_BIT) // 0x0a
#define ZPM_BCSET        (ZPM_FETCH_BIT | ZPM_COMPARE_BIT | ZPM_SET_BIT) // 0b
#define ZPM_CAS          (ZPM_FETCH_BIT | ZPM_SWAP_BIT) // 0x0c
#define ZPM_CAS2         (ZPM_FETCH_BIT | ZPM_SWAP_BIT | ZPM_CSWAP2_BIT) // 0x0d

/* extension operations; instructions identified by the imm8-field in opcodes */
#define ZPM_EXT_UNIT     0x0e

/* extension instructions; support vectors with 8- and 16-bit vector items */
/*
 * - vector size: 32 << arg1sz
 * - item size: 8 << arg2sz
 */
/* logical operations */
#define ZPM_VNOT         0x00
#define ZPM_VAND         0x01
#define ZPM_VOR          0x02
#define ZPM_VXOR
/* multiplier and divider operations */
/* multiplier operations */
#define ZPM_VMUL_BIT     0x20
#define ZPM_VSIGNED_BIT  0x01
#define ZPM_VMULHI_BIT   0x02
#define ZPM_VMULW_BIT    0x04
#define ZPM_VMUL         (ZPM_VMUL_BIT) // 0x20
#define ZPM_VMULS        (ZPM_VMUL_BIT | ZPM_VSIGNED_BIT) // 0x21
#define ZPM_VMULHI       (ZPM_VMUL_BIT | ZPM_VMULHI_BIT) // 0x22
#define ZPM_VMULSHI      (ZPM_VMUL_BIT | ZPM_VMULHI_BIT | ZPM_VSIGNED_BIT) // 23
#define ZPM_VMULW        (ZPM_VMUL_BIT | ZPM_VMULW_BIT) // 0x24
#define ZPM_VMULSW       (ZPM_VMUL_BIT | ZPM_VSIGNED_BIT) // 0x25
/* divider operations */
#define ZPM_VDIV_BIT     0x08
#define ZPM_VREM_BIT     0x01
#define ZPM_VREC_BIT     0x02
#define ZPM_VCREC_BIT    0x01
#define ZPM_VDIV         (0x20 | ZPM_VDIV_BIT) // 0x28
#define ZPM_VREM         (0x20 | ZPM_VDIV_BIT | ZPM_VREM_BIT) // 0x29
#define ZPM_VDREC        (0x20 | ZPM_VDIV_BIT | ZPM_VREC_BIT) // 0x2a
// 0x2b
#define ZPM_VCREC        (0x23 | ZPM_DIV_BIT | ZPM_VREC_BIT | ZPM_VCREC_BIT)
/* skip ["branch"] operations */
#define ZPM_VSKIPZ       0x30
#define ZPM_VSKIPNZ      0x31
#define ZPM_VSKIPE       ZPM_VSKIPZ
#define ZPM_VSKIPNE      ZPM_VSKIPNZ
#define ZPM_VSKIPO       0x32
#define ZPM_VSKIPNO      0x33
#define ZPM_VSKIPC       0x34
#define ZPM_VSKIPNC      0x35
#define ZPM_VSKIPLT      0x36
#define ZPM_VSKIPLE      0x37
#define ZPM_VSKIPGT      0x38
#define ZPM_VSKIPGE      0x39
/* shifter operations */
#define ZPM_VSHIFT_BIT   0x40 // count in low 4 bits
#define ZPM_VROT_BITS    0x50 // count in low 4 bits
#define ZPM_VRIGHT_BIT   0x01
#define ZPM_VSIGNEXT_BIT 0x02
#define ZPM_VCARRY_BIT   0x02
#define ZPM_VSHL         ZPM_VSHIFT_BIT // 0x40
#define ZPM_VSHL         (ZPM_VSHIFT_BIT | ZPM_VRIGHT_BIT) // 0x41
#define ZPM_VSAR         (ZPM_VSHIFT_BIT | ZPM_VSIGNEXT_BIT) // 0x42
#define ZPM_VROL         ZPM_VROT_BITS // 0x50
#define ZPM_VROR         (ZPM_VROT_BITS | ZPM_VRIGHT_BIT) // 0x51
#define ZPM_RCL          (ZPM_VROT_BITS | ZPM_VCARRY_BIT) // 0x52
#define ZPM_VRCR         (ZPM_VROT_BITS | ZPM_VCARRY_BIT | ZPM_VRIGHT_BIT) // 53
/* miscellaneous operations */
#define ZPM_VUNPK        0xfe
#define ZPM_VPK          0xff

/* processor parameters */
#define ZPM_COPROC_UNIT  0x0f // coprocessor; FPU, VEC, ...
#define ZPM_NALU_MAX     256 // max number of ALU operations

#endif /* __ZPM_ISA_H__ */

