#ifndef __SYS_IO_H__
#define __SYS_IO_H__

#include <features.h>
#include <errno.h>
#if (_ZERO_SOURCE)
#include <zero/param.h>
#if 0
#include <kern/syscall.h>
#include <kern/syscallnum.h>
#endif
#include <sys/zero/syscall.h>
#endif

#define _IODELAY()  "outb %%al, $0x80\n"
#define iodelay() __asm__ __volatile__ ("outb %al, $0x80\n");

#if (_ZERO_SOURCE) && (!__KERNEL__)
extern ASMLINK long _syscall(long num, long arg1, long arg2, long arg3);
#endif

#if (_ZERO_SOURCE)

#if (!__KERNEL__)
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
#endif /* !__KERNEL__ */

#endif /* _ZERO_SOURCE */

static __inline__ unsigned char
inb(unsigned short port)
{
    unsigned char ret = 0;
    
    __asm__ __volatile__ ("inb %1, %b0\n" : "=a" (ret) : "Nd" (port));
    
    return ret;
    
}

static __inline__ unsigned char
inb_p(unsigned short port)
{
    unsigned char ret = 0;
    
    __asm__ __volatile__ ("inb %1, %b0\n"
                          _IODELAY()
                          : "=a" (ret) : "Nd" (port));

    return ret;
    
}

static __inline__ unsigned short
inw(unsigned short port)
{
    unsigned short ret = 0;

    __asm__ __volatile__ ("inw %1, %w0\n" : "=a" (ret) : "Nd" (port));

    return ret;
    
}

static __inline__ unsigned short
inw_p(unsigned short port)
{
    unsigned short ret = 0;

    __asm__ __volatile__ ("inw %1, %w0\n"
                          _IODELAY()
                          : "=a" (ret) : "Nd" (port));

    return ret;
    
}

static __inline__ unsigned int
inl(unsigned short port)
{
    unsigned int ret = 0;

    __asm__ __volatile__ ("inl %1, %0\n" : "=a" (ret) : "Nd" (port));

    return ret;
    
}

static __inline__ unsigned int
inl_p(unsigned short port)
{
    unsigned int ret = 0;

    __asm__ __volatile__ ("inl %1, %0\n"
                          _IODELAY()
                          : "=a" (ret) : "Nd" (port));

    return ret;
    
}

static __inline__ void
outb(unsigned char byte, unsigned short port)
{
    __asm__ __volatile__ ("outb %b0, %w1\n" : : "a" (byte), "Nd" (port));

    return;
}

static __inline__ void
outb_p(unsigned char byte, unsigned short port)
{
    __asm__ __volatile__ ("outb %b0, %w1\n"
                          _IODELAY()
                          : : "a" (byte), "Nd" (port));

    return;
}

static __inline__ void
outw(unsigned short word, unsigned short port)
{
    __asm__ __volatile__ ("outw %w0, %w1\n" : : "a" (word), "Nd" (port));

    return;
}

static __inline__ void
outw_p(unsigned short word, unsigned short port)
{
    __asm__ __volatile__ ("outw %w0, %w1\n"
                          _IODELAY()
                          : : "a" (word), "Nd" (port));

    return;
}

static __inline__ void
outl(unsigned int longword, unsigned short port)
{
    __asm__ __volatile__ ("outl %0, %w1\n" : : "a" (longword), "Nd" (port));

    return;
}

static __inline__ void
outl_p(unsigned int longword, unsigned short port)
{
    __asm__ __volatile__ ("outl %0, %w1\n"
                          _IODELAY()
                          : : "a" (longword), "Nd" (port));

    return;
}

#endif /* __SYS_IO_H__ */

