/*
 * cpp.h - internal preprocessor header for Lightning C Tool Library.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_CPP_H
#define EARTHQUAKE_INTERNAL_CPP_H

#include <sys/types.h>
#include <sys/param.h>

#include <zero/cdecl.h>
#include <zero/param.h>

struct __Ecpptoken * __Ecppstage1(struct __Efile *file);
struct __Ecpptoken * __Ecppgettok(struct __Efile *file);
struct __Ecpptoken * __Ecppgetop(struct __Efile *file, int ch);
struct __Ecpptoken * __Ecppgetsep(struct __Efile *file, int ch);
struct __Ecpptoken * __Ecppgetconst(struct __Efile *file, int ch);
struct __Ecpptoken * __Ecppgetstr(struct __Efile *file, int ch);
struct __Ecpptoken * __Ecppgetid(struct __Efile *file, int ch);
struct __Ecpptoken * __Ecppparseop(struct __Efile *file);
struct __Ecpptoken * __Ecppgetdef(struct __Efile *file);
struct __Ecpptoken * __Ecppinclude(struct __Efile *file);
struct __Ecpptoken * __Ecppgetif(struct __Efile *file);
int __Ecppskip(struct __Efile *file);
int __Ecppscantok(struct __Efile *file, int ch);
int __Ecppgettrig(struct __Efile *file);
int __Ecpptrigval(int ch);
int __Ecppgetdig(struct __Efile *file, int ch);
int __Ecppparseesc(uint8_t *str);
int __Ecppescval(int ch);
struct __Ecpptoken * __Ecppalloctok(void);
uint8_t * __Ecppreallocname(struct __Ecpptoken *tok, size_t size);
void __Ecppfreetok(struct __Ecpptoken *tok);
void __Ecppfreetoks(struct __Ecpptoken *tok);
struct __Efile * __Ecppfindincl(uint8_t *name, int local);
int __Ecppoutput(uint8_t *filename);
void __Ecppreset(void);

struct __Ecppparse {
    struct __Ecpptoken *firsttok; /* first input token. */
    struct __Ecpptoken *lasttok;  /* last input token read. */
};

struct __Ecpp {
    struct __Ecppparse parse ALIGNED(CLSIZE);
};

#endif /* EARTHQUAKE_INTERNAL_CPP_H */

