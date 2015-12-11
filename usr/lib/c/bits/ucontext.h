#ifndef __BITS_UCONTEXT_H__
#define __BITS_UCONTEXT_H__

#include <signal.h>

#if defined(__x86_64__) || defined(__amd64__)
#include <x86-64/ucontext.h>
#elif (defined(__i386__) || defined(__i486__)                           \
       || defined(__i586__) || defined(__i686__))
#include <ia32/ucontext.h>
#endif

#include <stdint.h>
#include <zero/types.h>

typedef struct m_ctx mcontext_t;
#define __mcontext_t_defined

/* keep the beginning of this struct so we can use sigcontext_t */
typedef struct __ucontext {
    sigset_t           uc_sigmask;
    mcontext_t         uc_mcontext;
    unsigned long      uc_flags;
    struct __ucontext *uc_link;
    stack_t            uc_stack;
    struct m_fpstate   _fpstate;
} ucontext_t;

#endif /* __BITS_UCONTEXT_H__ */

