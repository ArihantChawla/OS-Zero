/*
 * THANKS
 * ------
 * - Henry 'froggey' Harrington for amd64-fixes
 */

#ifndef __AMD64__SETJMP_H__
#define __AMD64__SETJMP_H__

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
    sigset_t sigmask;
} PACK();

/*
 * callee-save registers: rbp, rbx, r12...r15
 */
#define __setjmp(env)                                                   \
    __asm__ __volatile__ ("movq %0, %%rax\n" : : "m" (env));            \
    __asm__ __volatile__ ("movq %%rbx, %c0(%%rax)\n"                    \
                          : \
                          : "i" (offsetof(struct _jmpbuf, rbx)));       \
    __asm__ __volatile__ ("movq %%r12, %c0(%%rax)\n"                    \
                          : \
                          : "i" (offsetof(struct _jmpbuf, r12)));       \
    __asm__ __volatile__ ("movq %%r13, %c0(%%rax)\n"                    \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, r13)));       \
    __asm__ __volatile__ ("movq %%r14, %c0(%%rax)\n"                    \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, r14)));       \
    __asm__ __volatile__ ("movq %%r15, %c0(%%rax)\n"                    \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, r15)));       \
    __asm__ __volatile__ ("movq %c0(%%rbp), %%rdx\n"                    \
                          :                                             \
                          : "i" (offsetof(struct m_frame, rbp)));       \
    __asm__ __volatile__ ("movq %%rdx, %c0(%%rax)\n"                    \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, rbp)));       \
    __asm__ __volatile__ ("movq %c0(%%rbp), %%rcx\n"                    \
                          : \: "i" (offsetof(struct m_frame, rip)));    \
    __asm__ __volatile__ ("movq %%rcx, %c0(%%rax)\n"                    \
                          : \
                          : "i" (offsetof(struct _jmpbuf, rip)));       \
    __asm__ __volatile__ ("leaq %c0(%%rbp), %%rdx\n"                    \
                          :                                             \
                          : "i" (offsetof(struct m_frame, args)));      \
    __asm__ __volatile__ ("movq %%rdx, %c0(%%rax)\n"                    \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, rsp)))

#define __longjmp(env, val)                                             \
    __asm__ __volatile__ ("movq %0, %%rcx\n" : : "m" (env));            \
    __asm__ __volatile__ ("movq %0, %%rax\n" : : "m" (val));            \
    __asm__ __volatile__ ("movq %c0(%%rcx), %%rbx"                      \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, rbx)));       \
    __asm__ __volatile__ ("movq %c0(%%rcx), %%r12"                      \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, r12)));       \
    __asm__ __volatile__ ("movq %c0(%%rcx), %%r13"                      \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, r13)));       \
    __asm__ __volatile__ ("movq %c0(%%rcx), %%r14"                      \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, r14)));       \
    __asm__ __volatile__ ("movq %c0(%%rcx), %%r15"                      \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, r15)));       \
    __asm__ __volatile__ ("movq %c0(%%rcx), %%rbp"                      \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, rbp)));       \
    __asm__ __volatile__ ("movq %c0(%%rcx), %%rsp"                      \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, rsp)));       \
    __asm__ __volatile__ ("movq %c0(%%rcx), %%rdx" \
                          :                                             \
                          : "i" (offsetof(struct _jmpbuf, rip)));       \
    __asm__ __volatile__ ("jmpq *%rdx\n")

#endif /* __AMD64__SETJMP_H__ */

