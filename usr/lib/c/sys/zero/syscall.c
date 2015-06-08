/*
 * libc to syscall interface for Zero
 */

#include <features.h>

#if (_ZERO_SOURCE)

#include <errno.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <kern/syscall.h>

#if (LONGSIZE == 4) || defined(__x86_64__) || defined(__amd64__)

ASMLINK
sysreg_t
_syscall(sysreg_t num, sysreg_t arg1, sysreg_t arg2, sysreg_t arg3)
{
    sysreg_t retval;

    retval = -1;
    __asm__ __volatile__ ("movq %3, %%rax\n"
                          "movq %4, %%rbx\n"
                          "movq %5, %%rcx\n"
                          "movq %6, %%rdx\n"
                          "int $0x80\n"
                          "jc 1f\n"
                          "movq %%rax, %0\n"
                          "jmp 2f\n"
                          "1:\n"
                          "movq %%rax, %1\n"
                          "cmpq %2, %%rax\n"
                          "jne 2f\n"
                          "movq %%rbx, %%rax\n"
                          "2:\n"
                          : "=a" (retval), "=m" (errno)
                          : "i" (EINTR),
                            "rm" (num), "rm" (arg1), "rm" (arg2), "rm" (arg3));

    return retval;
}

#elif defined(_WIN32) || defined(__i386__)

/*
 * prepare for a system call and do it
 * - EAX/RAX is used for system call number
 * - EBX/RBX is used for 1st system call argument
 * - EcX/RCX is used for 2nd system call argument
 * - EDX/RDX is used for 3rd system call argument
 */
ASMLINK
sysreg_t
_syscall(sysreg_t num, sysreg_t arg1, sysreg_t arg2, sysreg_t arg3)
{
    sysreg_t retval;

    retval = -1;
    __asm__ __volatile__ ("movl %3, %%eax\n"
                          "movl %4, %%ebx\n"
                          "movl %5, %%ecx\n"
                          "movl %6, %%edx\n"
                          "int $0x80\n"
                          "jc 1f\n"
                          "movl %%eax, %0\n"
                          "jmp 2f\n"
                          "1:\n"
                          "movl %%eax, %1\n"
                          "cmpl %2, %%eax\n"
                          "jne 2f\n"
                          "movl %%ebx, %%eax\n"
                          "2:\n"
                          : "=a" (retval), "=m" (errno)
                          : "i" (EINTR),
                            "rm" (num), "rm" (arg1), "rm" (arg2), "rm" (arg3));

    return retval;
}

#endif

#endif /* _ZERO_SOURCE */

