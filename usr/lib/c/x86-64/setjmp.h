/*
 * THANKS
 * ------
 * - Henry 'froggey' Harrington for amd64-fixes
 * - Jester01 and fizzie from ##c on Freenode
 */

#ifndef __X86_64_SETJMP_H__
#define __X86_64_SETJMP_H__

#include <features.h>
#include <stddef.h>
#include <stdint.h>
#include <signal.h>
#include <zero/cdecl.h>

struct _jmpbuf {
    int64_t  rbx;
    int64_t  r12;
    int64_t  r13;
    int64_t  r14;
    int64_t  r15;
    int64_t  rbp;
    int64_t  rsp;
    int64_t  rip;
#if defined(_POSIX_SOURCE)
    sigset_t sigmask;
#elif defined(_BSD_SOURCE)
    int      sigmask;
#endif
#if defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)
    long     havesigs;
#endif
};

typedef struct _jmpbuf jmp_buf[1];

struct _jmpframe {
    int64_t rbp;
    int64_t rip;
    uint8_t args[EMPTY];
};

#endif /* __X86_64_SETJMP_H__ */

