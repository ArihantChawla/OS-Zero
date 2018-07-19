#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <mach/param.h>
#include <zero/trix.h>
#include <sys/zero/syscall.h>

int
chroot(const char *path)
{
    int retval;

    retval = (int)_syscall(SYS_SYSCTL, SYSCTL_CHROOT, SYS_NOARG,
                           (uintptr_t)path);

    return retval;
}

/* enable or disable I/O on a range of ports */
static __inline__ int
ioperm(unsigned long from, unsigned long num, int val)
{
    struct sysioctl buf;
    int             retval;

    if (from > NIOPORT || (from + num > NIOPORT)) {
        errno = EINVAL;

        return -1;
    }
    buf.parm = val;
    buf.reg.ofs = from;
    buf.reg.len = num;
    retval = (int)_syscall(SYS_IOCTL, SYS_IOCTL_IOPERM, SYS_NOARG, (long)&buf);
    if (retval < 0) {
        errno = EPERM;
    }

    return retval;
}

