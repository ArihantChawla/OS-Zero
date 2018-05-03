#ifndef __VPU_V0_MACH_H__
#define __VPU_V0_MACH_H__

#include <stdint.h>

#define V0_NINST_MAX 256

/* Valhalla 0 (V0) processor machine interface */

/*
 * V0 Instruction Set
 * ------------------
 *
 * Argument Descriptions
 * ---------------------
 * r    register                vm->regs[op->reg]
 * i	immediate               op->val
 * d    immediate               op->arg[0].i32 etc.
 * m    memory [address]	v0getarg1(vm, op), v0getarg2(vm, op)
 *
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
 * CMP   0x03    rid    r	compare (subtract + set MSW-flags)
 * ADD   0x04    rid	r	addition (ignore over and underflows)
 * ADC   0x05    rid    r       addition with carry-flag
 * SUB   0x06    rid	r	subtract; ignore underflow
 * SBB   0x07    rid    r       subtract with carry-flag
 * SHL   0x08    ri     r       shift left [logical/fill with zero]
 * SHR   0x0a    ri     r       shift right logical (fill with zero)
 * SAR   0x0b    ri     r       shift right arithmetic (fill with sign-bit)
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
 * CLZ	   0x13    rid	r	Hamming weight/bit population (count of 1-bits)
 * HAM     0x14    rid  r       count leading zero bits
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
#define V0_CLZ 0x13
#define V0_HAM 0x14
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
 * STx     0x22    rid  rm      store register with sign-extension
 * PSH     0x23    rid  rm      push register
 * PSM     0x24    rid  rm      push many registers
 * POP     0x25    rid  rm      pop register
 * POM     0x26    rid  rm      pop many registers
 *
 * Opcode Notes
 * ------------
 * LDR/STR/STX          - 0x01 denotes memory write, 0x02 means sign-extend,
 *                        parm - size shift count
 * - type width in bits: b - 8, h - 16, w - 32, q - 64, o - 128, h - 256
 * - corresponding parm: b - 0, h - 1, w - 2, q - 3, o - 4, h - 5
 * PSH/PSM, POP/POM     - 0x02 denotes many-register operation
 *                      - dest is a register-bitmap (v0_R0_REG..v0_NREGS)
 */
#define V0_LDR 0x20
#define V0_STR 0x21
#define V0_STX 0x22
#define V0_PSH 0x23
#define V0_POP 0x24
#define V0_PSM 0x25
#define V0_POM 0x26

/*
 * BRANCH OPERATIONS
 * -----------------
 *
 * Instructions
 * ------------
 *
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
#define V0_BIZ 0x32
#define V0_BEQ 0x32
#define V0_BNZ 0x33
#define V0_BNE 0x33
#define V0_BLT 0x34
#define V0_BLE 0x35
#define V0_BGT 0x36
#define V0_BGE 0x37
#define V0_BIO 0x38
#define V0_BNO 0x39
#define V0_BIC 0x3a
#define V0_BNC 0x3b
#define V0_CSR 0x3c
#define V0_BEG 0x3d
#define V0_FIN 0x3e
#define V0_RET 0x3f

/* INPUT-OUTPUT OPERATIONS
 * -----------------------
 *
 * Instructions
 * ------------
 *
 * Mnemo   Opcode  Src	Dest	Brief                           Arguments
 * -----   ------  ---  ----    -----                           ---------
 * IRD     0x40    rd   r       read from I/O port              port, source
 * IWR     0x41    r    rd      write to I/O port               port, source
 * ICF     0x42    r    rd      configure I/O port              port, value
 *
 * Opcode Notes
 * ------------
 * V0_IRD       - bits 0-3 of parm are shift count for operation size
 * V0_IWR       - bits 0-3 of parm are shift count for operation size
 */
#define V0_IRD 0x40
#define V0_IWR 0x41
#define V0_ICF 0x42

/* MULTIPROCESSOR OPERATIONS
 * -------------------------
 *
 * NOTE: the instructions in this section are optional; indicated with MP-bit in
 * MFW.
 *
 * Mnemo   Opcode  Src  Dest        Brief                           Arguments
 * -----   ------  ---  ----        -----                           ---------
 */
#define V0_WFI 0x50 //   NONE            wait for event (e.g. interrupt)
#define V0_SEV 0x51 //   NONE            signal event

/* Memory Management & Synchronisation
 * -----------------------------------
 *
 * Instructions
 * ------------
 *
 * Mnemo   Opcode  Src  Dest        Brief                           Arguments
 * -----   ------  ---  ----        -----                           ---------
 * BRD     0x60                     full memory barrier
 * BWR     0x61                     memory read barrier
 * BAR     0x62                     memory write barrier
 * CPF     0x63    m                cache prefetch
 * CUL     0x64    m                cacheline unlock
 * CLK     0x65    m                cacheline lock
 * CLL     0x66    m    r           load linked
 * CST     0x67    r    m           store conditional (if cacheline clean)
 * FPG     0x68    m                flush memory TLB-entry
 * FLS     0x69    m    ri          flush cachelines
 * CAS     0x6a    r, d, m         compare and swap                val/want/adr
 * CS2     0x6a    r, d, m         dual-word compare and swap      val/want/adr
 *
 * Opcode Notes
 * ------------
 * BAR, BRD, BWR        - bits 0-1: 0 - read, 1 - write, 2 - both
 * CLK/CUL/CLL/CST      - 0x04 - locked operation
 *                        - 0x00 unlock, 0x01 lock, 0x02 load, 0x03 store
 */
#define V0_BAR 0x60
#define V0_BRD 0x61
#define V0_BWR 0x62
#define V0_CPF 0x63
#define V0_CUL 0x64
#define V0_CLK 0x65
#define V0_CLL 0x66
#define V0_CST 0x67
#define V0_FPG 0x68
#define V0_FLS 0x69
#define V0_CAS 0x6a
#define V0_CS2 0x6b

/* SYSTEM OPERATIONS
 * -----------------
 *
 * Mnemo   Opcode  S & D   Brief                           Arguments
 * -----   ------  -----   -----                           ---------
 */
#define V0_NOP 0xff
#define V0_HLT 0xfe
#define V0_RST 0xfd

#endif /* __VPU_V0_MACH_H__ */

