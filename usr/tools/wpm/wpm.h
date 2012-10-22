#ifndef __WPM_WPM_H__
#define __WPM_WPM_H__

#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <wpm/mem.h>
#if (ZPC)
#include <zpc/zpc.h>
#include <zpc/asm.h>
#endif

#define THRSTKSIZE  (128 * 1024)
#define WPMTEXTBASE 8192

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
typedef int32_t  wpmword_t;
typedef uint32_t wpmuword_t;
#endif

#if 0
#define align(adr, p2)                                                  \
    (!((adr) & ((p2) - 1)) ? (adr) : ((adr) + ((p2) - ((adr) & ((p2 - 1))))))
#endif

#define OPINVAL    0x00
#define RESOLVE    (~((wpmmemadr_t)0))

#define REGINDEX   0x10
#define REGINDIR   0x20
/* argument types */
#define ARGNONE    0x00	// no argument
#define ARGIMMED   0x01	// immediate argument
#define ARGADR     0x02	// symbol / memory address
#define ARGREG     0x03	// register
#define ARGSYM     0x04	// symbol address

#define PAGEPRES   0x00000001

/* machine status word */
#define MSW_ZF     0x00000001
#define MSW_CF     0x00000002
#define MSW_OF     0x00000004
#define MSW_SF     0x00000008

/* the first 4096-byte page at 0x00000000 has interrupt handler addresses */
#define TRAPTMR    0x00         // timer interrupt
#define TRAPDIV    0x01         // division by zero
#define TRAPBRK    0x01         // breakpoint
#define TRAPOF     0x02         // overflow
#define TRAPINV    0x03         // invalid opcode
#define TRAPPROT   0x04         // protection
#define NTRAP      16
#define INTKBD     0x10         // keyboard
#define INTMOUSE   0x11         // mouse
#define INTMSG     0x12         // incoming message
#define NINT       16

/* hooks */
#define HOOKPZERO  0            // r0 is number of pages
#define HOOKMALLOC 1            // r0 is size
#define HOOKMMAP   2            // r0 is desc, r1 is ofs, r2 is size, r3 is flg
#define HOOKOPEN   3            // r0 is pointer to NUL-terminated file name
#define HOOKREAD   4            // r0 is descriptor, r1 is buffer, r2 is length
#define HOOKWRITE  5            // r0 is descriptor, r1 is buffer, r2 is length
#define HOOKSEEK   6            // r0 is low offset, r1 is high offset, r2 is origin
#define HOOKCLOSE  7            // r0 is descriptor

/* standard I/O ports */
#define KBDINPORT  0x0000       // keyboard input
#define CONOUTPORT 0x0001       // console output
#define ERROUTPORT 0x0002       // error output

/* instruction set */

/* ALU instructions */
#define OPNOT      0x01	// 2's complement, args
#define OPAND      0x02	// logical AND
#define OPOR       0x03	// logical OR
#define OPXOR      0x04	// logical exclusive OR
#define OPSHR      0x05	// logical shift right (fill with zero)
#define OPSHRA     0x06	// arithmetic shift right (fill with sign)
#define OPSHL      0x07	// shift left (fill with zero)
#define OPROR      0x08	// rotate right
#define OPROL      0x09	// rotate left
#define OPINC      0x0a	// increment by one
#define OPDEC      0x0b	// decrement by one
#define OPADD	   0x0c	// addition
#define OPSUB	   0x0d	// subtraction
#define OPCMP      0x0e	// compare
#define OPMUL	   0x0f	// multiplication
#define OPDIV	   0x10	// division
#define OPMOD	   0x11	// modulus
#define OPBZ	   0x12	// branch if zero
#define OPBNZ	   0x13	// branch if not zero
#define OPBLT	   0x14	// branch if less than
#define OPBLE	   0x15	// branch if less than or equal to
#define OPBGT	   0x16	// branch if greater than
#define OPBGE	   0x17	// branch if greater than or equal to
#define OPBO	   0x18	// branch if overflow
#define OPBNO	   0x19	// branch if no overflow
#define OPBC	   0x1a	// branch if carry
#define OPBNC	   0x1b	// branch if no carry
#define OPPOP	   0x1c	// pop from stack
#define OPPUSH	   0x1d	// push to stack
#define OPMOV	   0x1e	// load/store 32-bit longword
#define OPMOVB     0x1f // load/store 8-bit byte
#define OPMOVW     0x20 // load/store 16-bit word
#define OPJMP      0x21 // jump to given address
#define OPCALL     0x22	// call subroutine
#define OPENTER    0x23	// subroutine prologue
#define OPLEAVE    0x24	// subroutine epilogue
#define OPRET	   0x25	// return from subroutine
#define OPLMSW     0x26	// load machine status word
#define OPSMSW	   0x27	// store machine status word
#define OPRESET    0x28 // reset into well-known state
#define OPNOP      0x29 // dummy operation
#define OPHLT      0x2a // halt execution
#define OPBRK      0x2b // breakpoint
#define OPTRAP     0x2c // trigger a trap (software interrupt)
#define OPCLI      0x2d // trigger a trap (software interrupt)
#define OPSTI      0x2e // trigger a trap (software interrupt)
#define OPIRET     0x2f // return from interrupt handler
#define OPTHR      0x30 // start new thread at given address
#define OPCMPSWAP  0x31 // atomic compare and swap
#define OPINB      0x32 // read 8-bit byte from port
#define OPOUTB     0x33 // write 8-bit byte to port
#define OPINW      0x34 // read 16-bit word
#define OPOUTW     0x35 // write 16-bit word
#define OPINL      0x36 // read 32-bit long
#define OPOUTL     0x37 // write 32-bit long
#define OPHOOK     0x38 // hook into [virtual] operating system services
#define WPMNASMOP  0x39
/* unit IDS */
#define UNIT_ALU   0x00	// arithmetic logical unit
#define UNIT_SIMD  0x01	// vector unit
#define UNIT_FPU   0x02	// floating point unit
/* register flags */
#define NREG       16
#define NFREG      16
#define NVREG      16
struct _wpmopcode {
    wpmuword_t code;
    wpmword_t  args[2];
};

struct wpmopcode {
    unsigned  inst     : 8;	// instruction ID
    unsigned  unit     : 2;	// unit ID
    unsigned  arg1t    : 3;	// argument #1 type
    unsigned  arg2t    : 3;     // argument #2 type
    unsigned  reg1     : 6;	// register #1 ID + addressing flags
    unsigned  reg2     : 6;	// register #2 ID + addressing flags
    unsigned  size     : 2;     // 1..3, shift count
    unsigned  res      : 2;
    wpmword_t args[2];
} __attribute__ ((__packed__));

struct wpmobjhdr {
    wpmuword_t nsym;      // number of [global] symbols
    wpmuword_t fsize;     // size of file
    wpmuword_t tofs;      // text segment offset
    wpmuword_t tsize;     // text segment size
    wpmuword_t dofs;      // data segment offset
    wpmuword_t dsize;     // data segment size
    wpmuword_t bofs;      // bss segment offset
    wpmuword_t bsize;     // bss segment size
};

/* initial state: all bytes zero */
struct wpmcpustate {
    wpmuword_t msw;               // machine status word
    wpmuword_t fp;                // frame pointer
    wpmuword_t sp;                // stack pointer
    wpmuword_t pc;                // program counter (instruction pointer)
#if 0
    wpmuword_t pd;                // page directory address
    wpmuword_t iv;                // interrupt vector address
#endif
    wpmuword_t isp;               // interrupt stack pointer
#if (ZPC)
    wpmword_t  regs[ZPCNREG] ALIGNED(CLSIZE);
    float      fregs[ZPCNREG];
    double     dregs[ZPCNREG];
#else
    wpmword_t  regs[NREG] ALIGNED(CLSIZE);
    double     fregs[NFREG] ALIGNED(CLSIZE);
#endif
};

struct wpm {
    struct wpmcpustate  cpustat;
    volatile long       shutdown;
    volatile long       thrid;
    wpmuword_t          brk;
};

struct wpm * wpminit(void);
void *       wpmloop(void *start);
#if (ZPC)
void         wpmprintop(struct zpcopcode *op);
#else
void         wpmprintop(struct wpmopcode *op);
#endif
void         wpminitthr(wpmmemadr_t pc);

extern __thread struct wpm *wpm;

struct wpmstackframe {
    wpmuword_t oldfp;
    wpmuword_t retadr;
    wpmuword_t args[EMPTY];
};


typedef void wpmophandler_t(struct wpmopcode *);
typedef void wpmhookfunc_t(struct wpmopcode *);

/* logical operations */
void opnot(struct wpmopcode *op);
void opand(struct wpmopcode *op);
void opor(struct wpmopcode *op);
void opxor(struct wpmopcode *op);
/* shift and rotate */
void opshr(struct wpmopcode *op);
void opshra(struct wpmopcode *op);
void opshl(struct wpmopcode *op);
void opror(struct wpmopcode *op);
void oprol(struct wpmopcode *op);
/* arithmetic operations */
void opinc(struct wpmopcode *op);
void opdec(struct wpmopcode *op);
void opadd(struct wpmopcode *op);
void opsub(struct wpmopcode *op);
void opcmp(struct wpmopcode *op);
void opmul(struct wpmopcode *op);
void opdiv(struct wpmopcode *op);
void opmod(struct wpmopcode *op);
/* branch operations */
void opbz(struct wpmopcode *op);
void opbnz(struct wpmopcode *op);
void opblt(struct wpmopcode *op);
void opble(struct wpmopcode *op);
void opbgt(struct wpmopcode *op);
void opbge(struct wpmopcode *op);
void opbo(struct wpmopcode *op);
void opbno(struct wpmopcode *op);
void opbc(struct wpmopcode *op);
void opbnc(struct wpmopcode *op);
/* stack operations */
void oppop(struct wpmopcode *op);
void oppush(struct wpmopcode *op);
/* load/store */
void opmov(struct wpmopcode *op);
void opmovd(struct wpmopcode *op);
void opmovb(struct wpmopcode *op);
void opmovw(struct wpmopcode *op);
/* jump */
void opjmp(struct wpmopcode *op);
/* function call interface */
void opcall(struct wpmopcode *op);
void openter(struct wpmopcode *op);
void opleave(struct wpmopcode *op);
void opret(struct wpmopcode *op);
/* machine status word */
void oplmsw(struct wpmopcode *op);
void opsmsw(struct wpmopcode *op);
/* reset and shutdown */
void opreset(struct wpmopcode *op);
void opnop(struct wpmopcode *op);
void ophlt(struct wpmopcode *op);
void opbrk(struct wpmopcode *op);
void optrap(struct wpmopcode *op);
void opcli(struct wpmopcode *op);
void opsti(struct wpmopcode *op);
void opiret(struct wpmopcode *op);
void opthr(struct wpmopcode *op);
void opcmpswap(struct wpmopcode *op);
void opinb(struct wpmopcode *op);
void opoutb(struct wpmopcode *op);
void opinw(struct wpmopcode *op);
void opoutw(struct wpmopcode *op);
void opinl(struct wpmopcode *op);
void opoutl(struct wpmopcode *op);
void ophook(struct wpmopcode *op);

#endif /* __WPM_WPM_H__ */

