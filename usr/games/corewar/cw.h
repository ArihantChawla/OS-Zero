#ifndef __COREWAR_CW_H__
#define __COREWAR_CW_H__

#include <zero/param.h>
#include <zero/cdecl.h>

#define CWNCORE     4096
#define CWNONE      (~0)
#define CWCOREEMPTY NULL

/* opcodes */
#define CWOPDAT     0
#define CWOPMOV     1
#define CWOPADD     2
#define CWOPSUB     3
#define CWOPJMP     4
#define CWOPJMZ     5
#define CWOPDJZ     6
#define CWOPCMP     7
#define CWNOP       8
/* flags */
/* signed value */
#define CWSIGNED    0x00
/* addressing modes, default is relative */
#define CWADRIMM    0x01        // immediate
#define CWADRINDIR  0x02        // indirect

/* instruction flags */
#define CWSIGNBIT   (1U << CWSIGNED)
#define CWIMMBIT    (1U << CWADRIMM)
#define CWINDIRBIT  (1U << CWADRINDIR)

/* 64-bit structure */
struct cwinstr {
    unsigned op   : 8;
    unsigned aflg : 8;
    unsigned bflg : 8;
    unsigned pad  : 8;
    signed a      : 16;
    signed b      : 16;
} PACK();

#endif /* __COREWAR_CW_H__ */

