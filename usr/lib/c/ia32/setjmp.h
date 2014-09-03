#ifndef __IA32_SETJMP_H__
#define __IA32_SETJMP_H__

#include <stddef.h>
#include <stdint.h>
#include <signal.h>

#include <zero/cdecl.h>

struct _jmpbuf {
    int32_t  ebx;
    int32_t  esi;
    int32_t  edi;
    int32_t  ebp;
    int32_t  esp;
    int32_t  eip;
#if (_POSIX_SOURCE)
    sigset_t sigmask;
#elif (_BSD_SOURCE)
    int      sigmask;
#endif
#if (_POSIX_C_SOURCE) || (_XOPEN_SOURCE)
    long     havesigs;
#endif
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
    __asm__ __volatile__ ("movl %0, %%eax\n"                            \
                          "movl %%ebx, %c1(%%eax)\n"                    \
                          "movl %%esi, %c2(%%eax)\n"                    \
                          "movl %%edi, %c3(%%eax)\n"                    \
                          "movl %c4(%%ebp), %%edx\n"                    \
                          "movl %%edx, %c5(%%eax)\n"                    \
                          "movl %c6(%%ebp), %%ecx\n"                    \
                          "movl %%ecx, %c7(%%eax)\n"                    \
                          "leal %c8(%%ebp), %%edx\n"                    \
                          "movl %%edx, %c9(%%eax)\n"                    \
                          :                                             \
                          : "m" (env),                                  \
                          "i" (offsetof(struct _jmpbuf, ebx)),          \
                            "i" (offsetof(struct _jmpbuf, esi)),        \
                            "i" (offsetof(struct _jmpbuf, edi)),        \
                            "i" (offsetof(struct _jmpframe, ebp)),      \
                            "i" (offsetof(struct _jmpbuf, ebp)),        \
                            "i" (offsetof(struct _jmpframe, eip)),      \
                            "i" (offsetof(struct _jmpbuf, eip)),        \
                            "i" (offsetof(struct _jmpframe, args)),     \
                            "i" (offsetof(struct _jmpbuf, esp))         \
                          : "eax", "ecx", "edx")

#define __longjmp(env, val)                                             \
    __asm__ __volatile__ ("movl %0, %%ecx\n"                            \
                          "movl %1, %%eax\n"                            \
                          "cmp $0, %eax\n"                              \
                          "jne 0f\n"                                    \
                          "movl $1, %eax\n"                             \
                          "0:\n"                                        \
                          "movl %c2(%%ecx), %%ebx"                      \
                          "movl %c3(%%ecx), %%esi"                      \
                          "movl %c4(%%ecx), %%edi"                      \
                          "movl %c5(%%ecx), %%ebp"                      \
                          "movl %c6(%%ecx), %%esp"                      \
                          "movl %c7(%%ecx), %%edx"                      \
                          "jmpl *%edx\n"                                \
                          :                                             \
                          : "m" (env),                                  \
                            "m" (val),                                  \
                            "i" (offsetof(struct _jmpbuf, ebx)),        \
                            "i" (offsetof(struct _jmpbuf, esi)),        \
                            "i" (offsetof(struct _jmpbuf, edi)),        \
                            "i" (offsetof(struct _jmpbuf, ebp)),        \
                            "i" (offsetof(struct _jmpbuf, esp)),        \
                            "i" (offsetof(struct _jmpbuf, eip))         \
                          : "eax", "ebx", "ecx", "edx",                 \
                            "esi", "edi", "ebp", "esp")

#endif /* __IA32_SETJMP_H__ */

