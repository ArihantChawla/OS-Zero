#ifndef __COREWAR_CW_H__
#define __COREWAR_CW_H__

#include <zero/param.h>
#include <zero/cdecl.h>

#define CWNTURN     (128 * 1024)
#define CWNPROC     1024
#define CWINVAL     0xff
#define CWNCORE     8000
#define CWNONE      (~0)
#define CWCOREEMPTY NULL

/* opcodes */
#define CWOPDAT     0
#define CWOPMOV     1
#define CWOPADD     2
#define CWOPSUB     3
#define CWOPJMP     4
#define CWOPJMZ     5
#define CWOPJMN     6
#define CWOPCMP     7
#define CWOPSLT     8
#define CWOPDJN     9
#define CWOPSPL     10
#define CWNOP       11
/* flags */
/* signed value */
#define CWSIGNED    0x00
/* addressing modes, default is direct (relative) */
#define CWADRREL    0x01        // relative (default)
#define CWADRIMM    0x02        // immediate
#define CWADRINDIR  0x03        // indirect
#define CWADRPREDEC 0x04

/* instruction flags */
#define CWSIGNBIT   (1U << CWSIGNED)
#define CWRELBIT    (1U << CWADRREL)
#define CWIMMBIT    (1U << CWADRIMM)
#define CWINDIRBIT  (1U << CWADRINDIR)
#define CWPREDECBIT (1U << CWADRPREDEC)

/* 64-bit structure */
struct cwinstr {
    unsigned op   : 8;
    unsigned aflg : 8;
    unsigned bflg : 8;
    unsigned pid  : 8;
    unsigned a    : 16;
    unsigned b    : 16;
} PACK();

#endif /* __COREWAR_CW_H__ */

