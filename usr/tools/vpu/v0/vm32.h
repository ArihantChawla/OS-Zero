#ifndef __V0_VM32_H__
#define __V0_VM32_H__

/* VIRTUAL MACHINE */

#define V0_TEXT_ADR 1024

#include <stdint.h>
#include <endian.h>
#include <zero/cdefs.h>
#include <zero/fastudiv.h>

struct v0;

typedef int32_t  v0reg;
typedef uint32_t v0ureg;
typedef v0ureg   v0memadr;
typedef void     v0iofunc_t(struct v0 *vm, uint16_t port, uint8_t reg);

struct v0iofuncs {
    v0iofunc_t *rdfunc;
    v0iofunc_t *wrfunc;
};

#define v0clrmsw(vm)   ((vm)->sysregs[V0_MSW_REG] = 0)
#define v0setcf(vm)    ((vm)->sysregs[V0_MSW_REG] |= V0_MSW_CF_BIT)
#define v0setzf(vm)    ((vm)->sysregs[V0_MSW_REG] |= V0_MSW_ZF_BIT)
#define v0setof(vm)    ((vm)->sysregs[V0_MSW_REG] |= V0_MSW_OF_BIT)
#define v0setif(vm)    ((vm)->sysregs[V0_MSW_REG] |= V0_MSW_IF_BIT)
#define v0cfset(vm)    ((vm)->sysregs[V0_MSW_REG] & V0_MSW_CF_BIT)
#define v0zfset(vm)    ((vm)->sysregs[V0_MSW_REG] & V0_MSW_ZF_BIT)
#define v0ofset(vm)    ((vm)->sysregs[V0_MSW_REG] & V0_MSW_OF_BIT)
#define v0ifset(vm)    ((vm)->sysregs[V0_MSW_REG] & V0_MSW_IF_BIT)

#define V0_NINST_MAX   256
#define V0_NIOPORT_MAX 4096 // must fit in val-field of struct v0op

/* accumulator (general-purpose register) IDs */
#define V0_R0_REG      0x00
#define V0_R1_REG      0x01
#define V0_R2_REG      0x02
#define V0_R3_REG      0x03
#define V0_R4_REG      0x04
#define V0_R5_REG      0x05
#define V0_R6_REG      0x06
#define V0_R7_REG      0x07
#define V0_R8_REG      0x08
#define V0_R9_REG      0x09
#define V0_R10_REG     0x0a
#define V0_R11_REG     0x0b
#define V0_R12_REG     0x0c
#define V0_R13_REG     0x0d
#define V0_R14_REG     0x0e
#define V0_R15_REG     0x0f
#define V0_NGENREG     16 // number of registers in group (general, system)
#define V0_NSAVEREG    8  // caller saves r0..r7
/* system register IDs */
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define V0_RET_LO      0x00 // [dual-word] return value register, low word
#define V0_RET_HI      0x01 // [dual-word] return value register, high word
#else
#define V0_RET_HI      0x00 // [dual-word] return value register, high word
#define V0_RET_LO      0x01 // [dual-word] return value register, low word
#endif
#define V0_MSW_REG     0x00 // machine status word
#define V0_FP_REG      0x01 // frame pointer
#define V0_SP_REG      0x02 // stack pointer
#define V0_PC_REG      0x03 // program counter i.e. instruction pointer
#define V0_LN_REG      0x04 // link-register; return address
#define V0_AC_REG      0x05 // argument count; used with RET
#define V0_VC_REG      0x05 // argument count; used with ENTER
#define V0_SYSREG_BIT  0x08 // denotes system-only access
#if 0
#define V0_IHT_REG     0x08 // interrupt handler table base address
#define V0_PDB_REG     0x09 // page directory base address register
#define V0_TID_REG     0x0a // task ID register
#define V0_TLS_REG     0x0b // thread-local storage base address register
#define V0_TSR_REG     0x0c // task-structure/state base address
#endif
#define V0_NSYSREG     16
/* values for sysregs[V0_MSW] */
#define V0_MSW_ZF_BIT  (1 << 0) // zero-flag
#define V0_MSW_OF_BIT  (1 << 1) // overflow-flag
#define V0_MSW_CF_BIT  (1 << 2) // carry-flag
#define V0_MWS_IF_BIT  (1 << 29) // interrupts pending
#define V0_MSW_SF_BIT  (1 << 30) // system-mode
#define V0_MSW_LF_BIT  (1 << 31) // bus lock flag
/* program segments */
#define V0_TEXT_SEG    0x00 // code
#define V0_RODATA_SEG  0x01 // read-only data (string literals etc.)
#define V0_DATA_SEG    0x02 // read-write (initialised) data
#define V0_BSS_SEG     0x03 // uninitialised (zeroed) runtime-allocated data
#define V0_STACK_SEG   0x04
#define V0_NSEG        8
/* option-bits for flg-member */
#define V0_TRACE       0x01
#define V0_BACKTRACE   0x02
#define V0_PROFILE     0x04
struct v0 {
    v0reg             genregs[V0_NGENREG];
    v0ureg            sysregs[V0_NSYSREG];
    v0ureg            segs[V0_NSEG];
    void             *seglims[V0_NSEG];
    long              flg;
    char             *mem;
    struct v0iofuncs *iovec;
    struct divuf16   *divu16tab;
};

/* OPCODES */

/*
 * 32-bit little-endian argument parcel
 * - declared as union for 32-bit alignment of all arguments
 */
union v0oparg {
    uint32_t adr;  // memory address
    uint32_t ndx;  // memory address
    //    uint32_t rmap; // register bitmap for PSHM, POPM - see V0_REG_BIT()
    uint32_t u32;  // unsigned 32-bit integer
    int32_t  i32;  // signed 32-bit integer
    uint16_t u16;  // unsigned 16-bit integer
    int16_t  i16;  // signed 16-bit integer
    uint8_t  u8;   // unsigned 8-bit integer
    int8_t   i8;   // signed 8-bit integer
};

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define v0mkopid(unit, inst) ((uint8_t)((unit) | ((inst) << 4)))
#define v0getunit(code)      ((code) & 0x0f)
#define v0getinst(code)      ((code) >> 4)
#else
#define v0mkopid(unit, inst) ((uint8_t)(((unit) << 4) | (inst)))
#define v0getunit(code)      ((code) >> 4)
#define v0getop(code)        ((code) & 0x0f)
#endif

/* addressing modes */
#define V0_REG_ADR     0x00 // (%reg), address in register
#define V0_DIR_ADR     0x01 // address word follows opcode or is in val-field
#define V0_NDX_ADR     0x02 // ndx(%reg), ndx in opcode val-field
#define V0_PIC_ADR     0x03 // PC-relative; position-independent code

/*
 * special uses for val
 * - shift or rotation count
 * - flags
 */
#define V0_IMM_VAL_MAX 0xfff
#define V0_IMM_VAL_MIN (-0x7ff - 1)
#define V0_TRAP_BIT    (1 << 13) // breakpoint
#define V0_SIGNED_BIT  (1 << 12) // signed operation

struct v0op {
    unsigned int  code : 8;
    unsigned int  reg1 : 4;
    unsigned int  reg2 : 4;
    unsigned int  adr  : 2;
    unsigned int  parm : 2;
    int           val  : 12;
    union v0oparg arg[EMPTY];
};

/* predefined I/O ports */
#define V0_STDIN_PORT  0       // keyboard input
#define V0_STDOUT_PORT 1       // console or framebuffer output
#define V0_STDERR_PORT 2       // console or framebuffer output
#define V0_RTC_PORT    3       // real-time clock
#define V0_TMR_PORT    3       // high-resolution timer for profiling
#define V0_MOUSE_PORT  4       // mouse input

/* framebuffer graphics interface */
#define V0_FB_BASE     (3UL * 1024 * 1024 * 1024)      // base address

/* exceptions */

#define V0_INV_OPCODE    0x01
#define V0_TEXT_FAULT    0x02
#define V0_STACK_FAULT   0x03
#define V0_SEG_FAULT     0x04
#define V0_DIV_BY_ZERO   0x05
#define V0_INV_MEM_READ  0x06
#define V0_INV_MEM_WRITE 0x07
#define V0_INV_MEM_ADR   0x08
#define V0_INV_IO_READ   0x09
#define V0_INV_IO_WRITE  0x0a

/* debugging */

struct v0opinfo {
    char *unit;
    char *op;
    char *func;
};

#endif /* __V0_VM32_H__ */

