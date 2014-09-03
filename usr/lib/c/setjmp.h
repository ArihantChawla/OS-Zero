#ifndef __SETJMP_H__
#define __SETJMP_H__

#if defined(__x86_64__) || defined(__amd64__)
#include <x86-64/setjmp.h>
#elif defined(__arm__)
#include <arm/setjmp.h>
#elif defined(__i386__)
#include <ia32/setjmp.h>
#endif

typedef struct _jmpbuf jmp_buf[1];
#if (_POSIX_C_SOURCE) || (_XOPEN_SOURCE)
typedef struct _jmpbuf sigjmp_buf;

/* ISO C prototypes. */
extern int  setjmp(jmp_buf env);
extern void longjmp(jmp_buf env, int val);

/* Unix prototypes. */
extern int  _setjmp(jmp_buf env);
extern void _longjmp(jmp_buf env, int val);

#endif /* __SETJMP_H__ */

