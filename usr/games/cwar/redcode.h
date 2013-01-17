#include <stdint.h>
#include <zero/cdecl.h>

/* inst field */
/* 0x14 == 20 operations, encoded in 5-bit field */
#define INSTDAT          0x01
#define INSTMOV          0x02
#define INSTADD          0x03
#define INSTSUB          0x04
#define INSTMUL          0x05
#define INSTDIV          0x06
#define INSTMOD          0x07
#define INSTJMP          0x08
#define INSTJMZ          0x09
#define INSTJMN          0x0a
#define INSTDJN          0x0b
#define INSTSPL          0x0c
#define INSTCMP          0x0d
#define INSTSEQ          0x0e
#define INSTSNE          0x0f
#define INSTSLT          0x10
#define INSTLDP          0x11
#define INSTSTP          0x12
#define INSTNOP          0x13
#define NINSTOP          0x14
#define INSTOPMASK       0x1f
#define INSTADRMASK 

/* flags field */
#define ADRIMMED         0x01
#define ADRDIRECT        0x02
#define ADRAINDIR        0x04
#define ADRBINDIR        0x08
#define ADRAINDIRPREDEC  0x10
#define ADRBINDIRPREDEC  0x20
#define ADRAINDIRPOSTINC 0x40
#define ADRBINDIRPOSTINC 0x80

struct redcodeop {
    uint_fast8_t  inst;
    uint_fast8_t  flags;
    uint_fast32_t args[EMPTY];
};

