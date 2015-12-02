#ifndef __BITS_UCONTEXT_H__
#define __BITS_UCONTEXT_H__

#include <signal.h>

/* FIXME: implement mcontext_t for Zero Somewhere(TM) :) */
#if ((defined(__i386__) || defined(__i486__)                            \
      || defined(__i586__) || defined(__i686__))                        \
     && !defined(__x86_64__) && !defined(__amd64__))

#include <stdint.h>
#include <zero/types.h>

#define __mcontext_t_defined

typedef struct __ucontext {
    sigset_t            uc_sigmask;
    mcontext_t          uc_mcontext;
    struct __ucontext  *uc_link;
    stack_t             uc_stack;
#if 0
    ASMLINK void      (*__func)(int, ...);
    int                 __narg;
    int                *__args;
#endif
} ucontext_t;

#endif /* __BITS_UCONTEXT_H__ */

