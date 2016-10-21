#ifndef __BITS_SETJMP_H__
#define __BITS_SETJMP_H__

#if !defined(__KERNEL__)

#include <features.h>
#include <zero/cdefs.h>
#if ((defined(__i386__) || defined(__i486__)                            \
      || defined(__i586__) || defined(__i686__))                        \
     && !defined(__x86_64__) && !defined(__amd64__))
#include <ia32/setjmp.h>
#elif defined(__x86_64__) || defined(__amd64__)
#include <x86-64/setjmp.h>
#elif defined(__arm__)
#include <arm/setjmp.h>
#endif
#include <zero/cdefs.h>

#if defined(__GLIBC__)
typedef jmp_buf __jmp_buf;
#endif

#endif /* !defined(__KERNEL__) */

#endif /* __BITS_SETJMP_H__ */

