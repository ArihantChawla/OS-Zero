#ifndef __ZPM_ZPM_H__
#define __ZPM_ZPM_H__

/* INSTRUCTION SET */

#define ZPM_OP_NOP     0x00 // dummy operation
/* logical operations */
#define ZPM_OP_LOGIC   0x00
#define ZPM_OP_NOT     0x01 // 2's complement
#define ZPM_OP_AND     0x02 // logical AND
#define ZPM_OP_OR      0x03 // logical OR
#define ZPM_OP_XOR     0x04 // logical exclusive OR
/* shift operations */
#define ZPM_OP_SHIFT   0x01
#define ZPM_OP_SHL     0x01 // shift left (fill with zero)
#define ZPM_OP_SHR     0x02 // logical shift right (fill with zero)
#define ZPM_OP_SAR     0x03 // arithmetic shift right (fill with sign)
#define ZPM_OP_ROR     0x04 // rotate right
#define ZPM_OP_ROL     0x05 // rotate left
/* arithmetic operations */
#define ZPM_OP_ARITH   0x02
#define ZPM_OP_INC     0x01 // increment by one
#define ZPM_OP_DEC     0x02 // decrement by one
#define ZPM_OP_ADD     0x03 // addition
#define ZPM_OP_SUB     0x04 // subtraction
#define ZPM_OP_CMP     0x05 // compare
#define ZPM_OP_MUL     0x06 // multiplication
#define ZPM_OP_DIV     0x07 // division
#define ZPM_OP_MOD     0x08 // modulus
/* branch instructions */
#define ZPM_OP_BRANCH  0x03
#define ZPM_OP_JMP     0x01 // unconditional jump to given address
#define ZPM_OP_BZ      0x02 // branch if zero
#define ZPM_OP_BNZ     0x03 // branch if not zero
#define ZPM_OP_BLT     0x04 // branch if less than
#define ZPM_OP_BLE     0x05 // branch if less than or equal to
#define ZPM_OP_BGT     0x06 // branch if greater than
#define ZPM_OP_BGE     0x07 // branch if greater than or equal to
#define ZPM_OP_BO      0x08 // branch if overflow
#define ZPM_OP_BNO     0x09 // branch if no overflow
#define ZPM_OP_BC      0x0a // branch if carry
#define ZPM_OP_BNC     0x0b // branch if no carry
/* stack operations */
#define ZPM_OP_STACK   0x04
#define ZPM_OP_POP     0x01 // pop from stack
#define ZPM_OP_PUSH    0x02 // push to stack
#define ZPM_OP_PUSHA   0x03 // push all registers to stack
/* load-store */
#define ZPM_OP_MEM     0x05
#define ZPM_OP_LDR     0x01
#define ZPM_OP_STR     0x02
/* function calls */
#define ZPM_OP_FUNC    0x06
#define ZPM_OP_CALL    0x01 // call subroutine
#define ZPM_OP_ENTER   0x02 // subroutine prologue
#define ZPM_OP_LEAVE   0x03 // subroutine epilogue
#define ZPM_OP_RET     0x04 // return from subroutine
#define ZPM_OP_THR     0x05 // launch a new thread
/* system operations */
#define ZPM_OP_SYS     0x07
#define ZPM_OP_LMSW    0x01 // load machine status word
#define ZPM_OP_SMSW    0x02 // store machine status word
/* machine state */
#define ZPM_OP_MACH    0x08
#define ZPM_OP_RESET   0x01 // reset into well-known state
#define ZPM_OP_HLT     0x02 // halt execution
/* I/O */
#define ZPM_OP_IO      0x09
#define ZVM_OP_IN      0x01
#define ZVM_OP_OUT     0x02

/* MACHINE DESCRIPTION */

/* general purpose registers */
#define ZPM_EAX_REG    0 // first function argument, function return value
#define ZPM_EDX_REG    1 // second function argument
#define ZPM_ECX_REG    2 // third function argument
#define ZPM_EBX_REG    3
#define ZPM_EDI_REG    4
#define ZPM_ESI_REG    5
#define ZPM_EBP_REG    6 // frame pointer register
#define ZPM_ESP_REG    7 // stack pointer register
#define ZPM_NGENREG    8
/* special registers */
#define ZPM_EIP_REG    0 // program counter / instruction pointer register
#define ZPM_EFLAGS_REG 1 // machine status word register
#define ZPM_NEXTRA_REG 8
/* FPU registers */
#define ZPM_F0_REG     0
#define ZPM_F1_REG     1
#define ZPM_F2_REG     2
#define ZPM_F3_REG     3
#define ZPM_F4_REG     4
#define ZPM_F5_REG     5
#define ZPM_F6_REG     6
#define ZPM_F7_REG     7
#define ZPM_NFPUREG    8

/* macros for accessing virtual machine members */
#define eax_reg        genregs[ZPM_EAX_REG]
#define edx_reg        genregs[ZPM_EDX_REG]
#define ecx_reg        genregs[ZPM_ECX_REG]
#define ebx_reg        genregs[ZPM_EBX_REG]
#define edi_reg        genregs[ZPM_EDI_REG]
#define esi_reg        genregs[ZPM_ESI_REG]
#define ebp_reg        genregs[ZPM_EBP_REG]
#define esp_reg        genregs[ZPM_ESP_REG]
#define fp_reg         genregs[ZPM_EBP_REG]
#define sp_reg         genregs[ZPM_ESP_REG]
#define pc_reg         xtraregs[ZPM_EIP_REG]
#define msw_reg        xtragregs[ZPME_EFLAGS_REG]
struct zpm {
    double   fpuregs[ZPM_NFPUREG];
    int32_t  genregs[ZPM_NGENREG];      // general purpose register context
    int32_t  xtraregs[ZPM_NEXTRAREG];   // special register context
    size_t   pmemsize;                  // size of "physical" memory in bytes
    int8_t  *physmem;                   // dynamic "physical" memory
};

/* MACHINE INTERFACE */

#endif /* __ZPM_ZPM_H__ */

