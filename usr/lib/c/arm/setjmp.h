#ifndef __MACH_ARM_SETJMP_H__
#define __MACH_ARM_SETJMP_H__

#include <stddef.h>
#include <stdint.h>
#include <signal.h>

#include <zero/cdecl.h>

#if 0 /* ARMv6-M */

/* THANKS to Kazu Hirata for putting this code online :) */

struct _jmpbuf {
    int32_t r4;
    int32_t r5;
    int32_t r6;
    int32_t r7;
    int32_t r8;
    int32_t r9;
    int32_t r10;
    int32_t fp;
    int32_t sp;
    int32_t lr;
    sigset_t sigmask;
} PACK();

#define __setjmp(env)                                                   \
    __asm__ ("mov r0, %0\n" : : "r" (env));				\
    __asm__ ("stmia r0!, { r4 - r7 }\n");                               \
    __asm__ ("mov r1, r8\n");						\
    __asm__ ("mov r2, r9\n");						\
    __asm__ ("mov r3, r10\n");						\
    __asm__ ("mov r4, fp\n");						\
    __asm__ ("mov r5, sp\n");						\
    __asm__ ("mov r6, lr\n");						\
    __asm__ ("stmia r0!, { r1 - r6 }\n");                               \
    __asm__ ("sub r0, r0, #40\n");					\
    __asm__ ("ldmia r0!, { r4, r5, r6, r7 }\n");			\
    __asm__ ("mov r0, #0\n");						\
    __asm__ ("bx lr\n")

#define __longjmp(env, val)                         			\
    __asm__ ("mov r0, %0\n" : : "r" (env));				\
    __asm__ ("mov r1, %0\n" : : "r" (val));				\
    __asm__ ("add r0, r0, #16\n");					\
    __asm__ ("ldmia r0!, { r2 - r6 }\n");                               \
    __asm__ ("mov r8, r2\n");						\
    __asm__ ("mov r9, r3\n");						\
    __asm__ ("mov r10, r4\n");						\
    __asm__ ("mov fp, r5\n");						\
    __asm__ ("mov sp, r6\n");						\
    __asm__ ("ldmia r0!, { r3 }\n");					\
    __asm__ ("sub r0, r0, #40\n");					\
    __asm__ ("ldmia r0!, { r4 - r7 }\n");                               \
    __asm__ ("mov r0, r1\n");						\
    __asm__ ("moveq r0, #1\n");						\
    __asm__ ("bx r3\n")

#endif /* 0 */
    
struct _jmpbuf {
    int32_t r4;
    int32_t r5;
    int32_t r6;
    int32_t r7;
    int32_t r8;
    int32_t r9;
    int32_t r10;
    int32_t fp;
    int32_t sp;
    int32_t lr;
    sigset_t sigmask;
} PACK();

#define __setjmp(env)                                                   \
    __asm__ ("movs r0, %0" : : "r" (env));				\
    __asm__ ("stmia r0!, { r4-r10, fp, sp, lr }\n");                    \
    __asm__ ("movs r0, #0\n")

#define __longjmp(env, val)                                             \
    __asm__ ("movs r0, %0" : : "r" (env));				\
    __asm__ ("movs r1, %0" : : "r" (val));				\
    __asm__ ("ldmia r0!, { r4-r10, fp, sp, lr }\n");                    \
    __asm__ ("movs r0, r1\n");						\
    __asm__ ("moveq r0, #1\n");						\
    __asm__ ("bx lr\n")

#endif /* __MACH_ARM_SETJMP_H__ */

