#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>

#define ZPCNREG    16

#define ZPCASMILL   0x00
#define ZPCASMNOT   0x01
#define ZPCASMSHR   0x02
#define ZPCASMSHRA  0x03
#define ZPCASMSHL   0x04
#define ZPCASMXOR   0x05
#define ZPCASMOR    0x06
#define ZPCASMAND   0x07
#define ZPCASMROR   0x08
#define ZPCASMROL   0x09
#define ZPCASMINC   0x0a
#define ZPCASMDEC   0x0b
#define ZPCASMADD   0x0c
#define ZPCASMSUB   0x0d
#define ZPCASMMUL   0x0e
#define ZPCASMDIV   0x0f
#define ZPCNASMOP   16

#define ZPCVPUILL   0x00        // illegal instruction
#define ZPCVPUNOT   0x01
#define ZPCVPUSHR   0x02        // logical right shift
#define ZPCVPUSHL   0x03        // logical left shift
#define ZPCVPUXOR   0x04        // logical OR
#define ZPCVPUOR    0x05        // logical OR
#define ZPCVPUAND   0x06        // logical AND
#define ZPCVPUADD   0x07        // arithmetic addition of subwords
#define ZPCVPUADDUS 0x08        // addition with unsigned saturation
#define ZPCVPUADDS  0x09        // addition with signed saturation
#define ZPCVPUSUB   0x0a        // subtraction
#define ZPCVPUMUL   0x0b        // multiplication
#define ZPCVPUUNPCK 0x0c        // unpack subwords
#define ZPCVPUPCK   0x0d        // pack subwords
#define ZPCNVPUOP   16

#define ZPCARGREG   0x00
#define ZPCARGIMMED 0x01
#define ZPCALU      0x00
#define ZPCVPU64    0x01
#define ZPCVPU128   0x02
struct zpcasminst {
    unsigned op     : 4;        // operation ID
    unsigned arg1t  : 4;        // argument #1 type
    unsigned arg2t  : 4;        // argument #2 type
    unsigned reg1   : 4;        // register ID #1
    unsigned reg2   : 4;        // register ID #2
    unsigned unit   : 4;        // unit ID
    unsigned arg1sz : 4;        // argument sizes in octets/bytes
    unsigned arg2sz : 4;        // argument sizes in octets/bytes
    unsigned pad2   : 32;       // 
    uint64_t args[EMPTY];
} PACK();

#define zpcgetstkreg(num)                                               \
    (zpcstkregs[(num)]->type == ZPCUINT                                 \
     ? zpcstkregs[(num)]->data.u64                                      \
     : zpcstkregs[(num)]->data.i64)
#define getstkarg1(op)                                                  \
    (((op)->arg1t == ZPCARGREG                                          \
      ? zpcgetstkreg((op)->reg1])                                       \
      : (op)->args[0])))
#define getstkarg2(op)                                                  \
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

