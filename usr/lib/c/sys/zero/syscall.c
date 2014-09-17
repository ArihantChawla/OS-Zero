#include <zero/param.h>
#include <zero/cdecl.h>

#if defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__)

ASMLINK
long
_syscall(long num, long arg1, long arg2, long arg3)
{
    long retval;

    retval = -1;
    __asm__ __volatile__ ("movq %1, %%eax\n"
                          "movq %2, %%ebx\n"
                          "movq %3, %%ecx\n"
                          "movq %4, %%edx\n"
                          "int $0x80\n"
                          "movq %%eax, %0\n"
                          : "=a" (retval)
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
    __asm__ __volatile__ ("movq %1, %%rax\n"
                          "movq %2, %%rbx\n"
                          "movq %3, %%rcx\n"
                          "movq %4, %%rdx\n"
                          "int $0x80\n"
                          "movq %%rax, %0\n"
                          : "=a" (retval)
                          : "rm" (num), "rm" (arg1), "rm" (arg2), "rm" (arg3));

    return retval;
}

#endif

