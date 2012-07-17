/*
 * cc.h - internal compiler header for Lightning C Tool Library.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_CC_H
#define EARTHQUAKE_INTERNAL_CC_H

#include <limits.h>

#include <zero/cdecl.h>
#include <zero/param.h>

#define WORD8_MAX 0xff
extern unsigned long __Eccoptbl[WORD8_MAX + 1];

void __Eccinitkeywords(void);
struct __Ecckeyword * __Eccfindkeyword(uint8_t *name);
struct __Ecctoken * __Eccparse(void);
struct __Ecctoken * __Eccparsetok(struct __Ecpptoken *tok);
struct __Ecctoken * __Eccparseid(struct __Ecpptoken *cpptok);
struct __Ecctoken * __Eccparseconst(struct __Ecpptoken *cpptok);
struct __Ecctoken * __Eccparsepow2(uint8_t *str, unsigned long log2);
struct __Ecctoken * __Eccparsedec(uint8_t *str);
struct __Ecctoken * __Eccparseop(struct __Ecpptoken *cpptok);
struct __Ecctoken * __Eccparsestr(struct __Ecpptoken *cpptok);
struct __Ecctoken * __Eccalloctok(int allocname);
uint8_t * __Eccreallocname(struct __Ecctoken *tok, size_t size);
void __Eccfreetok(struct __Ecctoken *tok);

struct __Eccop {
    unsigned long type;       /* result type. */
    struct __Ecctoken *op;    /* operator. */
    struct __Ecctoken *left;  /* left-side operand. */
    struct __Ecctoken *right; /* right-side operand. */
    struct __Eccop *prev;     /* previous in input. */
    struct __Eccop *next;     /* next in input. */
};

struct __Eccparse {
    struct __Ecctoken *block;    /* current block. */
    struct __Ecctoken *func;     /* current function. */
    struct __Ecctoken *firsttok; /* first input token. */
};

struct __Ecc {
    struct __Eccparse parse ALIGNED(CLSIZE);
};

#endif /* EARTHQUAKE_INTERNAL_CC_H */

