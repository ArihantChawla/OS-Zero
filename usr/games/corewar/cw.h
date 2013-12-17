#ifndef __COREWAR_CW_H__
#define __COREWAR_CW_H__

#include <zero/param.h>
#include <zero/cdecl.h>

#define CWNCORE     4096
#define CWNEXTOP    (~0UL)

#define CWNONE      (~0UL)
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
/* addressing modes */
#define CWADRIMM    0   // immediate
#define CWADRREL    1   // relative
#define CWADRINDIR  2

/* instruction flags */
#define CWIMMBIT    (1U << CWADRIMM)
#define CWRELBIT    (1U << CWADRREL)
#define CWINDIRBIT  (1U << CWADRINDIR)

struct cwinstr {
    unsigned op   : 16;
    unsigned aflg : 8;
    unsigned bflg : 8;
    signed a      : 16;
    signed b      : 16;
} PACK();

#endif /* __COREWAR_CW_H__ */

