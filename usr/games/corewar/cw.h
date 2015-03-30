#ifndef __COREWAR_CW_H__
#define __COREWAR_CW_H__

#include <zero/param.h>
#include <zero/cdecl.h>

#if (ZEUS)
#include <corewar/zeus.h>
#endif

#define CWNTURN     (128 * 1024)
#define CWNPROC     1024
#define CWINVAL     0xff
#define CWCORESIZE  8000
#define CWNONE      (~0L)
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

/* 64-bit instruction structure */
struct cwinstr {
    unsigned op   : 8;          // operation ID
    unsigned aflg : 8;          // operand #1 flags
    unsigned bflg : 8;          // operand #2 flags
    unsigned pid  : 8;          // process ID
    unsigned a    : 16;         // operand #1
    unsigned b    : 16;         // operand #2
} PACK();

typedef long cwinstrfunc(long, long);

/* virtual machine structure */
struct cwmars {
    long             runqtab[2][CWNPROC];       // process run queues
    cwinstrfunc     *functab[CWNOP];            // instruction handler table
    long             proccnt[2];                // process counts
    long             curproc[2];                // current running process IDs
    long             nturn[2];                  // number of turns available
    long             running;                   // flag to indicate if running
    long             curpid;                    // current program ID
#if (ZEUSWINX11)
    struct zeusx11   zeusx11;                   // X11 GUI structure
#endif
    struct cwinstr  *optab;                     // operation lookup structure
    char           **opnames;                   // operation mnemonic table
};

void cwexec(long pid);

#endif /* __COREWAR_CW_H__ */

