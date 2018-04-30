#ifndef __VPU_V0_MACH_H__
#define __VPU_V0_MACH_H__

#include <stdint.h>

#define V0_NINST_MAX 256

/* Valhalla 0 (V0) processor machine interface */

/* bits for MFR feature register */
#define V0_MFR_MMU (1U << 0) // memory management unit present
#define V0_MFR_FPU (1U << 1) // floating-point processor present
#define V0_MFR_GPU (1U << 2) // graphics processor present
#define V0_MFR_DSP (1U << 3) // digital signal processor present
#define V0_MFR_FPM (1U << 4) // fixed-point processor present

/* V0 Instruction Set
 * ------------------
 *
 * Argument Descriptions
 * ---------------------
 * r    register                vm->regs[op->reg]
 * i	immediate               op->val
 * d    immediate               op->arg[0].i32
 * m    memory [address]	v0getarg1(vm, op), v0getarg2(vm, op)
 *
 * Notes
 * -----
 *
 * ARITHMETIC-LOGICAL OPERATIONS
 *-----------------------------
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
 *
 * Opcode Notes
 * ------------
 * ADD/ADC, SUB/SBB     - 0x01 denotes carry/borrow-bit, 0x03 mean set flags
 * SHL/SAL, SHR/SAR     - 0x01 denotes arithmetic right shift, 0x02 is right
 *
 * Macros
 * ------
 */
//#define V0_NOP 0xff //   none  no operation                possibly system ops
#define V0_INC 0x01 //   r     increment by one
#define V0_DEC 0x02 //   r     decrement by one
#define V0_CMP 0x03 //   r, r   compare (subtract + set flags)
#define V0_ADD 0x04 //   ri, r   addition
#define V0_ADC 0x05 //   ri, r   addition with carry
#define V0_SUB 0x06 //   ri, r   subtraction
#define V0_SBB 0x07 //   ri, r   subraction with borrow
#define V0_SHL 0x08 //   ri, r   shift left logical
//#define V0_SAL 0x09 //   ri, r   shift left arithmetical
#define V0_SHR 0x0a //   ri, r   shift right logical
#define V0_SAR 0x0b //   ri, r   shift right arithmetic
#define V0_NOT 0x0c //   r       reverse all bits
#define V0_AND 0x0d //   ri, r   logical AND
#define V0_XOR 0x0e //   ri, r   logical exclusive OR
#define V0_LOR 0x0f //   ri, r   logical OR

/* Multiplication and Division
 * ---------------------------
 *
 * NOTE: division is done by computing a reciprocal and multiplying the dividend
 * with it EDIT: IF feasible... This needs to be done with floating-point
 * operations.
 *
 * Mnemo   Opcode         S & D   Brief                           Arguments
 * -----   ------         -----   -----                           ---------
 */
#define V0_ARP   0x10 //  ri, r   Multiplication
#define V0_MUL   0x11 //  ri, r   Multiplication, get high word
#define V0_MUH   0x12 //  ri, r
//#define V0_CRU 0x10 //   ri, r   compute unsigned reciprocal
//#define V0_CRS 0x11 //   ri, r   compute signed reciprocal
/* TODO: MLU/MLS? */
// #define V0_MLU 0x12 //  ri, r   unsigned multiplication
//#define V0_MLS 0x13 //   ri, r   signed multiplication
//#define V0_MUL 0x12

/* Bit Operations
 * --------------
 * NOTE: The instructions in this section are optional; indicated with the BO-
 * bit in MSW.
 *
 *
 * Opcode Notes
 * ------------
 * BTR/BTS/BTC             - bits 0-1: 0 - reset, 1 - set, 2 - complement
 *                         - store original value in RF
 * BSW/BSH/BSL             - bits 0-1: 0 - 16-bit, 1 - 32-bit, 2 - 64-bit
 *
 * Mnemo   Opcode  S & D   Brief                           Arguments
 * -----   ------  -----   -----                           ---------
 */
#define V0_BTR 0x14 //    rv, m   bit test and reset              bit ID, address
#define V0_BTS 0x15 //   rv, m   bit test and set                bit ID, address
#define V0_BTC 0x16 //   rv, m   bit test and complement         bit ID, address
#define V0_BHW 0x17 //   r, r    Hamming weight / bit population
#define V0_BLZ 0x18 //   r, r    leading zero count
//#define V0_BLS 0x19 //   r, r    leading sign-bit count
#define V0_BSH 0x20 //   r, r    16-bit byte swap
#define V0_BSW 0x21 //   r, r    32-bit byte swap
#define V0_BSL 0x22 //   r, r    64-bit byte swap

/* LOAD-STORE OPERATIONS
 * ---------------------
 *
 * Opcode Notes
 * ------------
 * LDR/STR                 - 0x01 denotes memory write
 * PSH/PSM, POP/POM        - 0x01 denotes many-register operation
 *
 * Mnemo   Opcode  S & D   Brief                           Arguments
 * -----   ------  -----   -----                           ---------
 */
#define V0_LDR 0x20 //     rim, r  load register
#define V0_STR 0x21 //     ri, rm  store register
#define V0_PSH 0x22 //     ri      push register
#define V0_PSM 0x23 //     i       push many registers      register bitmap
#define V0_POP 0x24 //     r       pop register
#define V0_POM 0x25 //     i       pop many registers       register bitmap

/* BRANCH OPERATIONS
 * -----------------
 *
 * Opcode Notes
 * ------------
 * JMP                     - 0x01 denotes relative jump
 * BZ/BNZ, BLT/BLE,
 * BGT/BGE, BO/BNO,
 * BC/BNC                  - 0x01 denotes zero-flag condition
 *
 * Mnemo   Opcode  S & D   Brief                           Arguments
 * -----   ------  -----   -----                           ---------
 */
#define V0_JMP 0x30 //    rim     absolute jump                   offset or address
#define V0_JMR 0x31 //    riv, rm relative jump                   offset, register ID
#define V0_BIZ 0x32 //   r, m    branch if zero
#define V0_BEQ 0x32 //   r, m    BZ
#define V0_BNZ 0x33 //   r, m    branch if not zero
#define V0_BNE 0x33 //   r, m    BNZ
#define V0_BLT 0x34 //   r, m    branch if less than
#define V0_BLE 0x35 //   r, m    branch if less or equal
#define V0_BGT 0x36 //   r, m    branch if greater than
#define V0_BGE 0x37 //   r, m    branch if greater or equal
#define V0_BIO 0x38 //   r, m    branch if overflow
#define V0_BNO 0x39 //   r, m    branch if no overflow
#define V0_BIC 0x3a //   r, m    branch if carry/borrow
#define V0_BNC 0x3b //   r, m    branch if no carry/borrow
#define V0_RT0 0x3c //   NONE    return from subroutine          stack byte count
#define V0_RT1 0x3d //   iv      return from subroutine          stack byte count

/* INPUT-OUTPUT OPERATIONS
 * -----------------------
 *
 * Opcode Notes
 * ------------
 * RDB/RDH/RDW/RDL,
 * WDB/WDH/WDW/WDL         - bits 0-2 are shift count for operation size
 * ISR/ISW/ISU             - bits 0-2: 0 - read, 1 - write, 2 user
 * ICR/ICW/ICU             - bits 0-2: 3 - read, 4 - write, 5 user
 *
 * Mnemo   Opcode  S & D   Brief                           Arguments
 * -----   ------  -----   -----                           ---------
 */
#define V0_IRB 0x40 //    rv, r   read 1 byte                     port, destination
#define V0_IRH 0x41 //   rv, r   read 2 bytes (short)            port, destination
#define V0_IRW 0x42 //   rv, r   read 4 bytes (word)             port, destination
#define V0_IRL 0x43 //   rv, r   read 8 bytes (longword)         port, destination
#define V0_IWB 0x44 //   ri, rv  write 1 byte                    source, port
#define V0_IWH 0x45 //   ri, rv  write 2 bytes (short)           source, port
#define V0_IWW 0x46 //   ri, rv  write 4 bytes (word)            source, port
#define V0_IWL 0x47 //   ri, rv  write 8 bytes (longword)        source, port
#define V0_ISF 0x48 //   ri, rv  set port-flag                   0 or non-zero, port
#define V0_ICF 0x49 //   ri, rv  check port-flag                 0 or non-zero, port

/* SYSTEM OPERATIONS
 * -----------------
 *
 * Mnemo   Opcode  S & D   Brief                           Arguments
 * -----   ------  -----   -----                           ---------
 */
#define V0_CSR 0x50 //   rm      call subroutine                 subroutine address
#define V0_BEG 0x51 //   iv      begin subroutine                stack byte count
#define V0_FIN 0x52 //    NONE    finish subroutine
#define V0_HLT 0x53 //   NONE    halt processor
#define V0_RST 0x54 //   NONE    reset processor

/* MULTIPROCESSOR OPERATIONS
 * -------------------------
 *
 * NOTE: the instructions in this section are optional; indicated with MP-bit in
 * MFW.
 *
 * Instruction Prefixes
 * --------------------
 *
 * 0xff    - memory bus lock
 * The MLK-prefix may be used to lock the memory bus for a single operation of
 * - INC, DEC, ADD, SDC, SUB, SBB, NOT, AND, XOR, OR, BTR, BTS, BTC, LDR, STR
 *
 *
 * Mnemo   Opcode  S & D           Brief                           Arguments
 * -----   ------  -----           -----                           ---------
 */
#define V0_WFI 0x55 //   NONE            wait for event (e.g. interrupt)
#define V0_SEV 0x56 //   NONE            signal event

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
#define V0_RDB 0x60 //   NONE            memory read barrier
#define V0_WRB 0x61 //   NONE            memory write barrier
#define V0_BAR 0x62 //   NONE            full memory barrier
#define V0_CPF 0x63 //   m               cache prefetch            byte address
#define V0_FPG 0x64 //   m               flush TLB entry           byte address
#define V0_FLF 0x65 //   m               flush cachelines
#define V0_LCK 0x66 //   m               cacheline lock            byte address
#define V0_LDL 0x67 //   m               linked load
#define V0_STC 0x68 //   ri, m           conditional store
#define V0_ULK 0x69 //   m               cacheline unlock          byte address

/* Atomic Operations
 * -----------------
 *
 * Opcode Notes
 * ------------
 */

#define V0_CAS 0x6a //   m, ri, ri       compare and swap
#define V0_CS2 0x6b //   m, ri, ri       double-word compare and swap

#endif /* __VPU_V0_MACH_H__ */

