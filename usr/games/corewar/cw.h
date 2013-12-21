#ifndef __COREWAR_CW_H__
#define __COREWAR_CW_H__

#include <zero/param.h>
#include <zero/cdecl.h>

//#define CWNTURN     (128 * 1024)
#define CWNTURN     0xffffffff
#define CWNPROC     128
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
#define CWADRIMM    0x01        // immediate
#define CWADRINDIR  0x02        // indirect
#define CWADRPREDEC 0x03

/* instruction flags */
#define CWSIGNBIT   (1U << CWSIGNED)
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

