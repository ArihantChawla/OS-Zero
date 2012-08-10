#include <zero/param.h>
#include <zero/cdecl.h>

#define ZPCNREG    16

#define ZPCASMILL  0x00
#define ZPCASMNOT  0x01
#define ZPCASMSHR  0x02
#define ZPCASMSHRA 0x03
#define ZPCASMSHL  0x04
#define ZPCASMXOR  0x05
#define ZPCASMOR   0x06
#define ZPCASMAND  0x07
#define ZPCASMROR  0x08
#define ZPCASMROL  0x09
#define ZPCASMINC  0x0a
#define ZPCASMDEC  0x0b
#define ZPCASMADD  0x0c
#define ZPCASMSUB  0x0d
#define ZPCASMMUL  0x0e
#define ZPCASMDIV  0x0f
#define ZPCNASMOP  16

#define ZPCARGREG  0x00
struct zpcasminst {
    unsigned op    : 4;
    unsigned arg1t : 4;
    unsigned arg2t : 4;
    unsigned reg1  : 4;
    unsigned reg2  : 4;
    unsigned pad   : 44;
    uint64_t args[0];
} PACK();

#define zpcgetstkreg(num)                                               \
    (zpcstkregs[(num)]->type == ZPCUINT                                 \
     ? zpcstkregs[(num)]->data.u64                                      \
     : zpcstkregs[(num)]->data.i64)
#define getstkarg1(op)                                                  \
    (((op)->arg1t == ZPCARGREG                                          \
      ? zpcgetstkreg((op)->reg1])                                       \
      : (op)->args[0])))
#define getintarg2(op)                                                  \
    (((op)->arg2t == ZPCARGREG                                          \
      ? zpcgetstkreg((op)->arg2)                                        \
      : (arg1t == ZPCARGREG                                             \
         ? (op)->args[1]                                                \
         : (op)->args[0])))
#define getintarg1(op)                                                  \
    (((op)->arg1t == ZPCARGREG                                          \
      ? zpcintregs[(op)->reg1]                                          \
      : (op)->args[0]))
#define getintarg2(op)                                                  \
    (((op)->arg2t == ZPCARGREG                                          \
      ? zpcintregs[(op)->arg2]                                          \
      : (arg1t == ZPCARGREG                                             \
         ? (op)->args[1]                                                \
         : (op)->args[0])))

uint64_t zpcintregs[ZPCNREG];
double   zpcfltregs[ZPCNREG];
