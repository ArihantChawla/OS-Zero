#ifndef __STDIO_H__
#define __STDIO_H__

#include <features.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
#include <zero/param.h>
#include <kern/io.h>
#include <stdarg.h>

typedef struct file   FILE;
typedef off_t         fpos_t;

extern FILE          *stdin;
extern FILE          *stdout;
extern FILE          *stderr;

#define EOF           (-1)
#define BUFSIZ        PAGESIZE
#define FILENAME_MAX  NAME_MAX
#define FOPEN_MAX     OPEN_MAX
/* setvbuf() arguments */
#define _IOFBF        0
#define _IOLBF        1
#define _IONBF        2
#define L_ctermid     128
#define L_cuserid     128
#define L_tmpnam      PATH_MAX
/* SEEK_CUR, SEEK_END, and SEEK_SET come from <kern/io.h> */
//#define TMP_MAX      10000
/* NULL is in <stddef.h> */
#define P_tmpdir      "/tmp"

#endif /* __STDIO_H__ */

