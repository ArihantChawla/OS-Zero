#include <errno.h>
#include <zero/param.h>
#include <zero/cdecl.h>

#if defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__)

ASMLINK
long
_syscall(long num, long arg1, long arg2, long arg3)
{
    long retval;

    retval = -1;
    __asm__ __volatile__ ("movq %2, %%eax\n"
                          "movq %3, %%ebx\n"
                          "movq %4, %%ecx\n"
                          "movq %5, %%edx\n"
                          "int $0x80\n"
                          "jc 1\n"
                          "movq %%eax, %0\n"
                          "jmp 2\n"
                          "1:\n"
                          "movq %%eax, %1\n"
                          "2:\n"
                          : "=a" (retval), "=m" (errno)
                          : "rm" (num), "rm" (arg1), "rm" (arg2), "rm" (arg3));

    return retval;
}

#elif defined(__x86_64__) || defined(__amd64__)

ASMLINK
long
_syscall(long num, long arg1, long arg2, long arg3)
{
    long retval;

    retval = -1;
    __asm__ __volatile__ ("movq %2, %%rax\n"
                          "movq %3, %%rbx\n"
                          "movq %4, %%rcx\n"
                          "movq %5, %%rdx\n"
                          "int $0x80\n"
                          "jc 1\n"
                          "movq %%rax, %0\n"
                          "jmp 2\n"
                          "1:\n"
                          "movq %%rax, %1\n"
                          "2:\n"
                          : "=a" (retval), "=m" (errno)
                          : "rm" (num), "rm" (arg1), "rm" (arg2), "rm" (arg3));

    return retval;
}

#endif

