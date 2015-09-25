#include <features.h>
#include <setjmp.h>
#include <signal.h>
#include <zero/cdecl.h>

#if defined(_POSIX_SOURCE)

#if (!SIG32BIT)
#if (PTHREAD)
#define _savesigmask(sp) pthread_sigmask(SIG_BLOCK, NULL, sp)
#define _loadsigmask(sp) pthread_sigmask(SIG_SETMASK, sp, NULL)
#else
#define _savesigmask(sp) sigprocmask(SIG_BLOCK, NULL, sp)
#define _loadsigmask(sp) sigprocmask(SIG_SETMASK, sp, NULL)
#endif
#else
#define _savesigmask(sp)
#define _loadsigmask(sp)
#endif

#elif defined(_BSD_SOURCE)

#if (SIG32BIT)
#define _savesigmask(sp) ((sp)->norm = sigblock(0))
#define _loadsigmask(sp) (sigsetmask((sp)->norm))
#else
#define _savesigmask(sp) (*(sp) = sigblock(0))
#define _loadsigmask(sp) (sigsetmask(*(sp)))
#endif

#else

#if (SIG32BIT)
#define _savesigmask(sp) ((sp)->norm = siggetmask())
#define _loadsigmask(sp) (sigsetmask((sp)->norm))
#else
#define _savesigmask(sp) (*(sp) = siggetmask())
#define _loadsigmask(sp) (sigsetmask(*(sp)))
#endif

#endif /* defined(_POSIX_SOURCE) */

ASMLINK
int
setjmp(jmp_buf env)
{
    __setjmp(env);
#if !(USEOLDBSD)
    _savesigmask(&(env->sigmask));
#endif

    return 0;
}

ASMLINK
NORET
void
longjmp(jmp_buf env,
        int val)
{
#if !(USEOLDBSD)
    _loadsigmask(&(env->sigmask));
#endif
    __longjmp(env, val);

    /* NOTREACHED */
}

ASMLINK
int
_setjmp(jmp_buf env)
{
    __setjmp(env);

    return 0;
}

ASMLINK
NORET
void
_longjmp(jmp_buf env,
         int val)
{
    __longjmp(env, val);

    /* NOTREACHED */
}

#if defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)

ASMLINK
int
sigsetjmp(sigjmp_buf env, int savesigs)
{
    __setjmp(env);

    if (savesigs) {
        _savesigmask(&(env->sigmask));
        env->havesigs = 1;
    }

    return 0;
}

ASMLINK
NORET
void
siglongjmp(sigjmp_buf env, int val)
{
    if (env->havesigs) {
        _loadsigmask(&(env->sigmask));
    }
    __longjmp(env, val);

    /* NOTREACHED */
}

#endif

#if (defined(__i386__) || defined(__i486__)                             \
     || defined(__i586__) || defined(__i686__))                         \
    && !defined(__x86_64__) && !defined(__amd64__)

/*
 * callee-save registers: ebx, edi, esi, ebp, ds, es, ss.
 */

NOINLINE int
__setjmp(jmp_buf env)
{
    __asm__ __volatile__ ("movl %0, %%eax\n"
                          "movl %%ebx, %c1(%%eax)\n"
                          "movl %%esi, %c2(%%eax)\n"
                          "movl %%edi, %c3(%%eax)\n"
                          "movl %c4(%%ebp), %%edx\n"
                          "movl %%edx, %c5(%%eax)\n"
                          "movl %c6(%%ebp), %%ecx\n"
                          "movl %%ecx, %c7(%%eax)\n"
                          "leal %c8(%%ebp), %%edx\n"
                          "movl %%edx, %c9(%%eax)\n"
                          :
                          : "m" (env),
                          "i" (offsetof(struct _jmpbuf, ebx)),
                            "i" (offsetof(struct _jmpbuf, esi)),
                            "i" (offsetof(struct _jmpbuf, edi)),
                            "i" (offsetof(struct _jmpframe, ebp)),
                            "i" (offsetof(struct _jmpbuf, ebp)),
                            "i" (offsetof(struct _jmpframe, eip)),
                            "i" (offsetof(struct _jmpbuf, eip)),
                            "i" (offsetof(struct _jmpframe, args)),
                            "i" (offsetof(struct _jmpbuf, esp))
                          : "eax", "ecx", "edx");

    return 0;
}

void
__longjmp(jmp_buf env, int val)
{
    __asm__ __volatile__ ("movl %0, %%ecx\n"
                          "movl %1, %%eax\n"
                          "cmp $0, %eax\n"
                          "jne 0f\n"
                          "movl $1, %eax\n"
                          "0:\n"
                          "movl %c2(%%ecx), %%ebx"
                          "movl %c3(%%ecx), %%esi"
                          "movl %c4(%%ecx), %%edi"
                          "movl %c5(%%ecx), %%ebp"
                          "movl %c6(%%ecx), %%esp"
                          "movl %c7(%%ecx), %%edx"
                          "jmpl *%edx\n"
                          :
                          : "m" (env),
                            "m" (val),
                            "i" (offsetof(struct _jmpbuf, ebx)),
                            "i" (offsetof(struct _jmpbuf, esi)),
                            "i" (offsetof(struct _jmpbuf, edi)),
                            "i" (offsetof(struct _jmpbuf, ebp)),
                            "i" (offsetof(struct _jmpbuf, esp)),
                            "i" (offsetof(struct _jmpbuf, eip))
                          : "eax", "ebx", "ecx", "edx",
                            "esi", "edi", "ebp", "esp");
}

#elif defined(__x86_64__) || defined(__amd64__)
/*
 * callee-save registers: rbp, rbx, r12...r15
 */
NOINLINE int
__setjmp(jmp_buf env)
{
    __asm__ __volatile__ ("movq %0, %%rax\n"
                          "movq %%rbx, %c1(%%rax)\n"
                          "movq %%r12, %c2(%%rax)\n"
                          "movq %%r13, %c3(%%rax)\n"
                          "movq %%r14, %c4(%%rax)\n"
                          "movq %%r15, %c5(%%rax)\n"
                          "movq %c6(%%rbp), %%rdx\n"
                          "movq %%rdx, %c7(%%rax)\n"
                          "movq %c8(%%rbp), %%rcx\n"
                          "movq %%rcx, %c9(%%rax)\n"
                          "leaq %c10(%%rbp), %%rdx\n"
                          "movq %%rdx, %c11(%%rax)\n"
                          :
                          : "m" (env),
                            "i" (offsetof(struct _jmpbuf, rbx)),
                            "i" (offsetof(struct _jmpbuf, r12)),
                            "i" (offsetof(struct _jmpbuf, r13)),
                            "i" (offsetof(struct _jmpbuf, r14)),
                            "i" (offsetof(struct _jmpbuf, r15)),
                            "i" (offsetof(struct _jmpframe, rbp)),
                            "i" (offsetof(struct _jmpbuf, rbp)),
                            "i" (offsetof(struct _jmpframe, rip)),
                            "i" (offsetof(struct _jmpbuf, rip)),
                            "i" (offsetof(struct _jmpframe, args)),
                            "i" (offsetof(struct _jmpbuf, rsp))
                          : "rax", "rcx", "rdx");

    return 0;
}

void
__longjmp(jmp_buf env, int val)
{
    __asm__ __volatile__ ("movq %0, %%rcx\n"
                          "movq %1, %%rax\n"
                          "movq %c2(%%rcx), %%rbx\n"
                          "movq %c3(%%rcx), %%r12\n"
                          "movq %c4(%%rcx), %%r13\n"
                          "movq %c5(%%rcx), %%r14\n"
                          "movq %c6(%%rcx), %%r15\n"
                          "movq %c7(%%rcx), %%rbp\n"
                          "movq %c8(%%rcx), %%rsp\n"
                          "movq %c9(%%rcx), %%rdx\n"
                          "jmpq *%%rdx\n"
                          :
                          : "m" (env),
                            "m" (val),
                            "i" (offsetof(struct _jmpbuf, rbx)),
                            "i" (offsetof(struct _jmpbuf, r12)),
                            "i" (offsetof(struct _jmpbuf, r13)),
                            "i" (offsetof(struct _jmpbuf, r14)),
                            "i" (offsetof(struct _jmpbuf, r15)),
                            "i" (offsetof(struct _jmpbuf, rbp)),
                            "i" (offsetof(struct _jmpbuf, rsp)),
                            "i" (offsetof(struct _jmpbuf, rip))
                          : "rax", "rbx", "rcx", "rdx",
                            "r12", "r13", "r14", "r15",
                            "rsp");
}

#endif

