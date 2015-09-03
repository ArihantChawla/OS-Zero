#ifndef __UCONTEXT_H__
#define __UCONTEXT_H__

#include <signal.h>

/* FIXME: implement mcontext_t for Zero Somewhere(TM) :) */

typedef struct ucontext {
    struct ucontext *uc_link;
    sigset_t         uc_sigmask;
    stack_t          uc_stack;
//    mcontext_t       uc_mcontext;
} ucontext_t;

#if !defined(__KERNEL__)

int  getcontext(ucontext_t *uc);
int  setcontext(const ucontext_t *uc);
void makecontext(ucontext_t *uc, void (*func)(), int argc, ...);
int  swapcontext(ucontext_t *olduc, const ucontext_t *uc);

#endif /* !defined(__KERNEL__) */

#endif /* __UCONTEXT_H__ */

