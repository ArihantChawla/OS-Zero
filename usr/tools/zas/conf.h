/* zero virtual machine compile-time configuration */

#ifndef __ZAS_CONF_H__
#define __ZAS_CONF_H__

#define V0          1

#if defined(V0)
#include <v0/mach.h>
#include <v0/vm32.h>
#define ZAS32BIT    1
#define ZASNOP      V0_NOP
#define ZASREGINDEX 0x40000000U
#define ZASREGINDIR 0x80000000U
typedef struct v0op   zasop_t;
typedef union v0oparg zasarg_t;
#define zasadrtoptr(adr) (&v0vm->mem[adr])
#endif

#if !defined(ZASZEROHASH) && !ZASZEROHASH
#define ZASZEROHASH 1
#endif

#if 0
/* assembler features */
#undef ZASALIGN
/* align instructions and data on virtual machine word boundaries */
#define ZASALIGN 1
#endif

/*
 * choose input file buffering scheme
 * - default is to use <stdio.h> functionality
 */
#ifndef ZASMMAP
/* use mmap()'d regions for input file buffers */
#define ZASMMAP  1
#endif
#ifndef ZASBUF
/* use explicit I/O buffering of input files */
#define ZASBUF   0
#endif

#endif /* __ZAS_CONF_H__ */

