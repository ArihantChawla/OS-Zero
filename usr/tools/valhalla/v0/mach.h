#ifndef __VPU_V0_MACH_H__
#define __VPU_V0_MACH_H__

#include <stdint.h>

#define V0_NINST_MAX 256

/* Valhalla 0 (V0) processor machine interface */

/*
 * Instruction Prefixes
 * --------------------
 *
 * 0xff    - memory bus lock
 * The MLK-prefix may be used to lock the memory bus for a single operation of
 * - INC, DEC, ADD, SDC, SUB, SBB, NOT, AND, XOR, OR, BTR, BTS, BTC, LDR, STR
 *
 */

/* bits for MFR feature register */

/* TODO: integrate FPU-operations into the basic instruction set */

#define V0_MFR_MMU (1U << 0) // memory management unit present
#define V0_MFR_FPU (1U << 1) // floating-point processor present
#define V0_MFR_GPU (1U << 2) // graphics processor present
#define V0_MFR_DSP (1U << 3) // digital signal processor present
#define V0_MFR_FPM (1U << 4) // fixed-point processor present

/*
 * V0 Instruction Set
 * ------------------
 *
 * Argument Descriptions
 * ---------------------
 * r    register                vm->regs[op->reg]
 * i	immediate               op->val
 * d    immediate               op->arg[0].i32
 * m    memory [address]	v0getarg1(vm, op), v0getarg2(vm, op)
 */

/*
 * ARITHMETIC-LOGICAL OPERATIONS
 * -----------------------------
 *
 * Instructions
 * ------------
 *
 * Mnemo Opcode  Src    Dest	Brief
 * ----- ------  ---    ----    -----
 * INC	 0x01           r       increment by one
 * DEC   0x02           r       decrement by one
 * CMP   0x03    ri     r	compare (subtract + set MSW-flags)
 * ADD   0x04    rid	r	addition (ignore over and underflows)
 * ADC   0x05    rid    r       addition with carry-flag
 * SUB   0x06    rid	r	subtract; ignore underflow
 * SBB   0x07    rid    r       subtract with carry-flag
 * SHR   0x0a    rid    r       shift right logical (fill with zero)
 * SAR   0x0b    rid    r       shift right arithmetic (fill with sign-bit)
 * NOT   0x0c           r       bitwise inverse
 * AND   0x0d    rid    r       logical AND
 * XOR   0x0e    rid    r       logical XOR
 * LOR   0x0f    rid    r       logical OR
 *
 * Opcode Notes
 * ------------
 * ADD/ADC, SUB/SBB     - 0x01 denotes carry/borrow-bit, 0x03 mean set flags
 * SHL      SHR/SAR     - 0x01 denotes arithmetic right shift, 0x02 is right
 *
 * Macros
 * ------
 */
#define V0_INC 0x01 //   r     increment by one
#define V0_DEC 0x02 //   r     decrement by one
#define V0_CMP 0x03 //   r, r   compare (subtract + set flags)
#define V0_ADD 0x04 //   ri, r   addition
#define V0_ADC 0x05 //   ri, r   addition with carry
#define V0_SUB 0x06 //   ri, r   subtraction
#define V0_SBB 0x07 //   ri, r   subraction with borrow
#define V0_SHL 0x08 //   ri, r   shift left logical
#define V0_SHR 0x0a //   ri, r   shift right logical
#define V0_SAR 0x0b //   ri, r   shift right arithmetic
#define V0_NOT 0x0c //   r       reverse all bits
#define V0_AND 0x0d //   ri, r   logical AND
#define V0_XOR 0x0e //   ri, r   logical exclusive OR
#define V0_LOR 0x0f //   ri, r   logical OR

/*
 * MULTIPLICATION AND DIVISION
 * ---------------------------
 *
 * Notes
 * -----
 * - reciprocals as well as results are stored in [64-bit] registers
 * - division is done by computing a reciprocal and multiplying the dividend
 *   with it EDIT: IF feasible... This needs to be done with floating-point
 *   operations.
 *
 * Instructions
 * ------------
 *
 * Mnemo Opcode  Src    Dest	Brief
 * ----- ------  ---    ----    -----
 * CRP   0x10    rid    r       calculate reciprocal
 * MUL   0x11    rid    r       multiply and return low word
 * MUH   0x12    rid    r       multiply and return high word
 *
 * Opcode Notes
 * ------------
 * MUL/MUH		- 0x2 denotes high word return
 */
#define V0_CRP   0x10 //  ri, r   Multiplication
#define V0_MUL   0x11 //  ri, r   Multiplication, get low word of results
#define V0_MUH   0x12 //  ri, r   Multiplication, get high word of results

/*
 * BIT OPERATIONS
 * --------------
 *
 * Notes
 * -----
 * - the instructions BTS, BTC, BSH, BSW, BSL, BSQ in this section are optional;
 *   indicated with the BO-bit in MFW.
 *
 * Instructions
 * ------------
 *
 * Mnemo   Opcode  Src	Dest	Brief
 * -----   ------  ---  ----    -----
 * HAM	   0x13    rid	r	Hamming weight/bit population (count of 1-bits)
 * CLZ     0x14    rid  r       count leading zero bits
 * BTR     0x15    rid  r       bit test and reset; original bit in VF
 * BTS     0x16    rid  r       bit test and set; original bit in VF
 * BTC     0x17    rid  r       bit test and complement; original bit in VF
 * SWP     0x18    rid  r       byteswap
 *
 * Opcode Notes
 * ------------
 * BTR/BTS/BTC          - bits 0-1: 0 - reset, 1 - set, 2 - complement
 *                      - store original value in RF
 * SWP                  - parm - size shift count
 * - type width in bits: b - 8, h - 16, w - 32, q - 64, o - 128, h - 256
 */
#define V0_HAM 0x13
#define V0_CLZ 0x14
#define V0_BTR 0x15
#define V0_BTS 0x16
#define V0_BTC 0x17
#define V0_SWP 0x18

/*
 * LOAD-STORE OPERATIONS
 * ---------------------
 *
 * Instructions
 * ------------
 *
 * Mnemo   Opcode  Src	Dest	Brief
 * -----   ------  ---  ----    -----
 * LDR     0x20    rid  r       load register
 * STR     0x21    rid  rm      store register
 * PSH     0x22    rid  rm      store register
 * PSM     0x23    rid  rm      store register
 * POP     0x24    rid  rm      store register
 * POM     0x25    rid  rm      store register
 *
 * Opcode Notes
 * ------------
 * LDR/STR              - 0x01 denotes memory write, parm - size shift count
 * - type width in bits: b - 8, h - 16, w - 32, q - 64, o - 128, h - 256
 * PSH/PSM, POP/POM     - 0x01 denotes many-register operation
 *                      - dest is a register-bitmap (v0_R0_REG..v0_NREGS)
 */
#define V0_LDR 0x20
#define V0_STR 0x21
#define V0_PSH 0x22
#define V0_PSM 0x23
#define V0_POP 0x24
#define V0_POM 0x25

/*
 * BRANCH OPERATIONS
 * -----------------
 *
 * Instructions
 * ------------
 * Mnemo   Opcode  Src	Dest	Brief                           Arguments
 * -----   ------  ---  ----    -----                           ---------
 * JMP     0x30         rdm     jump; branch unconditionally
 * JMR     0x31         rdm     short jump
 * BIZ     0x32         rdm     branch if zero
 * BEQ     0x32         rdm     synonym for BIZ
 * BNZ     0x33         rdm     branch if non-zero
 * BNE     0x33         rdm     synonym for BNZ
 * BLT     0x34         rdm     branch if less than
 * BLE     0x35         rdm     branch if less than or equal
 * BGT     0x36         rdm     branch if greater than
 * BGE     0x37         rdm     branch if greater than or equal
 * BIO     0x38         rdm     branch if overflow
 * BNO     0x39         rdm     branch if no overflow
 * BIC     0x3a         rdm     branch if carry
 * BNC     0x3b         rdm     branch if no carry
 * BEG     0x3c    ri   rdm     function prologue; adjust stack
 * FIN     0x3d    ri   rdm     function epilogue; adjust stack
 * RET     0x3e         rdm     return from subroutine
 * RTN     0x3f    ri   rdm     return from subroutine with stack-increment
 *
 * Opcode Notes
 * ------------
 * JMP/JMR		- 0x01 denotes relative jump
 * BZ/BNZ, BLT/BLE,
 * BGT/BGE, BO/BNO,
 * BC/BNC               - 0x01 denotes zero-flag condition
 *
 */
#define V0_JMP 0x30
#define V0_JMR 0x31
#define V0_BIZ 0x32 //   r, m    branch if zero
#define V0_BEQ 0x32 // same as BIZ
#define V0_BNZ 0x33
#define V0_BNE 0x33 // same as BNZ
#define V0_BLT 0x34 //   r, m    branch if less than
#define V0_BLE 0x35 //   r, m    branch if less or equal
#define V0_BGT 0x36 //   r, m    branch if greater than
#define V0_BGE 0x37 //   r, m    branch if greater or equal
#define V0_BIO 0x38 //   r, m    branch if overflow
#define V0_BNO 0x39 //   r, m    branch if no overflow
#define V0_BIC 0x3a //   r, m    branch if carry/borrow
#define V0_BNC 0x3b //   r, m    branch if no carry/borrow
#define V0_CSR 0x3c
#define V0_BEG 0x3d //   ridm    subroutine prologue
#define V0_FIN 0x3e //   ridm    prepare for return from subroutine
#define V0_RET 0x3f //   NONE    return from subroutine    stack byte count

/* INPUT-OUTPUT OPERATIONS
 * -----------------------
 *
 * Opcode Notes
 * ------------
 * V0_IRD       - bits 0-2 of parm are shift count for operation size
 * V0_IWR       - bits 0-2 of parm are shift count for operation size
 *
 * Mnemo   Opcode  S & D   Brief                           Arguments
 * -----   ------  -----   -----                           ---------
 */
#define V0_IRD 0x40 // read
#define V0_IWR 0x41 // write
#define V0_ICF 0x42 // configure I/O; set/clear user-permissions, ...

/* MULTIPROCESSOR OPERATIONS
 * -------------------------
 *
 * NOTE: the instructions in this section are optional; indicated with MP-bit in
 * MFW.
 *
 * Mnemo   Opcode  S & D           Brief                           Arguments
 * -----   ------  -----           -----                           ---------
 */
#define V0_WFI 0x50 //   NONE            wait for event (e.g. interrupt)
#define V0_SEV 0x51 //   NONE            signal event

/* Memory Operations
 * -----------------
 *
 * Opcode Notes
 * ------------
 * RDB, WRB, FMB        - bits 0-1: 0 - read, 1 - write, 2 - both
 * LCK, LDL, STC, ULK	- bits 0-1: 0 - lock, 1 - load, 2 - store, 3 - release
 *
 *
 * Mnemo   Opcode  S & D           Brief                           Arguments
 * -----   ------  -----           -----                           ---------
 */
#define V0_BAR 0x60 //   NONE            full memory barrier
#define V0_BRD 0x61 //   NONE            memory read barrier
#define V0_BWR 0x62 //   NONE            memory write barrier
#define V0_CPF 0x63 //   m               cache prefetch            byte address
#define V0_CLK 0x64 //   m               cacheline lock            byte address
#define V0_ULK 0x65 //   m               cacheline unlock          byte address
#define V0_CLL 0x66 //   m               linked load
#define V0_CST 0x67 //   ri, m           conditional store
#define V0_FPG 0x68 //   m               flush TLB entry           byte address
#define V0_FLS 0x69 //   m               flush cachelines
#define V0_CAS 0x6a //   m, ri, ri       compare and swap
#define V0_CS2 0x6b //   m, ri, ri       double-word compare and swap

/* SYSTEM OPERATIONS
 * -----------------
 *
 * Mnemo   Opcode  S & D   Brief                           Arguments
 * -----   ------  -----   -----                           ---------
 */
#define V0_NOP 0xff
#define V0_HLT 0xfe //   NONE    halt processor
#define V0_RST 0xfd //   NONE    reset processor

#endif /* __VPU_V0_MACH_H__ */

