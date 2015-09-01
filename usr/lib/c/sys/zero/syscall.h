#ifndef __SYS_ZERO_SYSCALL_H__
#define __SYS_ZERO_SYSCALL_H__

#include <zero/cdecl.h>
#include <zero/param.h>
#include <kern/syscall.h>

#if (LONGSIZE == 4) || defined(__x86_64__) || defined(__amd64__)
ASMLINK
sysreg_t _syscall(sysreg_t num, sysreg_t arg1, sysreg_t arg2, sysreg_t arg3);
#elif defined(_WIN32) || defined(__i386__)
ASMLINK
sysreg_t _syscall(sysreg_t num, sysreg_t arg1, sysreg_t arg2, sysreg_t arg3);
#endif

#endif /* __SYS_ZERO_SYSCALL_H__ */

