/* processor sub-units */
#define V0_LOGIC  0x00
#define V0_SHIFT  0x01
#define V0_ARITH  0x02
#define V0_FLOW   0x03
#define V0_XFER   0x04
#define V0_STACK  0x05
#define V0_IO     0x06
#define V0_COPROC 0x0f

/* sub-unit instructions */

/* V0_LOGIC */
#define V0_NOT    0x00
#define V0_AND    0x01
#define V0_OR     0x02
#define V0_XOR    0x03

/* V0_SHIFT */
#define V0_SHL    0x00
#define V0_SHR    0x01
#define V0_SAR    0x02

/* V0_ARITH */
#define V0_INC    0x00
#define V0_DEC    0x01
#define V0_ADD    0x02
#define V0_ADC    0x03
#define V0_SUB    0x04
#define V0_SBB    0x05
#define V0_MUL    0x06
#define V0_DIV    0x07
#define V0_REM    0x08

/* V0_FLOW */
#define V0_JMP    0x00
#define V0_CALL   0x01
#define V0_BZ     0x02
#define V0_BNZ    0x03
#define V0_BC     0x04
#define V0_BNC    0x05
#define V0_BO     0x06
#define V0_BNO    0x07
#define V0_BEQ    V0_BZ
#define V0_BLT    0x08
#define V0_BLE    0x09
#define V0_BGT    0x0a
#define V0_BGE    0x0b

/* V0_XFER */
#define V0_LDR    0x00
#define V0_STR    0x01
#define V0_LDX    0x02
#define V0_STX    0x03

/* V0_STACK */
#define V0_PSH    0x00
#define V0_POP    0x01
#define V0_PSHX   0x02
#define V0_POPX   0x03
#define V0_PSHA   0x04
#define V0_POPA   0x05

/* 32-bit instruction opcode/parcel */
struct v0op {
    unsigned int unit  : 4;  // processor sub-unit
    unsigned int inst  : 4;  // sub-unit instruction
    unsigned int reg1  : 4;  // argument register #1
    unsigned int reg2  : 4;  // argument register #2
    unsigned int val   : 12; // immediate argument such as offset
    unsigned int argsz : 2;  // argument size is 8 << argsz
    unsigned int adr   : 2;  // addressing mode for load or store
};

