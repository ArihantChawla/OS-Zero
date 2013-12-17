#ifndef __COREWAR_CW_H__
#define __COREWAR_CW_H__

#include <zero/param.h>
#include <zero/cdecl.h>

#define CWNCORE     4096
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
/* addressing modes, default is relative */
#define CWADRIMM    0   // immediate
#define CWADRINDIR  1   // indirect

/* instruction flags */
#define CWIMMBIT    (1U << CWADRIMM)
#define CWINDIRBIT  (1U << CWADRINDIR)

/* 32-bit structure */
struct cwinstr {
    unsigned op   : 3;
    unsigned aflg : 2;
    unsigned bflg : 2;
    unsigned pad  : 1;
    unsigned a    : 12;
    unsigned b    : 12;
} PACK();

#endif /* __COREWAR_CW_H__ */

