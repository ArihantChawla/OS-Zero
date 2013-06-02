#ifndef __WPM_WPM_H__
#define __WPM_WPM_H__

#define WPMWORDSIZE 32

#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <wpm/mem.h>
#if (WPM_VC)
#include <vcode/vc.h>
#endif

#define THRSTKSIZE  (128 * 1024)
#define WPMTEXTBASE 8192UL

#if (WPMWORDSIZE == 32)
typedef int32_t  wpmword_t;
typedef uint32_t wpmuword_t;
#else
#error implement WPMWORDSIZE for non-32-bit setups
#endif

#if 0
#define align(adr, p2)                                                  \
    (!((adr) & ((p2) - 1)) ? (adr) : ((adr) + ((p2) - ((adr) & ((p2 - 1))))))
#endif

#define OPINVAL    0x00
#define RESOLVE    (~((wpmword_t)0))

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
#define MSWZF      0x00000001
#define MSWCF      0x00000002
#define MSWOF      0x00000004
#define MSWSF      0x00000008

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
#define OPCLI      0x2d // disable interrupts
#define OPSTI      0x2e // enable interrupts
#define OPIRET     0x2f // return from interrupt handler
#define OPTHR      0x30 // start new thread at given address
#define OPCMPSWAP  0x31 // atomic compare and swap
#define OPINB      0x32 // read 8-bit byte from port
#define OPOUTB     0x33 // write 8-bit byte to port
#define OPINW      0x34 // read 16-bit word
#define OPOUTW     0x35 // write 16-bit word
#define OPINL      0x36 // read 32-bit long
#define OPOUTL     0x37 // write 32-bit long
#define OPHOOK     0x38 // system services
#define WPMNASMOP  256
/* unit IDS */
#define UNIT_ALU   0x00	// arithmetic logical unit
#if (WPM_VC)
#define UNIT_VEC   0x01
#endif
#define WPMNUNIT   (1 << 3)
/* register flags */
#define NREG       16
#define NFREG      16
#if (WPM_VC)
#define NVREG      8    // later, 2 per thread
#define NVITEM     8
#endif
struct _wpmopcode {
    wpmuword_t code;
    wpmword_t  args[2];
};

/* VCODE instruction IDs */
#define OPVADD       0x01
#define OPVSUB       0x02
#define OPVMUL       0x03
#define OPVDIV       0x04
#define OPVMOD       0x05
#define OPVLT        0x06
#define OPVLTE       0x07
#define OPVGT        0x08
#define OPVGTE       0x09
#define OPVEQ        0x0a
#define OPVINEQ      0x0b
#define OPVSHL       0x0c
#define OPVSHR       0x0d
#define OPVNOT       0x0e
#define OPVAND       0x0f
#define OPVOR        0x10
#define OPVXOR       0x11
#define OPVSELECT    0x12
#define OPVRAND      0x13
#define OPVFLOOR     0x14
#define OPVCEIL      0x15
#define OPVTRUNC     0x16
#define OPVROUND     0x17
#define OPVITOF      0x18
#define OPVITOB      0x19
#define OPVBTOI      0x1a
#define OPVLOG       0x1b
#define OPVSQRT      0x1c
#define OPVEXP       0x1d
#define OPVSIN       0x1e
#define OPVCOS       0x1f
#define OPVTAN       0x20
#define OPVASIN      0x21
#define OPVACOS      0x22
#define OPVATAN      0x23
#define OPVSINH      0x24
#define OPVCOSH      0x25
#define OPVTANH      0x26
/* vector instructions */
#define OPVPLSCAN    0x27
#define OPVMULSCAN   0x28
#define OPVMAXSCAN   0x29
#define OPVMINSCAN   0x2a
#define OPVANDSCAN   0x2b
#define OPVORSCAN    0x2c
#define OPVXORSCAN   0x2d
#define OPVPLREDUCE  0x2e
#define OPVMULREDUCE 0x2f
#define OPVMAXREDUCE 0x30
#define OPVMINREDUCE 0x31
#define OPVANDREDUCE 0x32
#define OPVORREDUCE  0x33
#define OPVXORREDUCE 0x34
#define OPVPERMUTE   0x35
#define OPVDPERMUTE  0x36
#define OPVFPERMUTE  0x37
#define OPVBPERMUTE  0x38
#define OPVBFPERMUTE 0x39
#define OPVDFPERMUTE 0x3a
#define OPVEXTRACT   0x3b
#define OPVREPLACE   0x3c
#define OPVPACK      0x3d
#define OPVRANKUP    0x3e
#define OPVRANKDOWN  0x3f
#define OPVDIST      0x40
#define OPVINDEX     0x41
#define OPVLENGTH    0x42
/* segment descriptor instructions */
#define OPVMKDES     0x43
#define OPVLENGTHS   0x44
#define OPVCOPY      0x45
#define OPVPOP       0x46
#define OPVCPOP      0x47
#define OPVPAIR      0x48
#define OPVUNPAIR    0x49
/* nfo bits for vector operations */
#define OPV_INT      0x01
#define OPV_FLOAT    0x02
#define OPV_BOOL     0x03
struct wpmopcode {
    unsigned  inst     : 8;	// instruction ID
    unsigned  unit     : 2;	// unit ID
    unsigned  arg1t    : 3;	// argument #1 type
    unsigned  arg2t    : 3;     // argument #2 type
    unsigned  reg1     : 6;	// register #1 ID + addressing flags
    unsigned  reg2     : 6;	// register #2 ID + addressing flags
    unsigned  size     : 2;     // operation size == size << 2
    unsigned  res      : 2;     // reserved
    wpmword_t args[2];
} __attribute__ ((__packed__));

/* argument types */
#define VOP_ADR 0
#define VOP_REG 1
struct wpmveccode {
    unsigned inst :  8;
    unsigned unit :  2;         // UNIT_VEC
    unsigned arg1t : 1;
    unsigned arg2t : 1;
    unsigned reg1  : 4;
    unsigned reg2  : 4;
    unsigned elsz  : 3;         // element size == opsz + 1
    unsigned treg  : 4;         // temporary register ID result
    unsigned res   : 5;
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
    wpmword_t  regs[NREG] ALIGNED(CLSIZE);
    double     fregs[NFREG] ALIGNED(CLSIZE);
#if (WPM_VC)
    /* address registers */
    vcint      varegs[NVREG] ALIGNED(CLSIZE);
//    int64_t   vregs[NVREG][NVITEM] ALIGNED(CLSIZE);
    vcint      vlregs[NVREG];    // vector lengths
    vcint      vtregs[NVREG];    // vector types
//    wpmadr_t  vsp;              // vector stack pointer
#endif
    wpmadr_t  msw;              // machine status word
    wpmadr_t  fp;               // frame pointer
    wpmadr_t  sp;               // stack pointer
    wpmadr_t  pc;               // program counter (instruction pointer)
#if 0
    wpmadr_t  pd;               // page directory address
    wpmadr_t  iv;               // interrupt vector address
#endif
    wpmadr_t  isp;              // interrupt stack pointer
} ALIGNED(PAGESIZE);

struct wpm {
    struct wpmcpustate cpustat;
    volatile long      shutdown;
    volatile long      thrid;
    wpmadr_t           brk;
};

struct wpm * wpminit(void);
void *       wpmloop(void *start);
void         wpmprintop(struct wpmopcode *op);
void         wpminitthr(wpmmemadr_t pc);

extern __thread struct wpm *wpm;

struct wpmstackframe {
    wpmadr_t oldfp;
    wpmadr_t retadr;
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

