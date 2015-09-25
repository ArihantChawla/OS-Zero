#ifndef __BITS_UCONTEXT_H__
#define __BITS_UCONTEXT_H__

#include <signal.h>

/* FIXME: implement mcontext_t for Zero Somewhere(TM) :) */
#if ((defined(__i386__) || defined(__i486__)                            \
      || defined(__i586__) || defined(__i686__))                        \
     && !defined(__x86_64__) && !defined(__amd64__))

#include <stdint.h>
#include <zero/ia32/types.h>

typedef struct __mcontext {
    void               *gregs;
    void               *fpstate;
    struct m_pusha      genregs; // general purpose registers saved by PUSHA
    struct m_trapframe  iret;    // use IRET for jumping into contexts
    uint8_t             fctx[TCBFCTXSIZE];
} mcontext_t;
#define __mcontext_t_defined

#endif

typedef struct __ucontext {
#if defined(__mcontext_t_defined)
    mcontext_t        uc_mcontext;
#endif
    sigset_t          uc_sigmask;
    stack_t           uc_stack;
    struct ucontext  *uc_link;
    ASMLINK void    (*__func)(int, ...);
    int               __argc;
} ucontext_t;

#endif /* __BITS_UCONTEXT_H__ */

