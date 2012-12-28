/*
 * THANKS
 * ------
 * - Henry 'froggey' Harrington for amd64-fixes
 */

#ifndef __X86_64__SETJMP_H__
#define __X86_64__SETJMP_H__

#include <stddef.h>
#include <stdint.h>
#include <signal.h>
#include <zero/cdecl.h>

//#include <mach/abi.h>


struct _jmpbuf {
    int64_t  rbx;
    int64_t  r12;
    int64_t  r13;
    int64_t  r14;
    int64_t  r15;
    int64_t  rbp;
    int64_t  rsp;
    int64_t  rip;
#if (_POSIX_SOURCE)
    sigset_t sigmask;
#elif (_BSD_SOURCE)
    int      sigmask;
#endif
} PACK();

/*
 * callee-save registers: rbp, rbx, r12...r15
 */

#define __setjmp(env)                                                   \
    __asm__ __volatile__ ("movq %0, %%rax\n"                            \
                          "movq %%rbx, %c1(%%rax)\n"                    \
                          "movq %%r12, %c2(%%rax)\n"                    \
                          "movq %%r13, %c3(%%rax)\n"                    \
                          "movq %%r14, %c4(%%rax)\n"                    \
                          "movq %%r15, %c5(%%rax)\n"                    \
                          "movq %c6(%%rbp), %%rdx\n"                    \
                          "movq %%rdx, %c7(%%rax)\n"                    \
                          "movq %c8(%%rbp), %%rcx\n"                    \
                          "movq %%rcx, %c9(%%rax)\n"                    \
                          "leaq %c10(%%rbp), %%rdx\n"                   \
                          "movq %%rdx, %c11(%%rax)\n"                   \
                          : "m" (env),                                  \
                            "i" (offsetof(struct _jmpbuf, rbx)),        \
                            "i" (offsetof(struct _jmpbuf, r12)),        \
                            "i" (offsetof(struct _jmpbuf, r13)),        \
                            "i" (offsetof(struct _jmpbuf, r14)),        \
                            "i" (offsetof(struct _jmpbuf, r15)),        \
                            "i" (offsetof(struct m_frame, rbp)),        \
                            "i" (offsetof(struct _jmpbuf, rbp)),        \
                            "i" (offsetof(struct m_frame, rip)),        \
                            "i" (offsetof(struct _jmpbuf, rip)),        \
                            "i" (offsetof(struct m_frame, args)),       \
                            "i" (offsetof(struct _jmpbuf, rsp))         \
                          : "rax", "rcx", "rdx")

#define __longjmp(env, val)                                             \
    __asm__ __volatile__ ("movq %0, %%rcx\n"                            \
                          "movq %0, %%rax\n"                            \
                          "movq %c0(%%rcx), %%rbx"                      \
                          "movq %c0(%%rcx), %%r12"                      \
                          "movq %c0(%%rcx), %%r13"                      \
                          "movq %c0(%%rcx), %%r14"                      \
                          "movq %c0(%%rcx), %%r15"                      \
                          "movq %c0(%%rcx), %%rbp"                      \
                          "movq %c0(%%rcx), %%rsp"                      \
                          "movq %c0(%%rcx), %%rdx"                      \
                          "jmpq *%rdx\n"                                \
                          : "m" (env),                                  \
                            "m" (val),                                  \
                            "i" (offsetof(struct _jmpbuf, rbx)),        \
                            "i" (offsetof(struct _jmpbuf, r12)),        \
                            "i" (offsetof(struct _jmpbuf, r13)),        \
                            "i" (offsetof(struct _jmpbuf, r14)),        \
                            "i" (offsetof(struct _jmpbuf, r15)),        \
                            "i" (offsetof(struct _jmpbuf, rbp)),        \
                            "i" (offsetof(struct _jmpbuf, rsp)),        \
                            "i" (offsetof(struct _jmpbuf, rip))         \
                          : "rax", "rbx", "rcx", "rdx",                 \
                            "r12", "r13", "r14", "r15", "r16",          \
                            "rbp", "rsp")
        

#endif /* __X86_64__SETJMP_H__ */

