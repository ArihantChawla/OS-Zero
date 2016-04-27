#ifndef __V86_OP_H__
#define __V86_OP_H__

#include <ctype.h>
#include <stdint.h>

#define V86_OP_MAX_LEN   3      // maximum # of characters in mnemonic
#define V86_OP_CHAR_BITS 5      // A-Z

#define v86isopchar(c)   isalpha(toupper(c))
#define v86opcharid(c)   (toupper(c) - 'A')

typedef v86word v86opfunc(struct v86 *vm, struct v86op *op, v86word pc);

struct v86vmop {
    long       narg;
    v86opfunc *func;
};

#endif /* __V86_OP_H__ */

