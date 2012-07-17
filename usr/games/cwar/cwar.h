#ifndef __CWAR_H__
#define __CWAR_H__

#define CWARMEMSZ     (8 * 1024 * 1024)

#define CWARMSWZF     0x01
#define CWARMSWCF     0x02
#define CWARMSWSF     0x04

/* values for opcode */
#define CWAROPSINGLE  0x80      // single-argument operation
#define CWAROPDUAL    0x40      // dual-argument operation
/* logical operations */
#define CWAROPAND     0x01      // AND of two values
#define CWAROPOR      0x02      // OR of two values
#define CWAROPXOR     0x03      // XOR of two values
#define CWAROPCOMPL   0x04      // 2's complement
/* arithmetic operations */
#define CWAROPADD     0x05      // signed addition
#define CWAROPSUB     0x06      // signed subtraction
#define CWAROPDADDU   0x07      // unsigned addition
#define CWAROPSUBU    0x08      // unsigned subtraction
#define CWAROPMUL     0x09
#define CWAROPSHR     0x0a
#define CWAROPSHL     0x0b
#define CWAROPSHRL    0x0c
#define CWAROPDIV     0x0d
#define CWAROPMOD     0x0e 
/* memory operations */
#define CWAROPMOV     0x0f

/* values for opbits */
#define CWAROPIMMSZ   0x0007    // 3-bit shift count
#define CWAROPARG0MEM 0x0008    // argument 0 is memory
#define CWAROPARG1MEM 0x0010    // argument 1 is memory
#define CWAROPARG0PC  0x0020    // operand 0 is program counter
#define CWAROPARG0MSW 0x0040    // operand 0 is machine status word

/* values for opregs */
#define CWARREGRA     0x00      // register A
#define CWARREGRB     0x01      // register B
#define CWARREGRC     0x02      // register C
#define CWARREGRD     0x03      // register D
#define CWARREGRE     0x04      // register F
#define CWARREGRF     0x05      // register G
#define CWARREGRSP    0x06      // stack pointer
#define CWARREGRFP    0x07      // frame pointer

#define cwarsrcreg(ip)  ((ip)->opregs & 0xf)
#define cwardestreg(ip) (((ip)->opregs >> 4) & 0xf)
struct cwarinstr {
    uint8_t  opcode;            // instruction op code
    uint8_t  opregs;            // source and destination register IDs
    uint16_t opbits;            // miscellaneous bit parameters
    uint32_t immed;             // [embedded] immediate value
} PACK();

struct cwarframe {
    uint32_t retfp;
    uint32_t retadr;
    uint32_t args[EMPTY];
};

/* immediate address */
#define cwarpush(mp, val)                                               \
    ((mp)->genregs.rsp -= 8, (mp)->mem[(mp)->genregs.rsp] = (val))
#define cwarpop(mp, mem)                                                \
    ((mem) = ((mp)->mem)[(mp)->genregs.rsp], (mp)->genregs.rsp += 4)
#define cwarenter(mp)                                                   \
    ((mp)->genregs.rfp = (mp)->genregs.rsp)
#define cwarleave(mp)                                                   \
    cwarpop(mp, (mp)->genregs.fp); cwarpop(mp, (mp)->eip))
#define cwarcall1(mp, arg0)                                             \
    do {                                                                \
        cwarpush(mp, arg0);                                             \
        cwarpush(mp, eip);                                              \
        cwarpush(mp, fp);                                               \
        cwarenter(mp);                                                  \
    } while (0)
#define cwarcall2(mp, arg0)                                             \
    do {                                                                \
        cwarpush(mp, arg1);                                             \
        cwarpush(mp, arg0);                                             \
        cwarpush(mp, eip);                                              \
        cwarpush(mp, fp);                                               \
        cwarenter(mp);                                                  \
    } while (0)
#define cwargetarg0(mp)                                                 \
    (((uint32_t *)(mp)->mem)[((mp)->genregs.rfp >> 2) + 2])
#define cwargetarg1(mp)                                                 \
    (((uint32_t *)(mp)->mem)[((mp)->genregs.rfp >> 2) + 3])
#define cwarsetra(mp, val)                                              \
    ((mp)->genregs.ra = (val))
#define cwarsetrb(mp, val)                                              \
    ((mp)->genregs.rb = (val))
#define cwarsetrc(mp, val)                                              \
    ((mp)->genregs.rc = (val))
#define cwarsetrd(mp, val)                                              \
    ((mp)->genregs.rd = (val))
#define cwarsetre(mp, val)                                              \
    ((mp)->genregs.re = (val))
#define cwarsetrf(mp, val)                                              \
    ((mp)->genregs.rf = (val))
#define cwarsetrsp(mp, val)                                             \
    ((mp)->genregs.rsp = (val))
#define cwarsetrfp(mp, val)                                             \
    ((mp)->genregs.rfp = (val))
#define cwargetra(mp)                                                   \
    ((mp)->genregs.ra)
#define cwargetrb(mp)                                                   \
    ((mp)->genregs.rb)
#define cwargetrc(mp)                                                   \
    ((mp)->genregs.rc)
#define cwargetrd(mp)                                                   \
    ((mp)->genregs.rd)
#define cwargetre(mp)                                                   \
    ((mp)->genregs.re)
#define cwargetrf(mp)                                                   \
    ((mp)->genregs.rf)
#define cwargetrsp(mp)                                                  \
    ((mp)->genregs.rsp)
#define cwargetrfp(mp)                                                  \
    ((mp)->genregs.rfp)
#define cwargetretfp(mp)                                                \
    (((mp)->mem)[(mp)->genregs.rsp >> 2])
#define cwargetretadr(mp)                                               \
    (((mp)->mem)[((mp)->genregs.rsp >> 2) + 1])
#define cwarmemref1(mp, val)                                            \
    (((mp)->mem)[(val) >> 3])
/* register + immediate, register + register */
#define cwarmemref2(mp, arg0, arg1)                                     \
    (((mp)->mem)[arg0 + (arg1 << 3)])
struct cwarmach {
    struct {
        uint64_t ra;
        uint64_t rb;
        uint64_t rc;
        uint64_t rd;
        uint64_t re;
        uint64_t rf;
        uint64_t rsp;
        uint64_t rfp;
    } genregs;
    uint64_t  msw;
    uint64_t  pc;
    uint32_t *mem;
    size_t    memsz;
};

#endif /* __CWAR_H__ */

