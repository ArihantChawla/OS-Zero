/* valhalla virtual machine compile-time configuration */

#ifndef __VAS_CONF_H__
#define __VAS_CONF_H__

#define V0               1
#define VAS_LINE_BUFSIZE 65536

#if defined(V0)
#include <v0/mach.h>
#include <v0/vm.h>
#define VAS32BIT    1
#define VASNOP      V0_NOP
#define VASREGINDEX 0x40000000U
#define VASREGINDIR 0x80000000U
typedef struct v0op   vasop_t;
typedef union v0oparg vasarg_t;
#define vasadrtoptr(adr) (&v0vm->mem[adr])
#endif

#define VASZEROHASH 1

#if 0
/* assembler features */
#undef VASALIGN
/* align instructions and data on virtual machine word boundaries */
#define VASALIGN 1
#endif

/*
 * choose input file buffering scheme
 * - default is to use <stdio.h> functionality
 */
#ifndef VASMMAP
/* use mmap()'d regions for input file buffers */
#define VASMMAP  1
#endif
#ifndef VASBUF
/* use explicit I/O buffering of input files */
#define VASBUF   0
#endif

#endif /* __VAS_CONF_H__ */

