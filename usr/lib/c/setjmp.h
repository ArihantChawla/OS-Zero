#ifndef __SETJMP_H__
#define __SETJMP_H__

#if !defined(__KERNEL__)

#include <features.h>
#if ((defined(__i386__) || defined(__i486__)                            \
      || defined(__i586__) || defined(__i686__))                        \
     && !defined(__x86_64__) && !defined(__amd64__))
#include <ia32/setjmp.h>
#elif defined(__x86_64__) || defined(__amd64__)
#include <x86-64/setjmp.h>
#elif defined(__arm__)
#include <arm/setjmp.h>
#endif
#include <zero/cdecl.h>

#if (_POSIX_C_SOURCE) || (_XOPEN_SOURCE)
typedef struct _jmpbuf sigjmp_buf[1];
#endif

/* ISO C prototypes. */
int  setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

/* Unix prototypes. */
int  _setjmp(jmp_buf env);
void _longjmp(jmp_buf env, int val);

#if defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)
int  sigsetjmp(sigjmp_buf env, int savesigs);
void siglongjmp(sigjmp_buf env, int val);
#endif

#endif /* !defined(__KERNEL__) */

#endif /* __SETJMP_H__ */

