/* zero virtual machine compile-time configuration */

#ifndef __ZAS_CONF_H__
#define __ZAS_CONF_H__

#define ZASZEROHASH 1

/* virtual machine selection */
#ifndef ZVM
#define ZVM 1
#endif
#ifndef WPM
#define WPM 0
#endif
#ifndef ZEN
#define ZEN 0
#endif

/* assembler features */
#undef ZASALIGN
/* align instructions and data on virtual machine word boundaries */
#define ZASALIGN 1

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

