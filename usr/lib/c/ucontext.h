#ifndef __UCONTEXT_H__
#define __UCONTEXT_H__

#include <signal.h>

ucontext_t;
struct _ucontext {
    ucontext_t *uc_link;
    sigset_t    uc_sigmask;
    stack_t     uc_stack;
    mcontext_t  uc_mcontext;
};
typedef struct _ucontext ucontext_t;

int  getcontext(ucontext_t *uc);
int  setcontext(const ucontext_t *uc);
void makecontext(ucontext_t *uc, void (*func)(), int argc, ...);
int  swapcontext(ucontext_t *olduc, const ucontext_t *uc);

#endif /* __UCONTEXT_H__ */

