#ifndef __REX_REX_H__
#define __REX_REX_H__

#include <stdint.h>

#define LINEDEFLEN 1024

static int matchstar(int c, char *regexp, char *data);
static int matchhere(char **regexp, char *data);

#if (TRANSLATE)

#define NSPECBYTE 32

#define setspecbit(ch) setbit(specbitmap, (uint8_t)ch)
#define chisspec(ch)   bitset(specbitmap, (uint8_t)ch)

#define BEGIN 0x01
#define END   0x02
#define DOT   0x03
#define SET   0x04
#define STAR  0x05
#define CHAR  0x06

struct rex {
    uint8_t     cbits[32];
    struct rex *next;
    uint8_t     type;           // BEGIN, END, DOT, SET, STAR, CHAR
    uint8_t     n;              // 0 for any number of matches
    uint8_t     pad[6];
} PACK();

#endif

#endif /* __REX_REX_H__ */

