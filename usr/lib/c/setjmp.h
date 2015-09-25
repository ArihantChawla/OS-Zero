#ifndef __SETJMP_H__
#define __SETJMP_H__

#if !defined(__KERNEL__)

#include <features.h>
#include <zero/cdecl.h>
#if ((defined(__i386__) || defined(__i486__)                            \
      || defined(__i586__) || defined(__i686__))                        \
     && !defined(__x86_64__) && !defined(__amd64__))
#include <ia32/setjmp.h>
#elif defined(__x86_64__) || defined(__amd64__)
#include <x86-64/setjmp.h>
#elif defined(__arm__)
#include <arm/setjmp.h>
#endif

#if (_POSIX_C_SOURCE) || (_XOPEN_SOURCE)
typedef struct _jmpbuf sigjmp_buf[1];
#endif

/* ISO C prototypes. */
ASMLINK int  setjmp(jmp_buf env);
ASMLINK void longjmp(jmp_buf env, int val);

/* Unix prototypes. */
ASMLINK int  _setjmp(jmp_buf env);
ASMLINK void _longjmp(jmp_buf env, int val);

#if defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)
ASMLINK int  sigsetjmp(sigjmp_buf env, int savesigs);
ASMLINK void siglongjmp(sigjmp_buf env, int val);
#endif

#endif /* !defined(__KERNEL__) */

#endif /* __SETJMP_H__ */

