#ifndef __VPU_ISA_H__
#define __VPU_ISA_H__

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
#define VPU_BITS_UNIT    0x00
#define VPU_AND_BIT      0x01
#define VPU_OR_BIT       0x02
#define VPU_XOR_BITS     0x03
/* optional */
#define VPU_BITCNT_BIT   0x04
#define VPU_TRAIL_BIT    0x01
#define VPU_BITPOP_BIT   0x02
#define VPU_BFEXTRA_BIT  0x08
/* instructions */
#define VPU_NOT          0x00 // 2's complement (reverse all bits)
#define VPU_AND          VPU_AND_BIT // logical bitwise AND
#define VPU_OR           VPU_OR_BIT // logical bitwise OR
#define VPU_XOR          VPU_XOR_BITS // logical bitwise XOR (exclusive OR)
/* optional operations; HAVE_BITCNT_EXTRA */
#define VPU_CLZ          VPU_BITCNT_BIT // 0x04 count leading zero bits
#define VPU_CTZ          (VPU_BITCNT_BIT | VPU_TRAIL_BIT) // 0x05 trailing zeroes
#define VPU_BPOP         (VPU_BITCNT_BIT | VPU_BITPOP_BIT) // 0x06 count 1-bits
/* optional operations; HAVE_BFCNT_EXTRA */
#define VPU_BFCLZ        VPU_BFEXTRA_BIT // 0x08
#define VPU_BFCTZ        (VPU_BFEXTRA_BIT | VPU_BFTRAIL_BIT) // 0x09
#define VPU_BFPOP        (VPU_BFEXTRA_BIT | VPU_BFPOP_BIT) // 0x0a
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
#define VPU_SHIFT_UNIT   0x01
#define VPU_RIGHT_BIT    0x01
#define VPU_SIGNEXT_BIT  0x02
#define VPU_ROT_BIT      0x04
#define VPU_RCARRY_BIT   0x02
/* optional */
#define VPU_FIELD_BIT    0x08
#define VPU_FROT_BITS    (VPU_FIELD_BIT | VPU_ROT_BIT)
/* instructions */
#define VPU_SHL          0x00
#define VPU_SHR          VPU_RIGHT_BIT // 0x01
// NOTE: 0x02 is reserved
#define VPU_SAR          (VPU_SIGNEXT_BIT | VPU_RIGHT_BIT) // 0x03
#define VPU_ROL          VPU_ROT_BIT // 0x04
#define VPU_ROR          (VPU_ROT_BIT | VPU_RIGHT_BIT) // 0x05
#define VPU_RCL          (VPU_ROT_BIT | VPU_RCARRY_BIT) // 0x06
#define VPU_RCR          (VPU_ROT_BIT | VPU_RCARRY_BIT | VPU_RIGHT_BIT) // 0x07
/* optional operations; HAVE_SHBITFLD_EXTRA */
#define VPU_BFSHL        VPU_FIELD_BIT // 0x08
#define VPU_BFSHR        (VPU_FIELD_BIT | VPU_RIGHT_BIT) // 0x09
#define VPU_BFSAR        (VPU_FIELD_BIT | VPU_SIGNEXT_BIT | VPU_RIGHT_BIT) // 0b
#define VPU_BFROL        VPU_FROT_BITS // 0x0c
#define VPU_BFROR        (VPU_FROT_BITS | VPU_RIGHT_BIT) // 0d
#define VPU_BFRCL        (VPU_FROT_BITS | VPU_RCARRY_BIT) // 0x0e
#define VPU_BFRCR        (VPU_FROT_BITS | VPU_RCARRY_BIT | VPU_RIGHT_BIT) // 0e

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
#define VPU_ARITH_UNIT   0x02
#define VPU_DEC_BIT      0x01
#define VPU_ADD_BIT      0x02
#define VPU_MSW_BIT      0x01
#define VPU_ADDINV_BIT   0x04
#define VPU_CMP_BITS     0x05
#define VPU_SUB_BITS     0x06
#define VPU_VINC         VPU_VEC_BIT // 0x08
#define VPU_SATU_BIT     0x01
#define VPU_SATS_BIT     0x02
#define VPU_VSUB_BITS    0x0d
/* instructions */
#define VPU_INC          0x00
#define VPU_DEC          VPU_DEC_BIT // 0x01
#define VPU_ADD          VPU_ADD_BIT // 0x02
#define VPU_ADF          (VPU_ADD_BIT | VPU_MSW_BIT) // 0x03
#define VPU_ADI          (VPU_ADDINV_BIT) // // 0x04
#define VPU_CMP          VPU_CMP_BITS // 0x05
#define VPU_SUB          VPU_SUB_BITS // 0x06
#define VPU_SBB          (VPU_SUB_BITS | VPU_MSW_BIT) // 0x07
/* optional [vector] instructions; HAVE_ARITH_VEC */
#define VPU_VDEC         (VPU_VEC_BIT | VPU_DEC_BIT) // 0x09
#define VPU_VADD         (VPU_VEC_BIT | VPU_ADD_BIT) // 0x0a
#define VPU_VADDUS       (VPU_VEC_BIT | VPU_ADD_BIT | VPU_SATU_BIT) // 0x0b
#define VPU_VADDSS       (VPU_VEC_BIT | VPU_ADD_BIT | VPU_SATS_BIT) // 0x0c
#define VPU_VSUB         (VPU_VSUB_BITS) // 0x0d
#define VPU_VSUBUS       0x0e
#define VPU_VSUBSS       0x0f

/* flow control; branch and subroutine operations */
#define VPU_FLOW_UNIT    0x03
#define VPU_CALL_BIT     0x01
#define VPU_BZERO_BIT    0x02
#define VPU_NOTZERO_BIT  0x01
#define VPU_ZERO_BIT     0x01
#define VPU_OVERFLOW_BIT 0x04
#define VPU_CARRY_BITS   0x06
#define VPU_LESS_BIT     0x08
#define VPU_GREATER_BITS 0x0a
#define VPU_RET_BITS     0x0c
#define VPU_INTRET_BIT   0x01 // return from interrupt handler
#define VPU_FCOND_BITS   0x0e // conditional jump or call
/* instructions */
#define VPU_JMP          0x00
#define VPU_CALL         VPU_CALL_BIT // 0x01
#define VPU_BZ           VPU_BZERO_BIT // 0x02
#define VPU_BNZ          VPU_NOTZERO_BIT // 0x03
#define VPU_BO           (VPU_OVERFLOW_BIT) // 0x04
#define VPU_BNO          (VPU_OVERFLOW_BIT | VPU_ZERO_BIT) // 0x05
#define VPU_BC           VPU_CARRY_BITS // 0x06
#define VPU_BNC          (VPU_CARRY_BITS | VPU_ZERO_BIT) // 0x07
#define VPU_BLT          VPU_LESS_BIT // 0x08
#define VPU_BLE          (VPU_LESS_BIT | VPU_ZERO_BIT) // 0x09
#define VPU_BGT          VPU_GREATER_BITS // 0x0a
#define VPU_BGE          (VPU_GREATER_BITS | VPU_ZERO_BIT) // 0x0b
#define VPU_RET          VPU_RET_BITS // 0x0c
#define VPU_IRET         (VPU_RET_BITS | VPU_INTRET_BIT) // 0x0d
#define VPU_CJMP         VPU_FCOND_BITS // 0x0e
#define VPU_CCALL        (VPU_FCOND_BITS | VPU_CALL_BIT) // 0x0f

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
#define VPU_MUL_UNIT     0x04
#define VPU_MSIGN_BIT    0x01
#define VPU_MHIGH_BIT    0x02
#define VPU_MWIDTH_BIT   0x04
/* optional */
#define VPU_MULEXTRA_BIT 0x08
/* instructions */
#define VPU_MUL          0x00
#define VPU_MULS         VPU_MSIGN_BIT // 0x01
#define VPU_MULHI        VPU_MHIGH_BIT // 0x02
#define VPU_MULSHI       (VPU_MHIGH_BIT | VPU_MSIGN_BIT) // 0x03
#define VPU_MULW         VPU_MWIDTH_BIT // 0x04
#define VPU_MULSW        (VPU_MWIDTH_BIT | VPU_MSIGN_BIT) // 0x05
// NOTE: 0x06, 0x07 are reserved
/* optional instructions; HAVE_MUL_EXTRA */
#define VPU_MAC          VPU_MULEXTRA_BIT // 0x08 multiply-add
#define VPU_MACS         (VPU_MULEXTRA_BIT | VPU_MSIGN_BIT) // 0x09
#define VPU_MACW         (VPU_MULEXTRA_BIT | VPU_MWIDTH_BIT) // 0x0a
// 0x0b
#define VPU_MACSW        (VPU_MULEXTRA_BIT | VPU_MWIDTH_BIT | VPU_MSIGN_BIT)

/*
 * DIV    - divide
 * REM    - return remainder in second register
 * SETRND - set round-mode
 * SETRNC - set truncate-mode (default)
 * DREC   - divide with reciprocal
 * CREC   - compute [and cache] reciprocal
 */

/* divider */
#define VPU_DIV_UNIT     0x05
#define VPU_REM_BIT      0x01 // remainder-flag
/* optional */
#define VPU_DREC_BIT     0x02 // reciprocal division
#define VPU_CREC_BIT     0x01 // calculate reciprocal
#define VPU_ROUND_BIT    0x04
#define VPU_TRUNC_BIT    0x01
/* instructions */
#define VPU_DIV          0x00 // division, result in VPU_RET_LO
#define VPU_REM          VPU_REM_BIT // 0x01 remainder of division in VPU_RET_HI
/* optional operations; HAVE_DIV_EXTRA */
#define VPU_DREC         VPU_DREC_BIT // 0x02 reciprocal division
#define VPU_CREC         (VPU_DREC_BIT | VPU_CREC_BIT) // 0x03
#define VPU_SETRND       VPU_ROUND_BIT // 0x04
#define VPU_SETRNC       (VPU_ROUND_BIT | VPU_TRUNC_BIT) // 0x05

/*
 * CPY - register to register
 * LDR - memory to register
 * STR - register to memory
 */

/* load-store operations */
#define VPU_XFER_UNIT    0x06
#define VPU_LOAD_BIT     0x01
#define VPU_STORE_BIT    0x02 // write operation
#define VPU_SCPY_BIT     0x04
#define VPU_XCPY_BIT     0x08
#define VPU_XFCOND_BITS  0x0c
/* instructions */
#define VPU_CPY          0x00
#define VPU_LDR          (VPU_LOAD_BIT) // 0x01
#define VPU_STR          (VPU_STORE_BIT) // 0x02
#define VPU_SCPY         VPU_SCPY_BIT // 0x04
#define VPU_SLDR         (VPU_SCPY_BIT | VPU_LOAD_BIT) // 0x05
#define VPU_SSTR         (VPU_SCPY_BIT | VPU_STORE_BIT) // 0x06
#define VPU_XCPY         VPU_XCPY_BIT // 0x08
#define VPU_XLDR         (VPU_XCPY_BIT | VPU_LOAD_BIT) // 0x09
#define VPU_XSTR         (VPU_XCPY_BIT | VPU_STORE_BIT) // 0x0a
#define VPU_CCPY         VPU_COND_BITS // 0x0c
#define VPU_CLDR         (VPU_XFCOND_BITS | VPU_LOAD_BIT) // 0x0d
#define VPU_CSTR         (VPU_XFCOND_BITS | VPU_STORE_BIT) // 0x0e

/*
 * SREG - system register (msw, fp, sp)
 * XREG - system-only registers
 */

/* stack operations */
#define VPU_STACK_UNIT   0x07
#define VPU_POP_BIT      0x01
#define VPU_ALLREGS_BIT  0x02
#define VPU_SREG_BIT     0x04
#define VPU_XREG_BIT     0x08
#define VPU_STK_BITS     0x0c
#define VPU_SYSSTK_BIT   0x01
#define VPU_STKSIZE_BITS 0x0e
#define VPU_PSH          0x00
#define VPU_POP          VPU_POP_BIT // 0x01
#define VPU_PSHA         VPU_ALLREGS_BIT // 0x02
#define VPU_POPA         (VPU_ALLREGS_BIT | VPU_POP_BIT) // 0x03
#define VPU_SPSH         VPU_SREG_BIT // 0x04
#define VPU_SPOP         (VPU_SREG_BIT | VPU_POP_BIT) // 0x05
#define VPU_SPSHA        (VPU_SREG_BIT | VPU_ALLREGS_BIT) // 0x06
#define VPU_SPOPA        (VPU_SREG_BIT | VPU_ALLREGS_BIT | VPU_POP_BIT) // 0x07
#define VPU_XPSH         VPU_XREG_BIT // 0x08
#define VPU_XPOP         (VPU_XREG_BIT | VPU_POP_BIT) // 0x09
#define VPU_XPSHA        (VPU_XREG_BIT | XPM_ALLREGS_BIT) // 0x0a
#define VPU_XPOPA        (VPU_XREG_BIT | XPM_ALLREGS_BIT | VPU_POP_BIT) // 0x0b
/* optional operations; HAVE_STACK_EXTRA */
#define VPU_STK          VPU_STK_BITS // 0x0c
#define VPU_SSTK         (VPU_STK_BITS | VPU_SYSSTK_BIT) // 0x0d
#define VPU_STKSZ        VPU_STKSIZE_BITS // 0x0e
#define VPU_SSTKSZ       (VPU_STKSIZE_BITS | VPU_SYSSTK_BIT) // 0x0f

/* I/O operations */
#define VPU_IO_UNIT      0x08
#define VPU_IOWRITE_BIT  0x01
/* optional */
#define VPU_IOPCHK_BIT   0x02
#define VPU_IOPSET_BIT   0x04
/* instructions */
#define VPU_IOR          0x00
#define VPU_IOW          (VPU_IOWRITE_BIT) // 0x01
/* optional instructions; HAVE_IO_EXTRA */
#define VPU_IORCHK       VPU_IOPCHK_BIT
#define VPU_IORWCHK      (VPU_IOPCHK_BIT | VPU_IOWRITE_BIT)
#define VPU_IOPRSET      VPU_IOPSET_BITS
#define VPU_IOPRWSET     (VPU_IOPSET_BITS | VPU_IOWRITE_BIT)

/*
 * TSET, TCLR  - set and clear breakpoints
 * SCALL, SRET - invoke and return from system calls
 * WFE, SEV    - wait for and send events
 * WFI, SMI    - wait for and send [inter-processor] interrupts
 */

/* system operations */
#define VPU_SYS_UNIT     0x09
#define VPU_RESET_BIT    0x01
#define VPU_TRAP_BIT     0x02
#define VPU_INTR_BIT     0x04
#define VPU_BLOCK_BIT    0x01
#define VPU_SYS_BITS     0x06
#define VPU_SYSRET_BIT   0x01
#define VPU_EVENT_BIT    0x08
#define VPU_SEND_BIT     0x01
/* instructions */
#define VPU_BOOT         0x00 // execute bootstrap code from ROM
#define VPU_RST          VPU_RESET_BIT // 0x01
#define VPU_TSET         VPU_TRAP_BIT // 0x02
#define VPU_TCLR         (VPU_TRAP_BIT | VPU_RESET_BIT) // 0x03
#define VPU_STI          VPU_INTR_BIT // 0x04
#define VPU_CLI          (VPU_INTR_BIT | VPU_BLOCK_BIT) // 0x05
#define VPU_SCALL        VPU_SYS_BITS // 0x06
#define VPU_SRET         (VPU_SYS_BITS | VPU_SYSRET_BIT) // 0x07
/* optional instructions; HAVE_SYS_EXTRA */
#define VPU_WFE          VPU_EVENT_BIT // 0x08
#define VPU_SEV          (VPU_EVENT_BIT | VPU_SEND_BIT) // 0x09
#define VPU_WFI          (VPU_EVENT_BIT | VPU_TRAP_BIT) // 0x0a
#define VPU_SMI          (VPU_EVENT_BIT | VPU_TRAP_BIT | VPU_SEND_BIT) // 0x0b

/* OPTIONAL UNITS */

/* bitfields are parts of registers */

/* bitfield operations; HAVE_BITFLD_UNIT */
#define VPU_BITFLD_UNIT  0x0a
/* shift and rotate operations */
#define VPU_BFCPY_BIT    0x04
#define VPU_BFLOAD_BIT   0x01
#define VPU_BFSTORE_BIT  0x02
/* optional */
#define VPU_BFEXTRA_BIT  0x08
/* instructions */
#define VPU_BFNOT        0x00
#define VPU_BFAND        0x01
#define VPU_BFOR         0x02
#define VPU_BFXOR        0x03
#define VPU_BFCPY        VPU_BFCPY_BIT // 0x04
#define VPU_BFLDR_BIT    (VPU_BFCPY_BIT | VPU_BFLOAD_BIT) // 0x05
#define VPU_BFSTR_BIT    (VPU_BFCPY_BIT | VPU_BFSTORE_BIT) // 0x06

/* memory-management unit; HAVE_MEM_UNIT */
#define VPU_MEM_UNIT     0x0b
/* page/memory flags */
#define VPU_MEMEXEC_BIT  0x001 // execute permission
#define VPU_MEMWRITE_BIT 0x002 // write permission
#define VPU_MEMREAD_BIT  0x004 // read permission
#define VPU_MEMSYS_BIT   0x008 // system page
#define VPU_MEMGLOB_BIT  0x010 // global page (shared)
#define VPU_MEMWIRE_BIT  0x020 // wired page, i.e. stick in RAM
#define VPU_MEMNOC_BIT   0x040 // page not cached
#define VPU_MEMWRT_BIT   0x080 // page cached write-through (vs. write-back)
#define VPU_MEMWRC_BIT   0x100 // combine write operations for page
#define VPU_MEMPRES_BIT  0x200 // page present in RAM
#define VPU_MEMDIRTY_BIT 0x400 // dirty page (has been written to)
#define VPU_MEMBUSY_BIT  0x800 // page is locked/busy (e.g. being configured)
/* page operations */
#define VPU_PGSETATR_BIT 0x01 // set page attribute-bits
#define VPU_PGCLRATR_BIT 0x02 // clear page attribute-bits
#define VPU_PGINV_BITS   0x03 // invalidate page TLB-entry
/* optional operations */
/* cacheline operations */
#define VPU_CACHE_BIT    0x04
#define VPU_CACHELK_BIT  0x01 // lock cacheline
#define VPU_CACHEREL_BIT 0x02 // release/unlock cacheline
/* thread-local operations */
#define VPU_TLS_BIT      0x08
#define VPU_TLOAD_BIT    0x01 // thread-local memory to register
#define VPU_TSTORE_BIT   0x02 // register to thread-local memory
#define VPU_TUMAP_BITS   0x03 // unmap thread-local page
/* instructions */
#define VPU_PGINIT       0x00 // args: vmadr, flg
#define VPU_PGSETF       VPU_PGSETATR_BIT // args: vmadr, flg
#define VPU_PGCLRF       VPU_PGCLRATR_BIT // args: vmadr, flg
#define VPU_PGINV        VPU_PGINV_BITS // args: vmadr
/* optional operations; HAVE_CACHE_EXTRA */
#define VPU_CFTC         VPU_CACHE_BIT // args: cladr (prefetch)
#define VPU_CLK          (VPU_CACHE_BIT | VPU_CACHELK_BIT) // args: cladr (lock)
#define VPU_CREL         (VPU_CACHE_BIT | VPU_CACHEREL_BIT) // args: cladr
// NOTE: 0x07 is reserved
/* optional operations; HAVE_TLS_EXTRA */
#define VPU_TPGINIT      VPU_TLS_BIT // args: flg, if PRES then physadr
#define VPU_TLDR         (VPU_TLS_BIT | VPU_TLOAD_BIT) // args: ofs, reg
#define VPU_TSTR         (VPU_TLS_BIT | VPU_TSTORE_BIT) // args: reg, ofs
#define VPU_TPGUMAP      (VPU_TLS_BIT | VPU_TUMAP_BITS) // args: vmadr

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
#define VPU_TASK_UNIT    0x0c
#define VPU_TCONFIG_BIT  0x01
#define VPU_TCTX_BIT     0x02
#define VPU_TSAVE_BIT    0x01
#define VPU_TPROF_BIT    0x04
#define VPU_TPINFO_BIT   0x01
#define VPU_TPSAVE_BIT   0x01
#define VPU_TPSTOP_BIT   0x02
#define VPU_TCTL_BIT     0x08
#define VPU_TSWITCH_BIT  0x01
#define VPU_TSTOP_BIT    0x01
#define VPU_TKILL_BIT    0x01
#define VPU_STOP_BIT     0x02
#define VPU_TSIG_BIT     0x04
#define VPU_TEVENT_BIT   0x04
#define VPU_TSEND_BIT    0x01
#define VPU_TRECV_BIT    0x02
/* instructions */
#define VPU_TINIT        0x00
#define VPU_TCONF        VPU_TCONFIG_BIT // 0x01
#define VPU_TLOAD        VPU_TCTX_BIT // 0x02
#define VPU_TSAVE        (VPU_TCTX_BIT | VPU_TSAVE_BIT) // 0x03
#define VPU_TPROF        VPU_TPROF_BIT // 0x04
#define VPU_TPREAD       (VPU_TPROF_BIT | VPU_TPINFO_BIT) // 0x05
#define VPU_TPSTOP       (VPU_TPROF_BIT | VPU_TPSTOP_BIT) // 0x06
#define VPU_TPHOLD       (VPU_TPROF_BIT | VPU_TPSTOP_BIT | VPU_TPSAVE_BIT) // 07
/* optional instructions; HAVE_TCTL_EXTRA */
#define VPU_TYIELD       VPU_TCTL_BIT // 0x08
#define VPU_TSWTCH       (VPU_TCTL_BIT | VPU_TSWITCH_BIT) // 0x09
#define VPU_TSTOP        (VPU_TCTL_BIT | VPU_TSTOP_BIT) // 0x0a
#define VPU_TKILL        (VPU_TCTL_BIT | VPU_TSTOP_BIT | VPU_TKILL_BIT) // 0x0b
#define VPU_TSIG         (VPU_TCTL_BIT | VPU_TSIG_BIT) // 0x0c
#define VPU_TSEV         (VPU_TCTL_BIT | VPU_TEVENT_BIT | VPU_TSEND_BIT) // 0x0d
#define VPU_TREV         (VPU_TCTL_BIT | VPU_TEVENT_BIT | VPU_TRECV_BIT) // 0x0e
#define VPU_TRET         0x0f // return from task; exit if toplevel

/* inter-processor atomic operations (bus locked); HAVE_ATOM_UNIT */
#define VPU_ATOM_UNIT    0x0d
#define VPU_SET_BIT      0x01
#define VPU_FLIP_BIT     0x02
#define VPU_LLSC_BITS    0x03
#define VPU_FADD_BIT     0x04
#define VPU_FSUB_BIT     0x01
#define VPU_FINC_BIT     0x02
#define VPU_FDEC_BITS    (VPU_FADD_BIT | 0x03)
#define VPU_FETCH_BIT    0x08
#define VPU_COMPARE_BIT  0x02
#define VPU_CSWAP_BIT    0x04
#define VPU_CSWAP2_BIT   0x01
#define VPU_CLR          0x00
#define VPU_SET          VPU_SET_BIT // 0x01
#define VPU_CHG          VPU_FLIP_BIT // 0x02
#define VPU_LLSC         VPU_LLSC_BITS // 0x03
#define VPU_FADD         VPU_FADD_BIT // 0x04
#define VPU_FSUB         (VPU_FADD_BIT | VPU_FSUB_BIT) // 0x05
#define VPU_FINC         (VPU_FADD_BIT | VPU_FINC_BIT) // 0x06
#define VPU_FDEC         VPU_FDEC_BITS // 0x07
#define VPU_BTAC         VPU_FETCH_BIT // 0x08
#define VPU_BTAS         (VPU_FETCH_BIT | VPU_SET_BIT) // 0x09
#define VPU_BCCLR        (VPU_FETCH_BIT | VPU_COMPARE_BIT) // 0x0a
#define VPU_BCSET        (VPU_FETCH_BIT | VPU_COMPARE_BIT | VPU_SET_BIT) // 0b
#define VPU_CAS          (VPU_FETCH_BIT | VPU_SWAP_BIT) // 0x0c
#define VPU_CAS2         (VPU_FETCH_BIT | VPU_SWAP_BIT | VPU_CSWAP2_BIT) // 0x0d

/* extension operations; instructions identified by the imm8-field in opcodes */
#define VPU_EXT_UNIT     0x0e

/* extension instructions; support vectors with 8- and 16-bit vector items */
/*
 * - vector size: 32 << arg1sz
 * - item size: 8 << arg2sz
 */
/* logical operations */
#define VPU_VNOT         0x00
#define VPU_VAND         0x01
#define VPU_VOR          0x02
#define VPU_VXOR
/* multiplier and divider operations */
/* multiplier operations */
#define VPU_VMUL_BIT     0x20
#define VPU_VSIGNED_BIT  0x01
#define VPU_VMULHI_BIT   0x02
#define VPU_VMULW_BIT    0x04
#define VPU_VMUL         (VPU_VMUL_BIT) // 0x20
#define VPU_VMULS        (VPU_VMUL_BIT | VPU_VSIGNED_BIT) // 0x21
#define VPU_VMULHI       (VPU_VMUL_BIT | VPU_VMULHI_BIT) // 0x22
#define VPU_VMULSHI      (VPU_VMUL_BIT | VPU_VMULHI_BIT | VPU_VSIGNED_BIT) // 23
#define VPU_VMULW        (VPU_VMUL_BIT | VPU_VMULW_BIT) // 0x24
#define VPU_VMULSW       (VPU_VMUL_BIT | VPU_VSIGNED_BIT) // 0x25
/* divider operations */
#define VPU_VDIV_BIT     0x08
#define VPU_VREM_BIT     0x01
#define VPU_VREC_BIT     0x02
#define VPU_VCREC_BIT    0x01
#define VPU_VDIV         (0x20 | VPU_VDIV_BIT) // 0x28
#define VPU_VREM         (0x20 | VPU_VDIV_BIT | VPU_VREM_BIT) // 0x29
#define VPU_VDREC        (0x20 | VPU_VDIV_BIT | VPU_VREC_BIT) // 0x2a
// 0x2b
#define VPU_VCREC        (0x23 | VPU_DIV_BIT | VPU_VREC_BIT | VPU_VCREC_BIT)
/* skip ["branch"] operations */
#define VPU_VSKIPZ       0x30
#define VPU_VSKIPNZ      0x31
#define VPU_VSKIPE       VPU_VSKIPZ
#define VPU_VSKIPNE      VPU_VSKIPNZ
#define VPU_VSKIPO       0x32
#define VPU_VSKIPNO      0x33
#define VPU_VSKIPC       0x34
#define VPU_VSKIPNC      0x35
#define VPU_VSKIPLT      0x36
#define VPU_VSKIPLE      0x37
#define VPU_VSKIPGT      0x38
#define VPU_VSKIPGE      0x39
/* shifter operations */
#define VPU_VSHIFT_BIT   0x40 // count in low 4 bits
#define VPU_VROT_BITS    0x50 // count in low 4 bits
#define VPU_VRIGHT_BIT   0x01
#define VPU_VSIGNEXT_BIT 0x02
#define VPU_VCARRY_BIT   0x02
#define VPU_VSHL         VPU_VSHIFT_BIT // 0x40
#define VPU_VSHL         (VPU_VSHIFT_BIT | VPU_VRIGHT_BIT) // 0x41
#define VPU_VSAR         (VPU_VSHIFT_BIT | VPU_VSIGNEXT_BIT) // 0x42
#define VPU_VROL         VPU_VROT_BITS // 0x50
#define VPU_VROR         (VPU_VROT_BITS | VPU_VRIGHT_BIT) // 0x51
#define VPU_RCL          (VPU_VROT_BITS | VPU_VCARRY_BIT) // 0x52
#define VPU_VRCR         (VPU_VROT_BITS | VPU_VCARRY_BIT | VPU_VRIGHT_BIT) // 53
/* miscellaneous operations */
#define VPU_VUNPK        0xfe
#define VPU_VPK          0xff

/* processor parameters */
#define VPU_COPROC_UNIT  0x0f // coprocessor; FPU, VEC, ...
#define VPU_NALU_MAX     256 // max number of ALU operations

#endif /* __VPU_ISA_H__ */

