#ifndef __IA32__SETJMP_H__
#define __IA32__SETJMP_H__

#include <stddef.h>
#include <stdint.h>
#include <signal.h>

#include <mach/cdecl.h>

struct _jmpbuf {
    int32_t  ebx;
    int32_t  esi;
    int32_t  edi;
    int32_t  ebp;
    int32_t  esp;
    int32_t  eip;
    sigset_t sigmask;
} PACK();

struct _jmpframe {
    int32_t ebp;
    int32_t eip;
    uint8_t args[EMPTY];
} PACK();

/*
 * callee-save registers: ebx, edi, esi, ebp, ds, es, ss.
 */
#define __setjmp(env)                                                   \
    /* load jmp_buf pointer into EAX */                                 \
    __asm__ ("movl %0, %%eax\n" : : "m" (env));                         \
    /* save EBX, ESI, and EDI */                                        \
    __asm__ ("movl %%ebx, %c0(%%eax)\n" : : "i" (offsetof(struct _jmpbuf, \
                                                          ebx)));       \
    __asm__ ("movl %%esi, %c0(%%eax)\n" : : "i" (offsetof(struct _jmpbuf, \
                                                          esi)));       \
    __asm__ ("movl %%edi, %c0(%%eax)\n" : : "i" (offsetof(struct _jmpbuf, \
                                                          edi)));       \
    /* save caller frame pointer */                                     \
    __asm__ ("movl %c0(%%ebp), %%edx\n" : : "i" (offsetof(struct _jmpframe, \
                                                          ebp)));       \
    __asm__ ("movl %%edx, %c0(%%eax)\n" : : "i" (offsetof(struct _jmpbuf, \
                                                          ebp)));       \
    /* save caller instruction pointer */                               \
    __asm__ ("movl %c0(%%ebp), %%ecx\n" : : "i" (offsetof(struct _jmpframe, \
                                                          eip)));       \
    __asm__ ("movl %%ecx, %c0(%%eax)\n" : : "i" (offsetof(struct _jmpbuf, \
                                                          eip)));       \
    /* save caller stack pointer */                                     \
    __asm__ ("leal %c0(%%ebp), %%edx\n" : : "i" (offsetof(struct _jmpframe, \
                                                          args)));      \
    __asm__ ("movl %%edx, %c0(%%eax)\n" : : "i" (offsetof(struct _jmpbuf, \
                                                          esp)))
    
#define __longjmp(env, val)                                             \
    /* load jmp_buf pointer into ECX */                                 \
    __asm__ ("movl %0, %%ecx\n" : : "m" (env));                         \
    /* load return value into EAX */                                    \
    __asm__ ("movl %0, %%eax\n" : : "m" (val));                         \
    /* if val == 0, set return value in EAX to 1 */                     \
    __asm__ ("cmp $0, %eax\n"                                           \
             "jne 0f\n"                                                 \
             "movl $1, %eax\n"                                          \
             "0:\n");                                                   \
    /* restore EBX, ESI, EDI, EBP, and ESP to caller values */          \
    __asm__ ("movl %c0(%%ecx), %%ebx" : : "i" (offsetof(struct _jmpbuf, \
                                                        ebx)));         \
    __asm__ ("movl %c0(%%ecx), %%esi" : : "i" (offsetof(struct _jmpbuf, \
                                                        esi)));         \
    __asm__ ("movl %c0(%%ecx), %%edi" : : "i" (offsetof(struct _jmpbuf, \
                                                        edi)));         \
    __asm__ ("movl %c0(%%ecx), %%ebp" : : "i" (offsetof(struct _jmpbuf, \
                                                        ebp)));         \
    __asm__ ("movl %c0(%%ecx), %%esp" : : "i" (offsetof(struct _jmpbuf, \
                                                        esp)));         \
    /* jump back to caller with saved instruction pointer */            \
    __asm__ ("movl %c0(%%ecx), %%edx" : : "i" (offsetof(struct _jmpbuf, \
                                                        eip)));         \
    __asm__ ("jmpl *%edx\n")

#endif /* __IA32__SETJMP_H__ */

