#ifndef __IA32_SETJMP_H__
#define __IA32_SETJMP_H__

#include <features.h>
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
    int32_t ebp;
    int32_t eip;
    uint8_t args[EMPTY];
};

#endif /* __IA32_SETJMP_H__ */

