#ifndef __STDIO_H__
#define __STDIO_H__

#include <stddef.h>
#include <limits.h>
#include <sys/types.h>
#include <zero/param.h>
#include <kern/syscall.h>
#include <kern/file.h>

typedef struct file FILE;
#if (USE_XOPEN)
#if defined(GNUC)
typedef _G_va_list va_list;
#else
#include <stdarg.h>
#endif
#endif
typedef off_t fpos_t;
/* setvbuf argument values */
#define _IOFBF       0			// fully buffered
#define _IOLBF       1			// line buffered
#define _IONBF	     2           // unbuffered
#define BUFSIZE PAGESIZE
#define EOF          (-1)
#if (USESVID) || (_XOPEN_SOURCE)
#define P_tmpdir     "/tmp"
#endif
/* FIXME: do these belong just here? */
#define L_tmpnam     PATH_MAX
/* #define TMP_MAX */
#define L_ctermid    PATH_MAX
#define L_cuserid    128
#define FOPEN_MAX    128
#define FILENAME_MAX NAME_MAX
extern struct FILE *stdin;
extern struct FILE *stdout;
extern struct FILE *stderr;
#if defined(__STDC__)
#define stdin
#define stdout
#define stderr
#endif
extern int remove(const char *file);
extern int rename(const char *file, const char *newname);
#if (_GNU_SOURCE)
extern int renameat(int fd, const char *name, int newfd, const char *newname);
#endif
extern FILE *tmpfile(void);
extern char *tmpnam(char *str);
extern char *tmpnam_r(char *str);

#endif /* __STDIO_H__ */

