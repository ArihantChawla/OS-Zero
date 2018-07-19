/*
 * libc to syscall interface for Zero
 */

#include <features.h>
#include <errno.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <sys/zero/syscall.h>

#if defined(_WIN32) || (defined(__i386__)                               \
                        && !defined(__x86_64) && !defined(__amd64))

/*
 * system entry
 * ------------
 * - EAX is used for system call number
 * - EBX is used for 1st system call argument
 * - EcX is used for 2nd system call argument
 * - EDX is used for 3rd system call argument
 * LATER
 * -----
 * - ESI is used for 4th system call argument
 * - EDI is used for 5th system call argument
 * - EBP is used for 6th system call argument
 * after system call
 * -----------------
 * - %eax contains return value or errno if CF is not set in EFLAGS
 *   - otherwise, %eax contains errno
 */
ASMLINK
sysreg_t
_syscall(sysreg_t num, sysreg_t arg1, sysreg_t arg2, sysreg_t arg3)
{
    sysreg_t retval;

    __asm__ __volatile__ ("movl %2, %%eax\n"    // load system call number
                          "movl %3, %%ebx\n"    // load system call arguments
                          "movl %4, %%ecx\n"
                          "movl %5, %%edx\n"
                          "int $0x80\n"         // generate trap
                          "jnc 1f\n"            // return if carry clear
                          "movl %eax, %1\n"     // store errno from EAX
                          "movl $-1, %0"        // set return value to -1
                          "1:\n"
                          : "=a" (retval), "=m" (errno)
                          : "rm" (num), "rm" (arg1), "rm" (arg2), "rm" (arg3)
                          : "eax", "ebx", "ecx", "edx", "memory")

    return retval;
}

#elif (LONGSIZE == 4) || defined(__x86_64__) || defined(__amd64__)

/*
 * system entry
 * ------------
 * - RAX is used for system call number
 * - RDI is used for 1st system call argument
 * - RSI is used for 2nd system call argument
 * - RDX is used for 3rd system call argument
 * LATER
 * -----
 * - R10 is used for 4th system call argument
 * - R8 is used for 5th system call argument
 * - R9 is used for 6th system call argument
 * after system call
 * -----------------
 * - %eax contains return value or errno if CF is not set in EFLAGS
 *   - otherwise, %eax contains errno
 */

ASMLINK
sysreg_t
_syscall(sysreg_t num, sysreg_t arg1, sysreg_t arg2, sysreg_t arg3)
{
    sysreg_t retval;

    return retval;
}

#endif

